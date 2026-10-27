/* motion.c - targets in, smooth PWM out
 *
 * concurrency: the TIM6 isr (priority 2) reads and writes s_tgt / s_cur. the main loop
 * changes them only inside a short critical section, so the isr never sees half of a
 * 6-joint update.
 *
 * TODO(motion): acceleration limit (trapezoid) on top of the speed limit
 * TODO(motion): per-joint speed limits if the shoulder needs to be slower than the wrist
 */
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "main.h"

#define DT_S        (1.0f / (float)APP_CONTROL_HZ)

/* J2/J5 coupling. The servo_cal table gives J2 its absolute 50 deg floor; this
 * rule can only raise a floor. J5 at or above 65 deg (current or pending) lets
 * the shoulder use 50 deg, otherwise J2 must stay at 90 deg or above. While J2 is
 * below 90 deg (current or pending), J5 must stay at 65 deg or above.
 * Setters clamp the stored endpoint before returning, so an accepted target never
 * changes later. A clamped value must be reissued after the pose allows.
 * "current" is the commanded angle s_cur, not a measured joint angle: there are no
 * joint sensors, so a loaded or fast servo can lag this pose. */
#define J2_MIN_DEG        MOTION_J2_COUPLED_DEG     /* values in motion.h: voice HOME stages around them */
#define J5_WRIST_UP_DEG   MOTION_J5_WRIST_UP_DEG

static joint_vec_t       s_cur;                 /* commanded angle now */
static joint_vec_t       s_tgt;                 /* where the trajectory is heading */
static volatile float    s_jog_v[ARM_N_JOINTS]; /* deg/s, 0 = no jog */
static volatile uint32_t s_jog_ms[ARM_N_JOINTS];
static volatile speed_level_t s_speed = SPEED_FINE;
static volatile bool     s_started;             /* every joint has had its first pulse */
/* no joint has had a pulse before start-up, so each one jumps to home at full speed. all six in one
 * tick drew more current than the 6 V servo supply gives (27 Oct 2026): one joint at a time instead.
 * gripper first: during flashing its floating signal can drive it to an end */
static const int         BOOT_ORDER[ARM_N_JOINTS] = { J6_GRIPPER, J1_BASE, J2_SHOULDER, J3_ELBOW,
                                                      J4_WRIST_ROLL, J5_WRIST_PITCH };
static int               s_boot_n;              /* joints that have had their first pulse */
static uint32_t          s_boot_ms;
static volatile bool     s_idle = true;

static inline uint32_t irq_save(void)       { uint32_t p = __get_PRIMASK(); __disable_irq(); return p; }
static inline void     irq_restore(uint32_t p) { __set_PRIMASK(p); }

static float speed_deg_s(void)
{
  return (s_speed == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
}

static float joint_min_deg(int j, const joint_vec_t *cur, const joint_vec_t *pending)
{
  const servo_cal_t *c = servo_cal(j);
  float lo = c->min_deg;                            /* static calibration is always a floor */

  if (j == J2_SHOULDER) {
    if (cur->deg[J5_WRIST_PITCH] < J5_WRIST_UP_DEG && lo < J2_MIN_DEG) lo = J2_MIN_DEG;
    if (pending != NULL && pending->deg[J5_WRIST_PITCH] < J5_WRIST_UP_DEG &&
        lo < J2_MIN_DEG) {
      lo = J2_MIN_DEG;
    }
  } else if (j == J5_WRIST_PITCH) {
    if (cur->deg[J2_SHOULDER] < J2_MIN_DEG && lo < J5_WRIST_UP_DEG) lo = J5_WRIST_UP_DEG;
    if (pending != NULL && pending->deg[J2_SHOULDER] < J2_MIN_DEG &&
        lo < J5_WRIST_UP_DEG) {
      lo = J5_WRIST_UP_DEG;
    }
  }
  return lo;
}

static float clamp_joint_to(int j, float deg, float min_deg, bool *clamped)
{
  const servo_cal_t *c = servo_cal(j);
  float v = clampf(deg, min_deg, c->max_deg);
  if (clamped && v != deg) *clamped = true;
  return v;
}

static float clamp_joint(int j, float deg, const joint_vec_t *cur, bool *clamped)
{
  return clamp_joint_to(j, deg, joint_min_deg(j, cur, NULL), clamped);
}

void motion_init(void)
{
  servo_init();                                     /* CCR = 0: no pulse yet */
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    s_cur.deg[j] = servo_cal(j)->home_deg;
    s_tgt.deg[j] = s_cur.deg[j];
    s_jog_v[j] = 0.0f;
  }
  s_started = false;
  s_boot_n = 0;
  LOG("motion: init, home pose loaded, pulses start one joint every %lu ms when safety allows\r\n",
      (unsigned long)MOTION_BOOT_STAGGER_MS);
}

/* ---------------------------------------------------------------- isr side */
void motion_tick(void)
{
  if (!s_started) {
    if (!safety_motion_allowed()) return;
    uint32_t t = HAL_GetTick();
    if (s_boot_n > 0 && ms_since(t, s_boot_ms) < MOTION_BOOT_STAGGER_MS) return;
    int j = BOOT_ORDER[s_boot_n++];
    servo_write_deg(j, s_cur.deg[j]);               /* this joint goes to home now */
    s_boot_ms = t;
    if (s_boot_n == ARM_N_JOINTS) s_started = true; /* targets set meanwhile start moving next tick */
    return;
  }

  bool allowed = safety_motion_allowed();
  if (!allowed) {
    for (int j = 0; j < ARM_N_JOINTS; j++) {        /* freeze: nothing jumps when allowed again */
      s_tgt.deg[j] = s_cur.deg[j];
      s_jog_v[j] = 0.0f;
    }
    s_idle = true;
    return;                                          /* CCRs keep their last value */
  }

  uint32_t now = HAL_GetTick();
  float max_step = speed_deg_s() * DT_S;
  bool idle = true;

  for (int j = 0; j < ARM_N_JOINTS; j++) {
    /* Re-clamp the stored target every tick. J2 is processed before J5, so the
     * J5 bound sees J2's new angle and the tick can never end in an unsafe pose. */
    s_tgt.deg[j] = clamp_joint(j, s_tgt.deg[j], &s_cur, NULL);

    float v = s_jog_v[j];
    if (v != 0.0f) {
      if (ms_since(now, s_jog_ms[j]) > APP_JOG_TIMEOUT_MS) {
        s_jog_v[j] = 0.0f;                          /* jog not refreshed: stop */
        s_tgt.deg[j] = s_cur.deg[j];
      } else {
        s_tgt.deg[j] = clamp_joint(j, s_tgt.deg[j] + v * DT_S, &s_cur, NULL);
        idle = false;
      }
    }

    float err = s_tgt.deg[j] - s_cur.deg[j];
    float step = clampf(err, -max_step, max_step);
    if (step != 0.0f && absf(err) <= max_step) {
      s_cur.deg[j] = s_tgt.deg[j];                  /* last step lands exactly on the target */
    } else {
      s_cur.deg[j] += step;
    }
    if (s_cur.deg[j] != s_tgt.deg[j]) idle = false;   /* exact: the last step snaps onto the target */

    servo_write_deg(j, s_cur.deg[j]);
  }

  if (idle && !s_idle) (void)app_event_post0(EVT_MOTION_DONE);
  s_idle = idle;
}

/* ---------------------------------------------------------------- main-loop side */
motion_result_t motion_set_target(const joint_vec_t *target)
{
  if (target == NULL) return MOTION_REFUSED;

  uint32_t p = irq_save();
  joint_vec_t cur = s_cur;                          /* coupling uses the pose we are in */
  joint_vec_t t;
  bool clamped = false;
  for (int j = 0; j < ARM_N_JOINTS; j++) t.deg[j] = clamp_joint(j, target->deg[j], &cur, &clamped);

  /* A full target may pass the per-joint current checks and still be an unsafe
   * endpoint, e.g. current 125/65 -> requested 50/25. Keep the wrist request and
   * deterministically hold the shoulder at 90; report the clamp now, not later. */
  if (t.deg[J2_SHOULDER] < J2_MIN_DEG && t.deg[J5_WRIST_PITCH] < J5_WRIST_UP_DEG) {
    t.deg[J2_SHOULDER] = J2_MIN_DEG;
    clamped = true;
  }

  s_tgt = t;
  for (int j = 0; j < ARM_N_JOINTS; j++) s_jog_v[j] = 0.0f;
  s_idle = false;
  irq_restore(p);
  return clamped ? MOTION_CLAMPED : MOTION_OK;
}

motion_result_t motion_set_joint_target(int joint, float deg)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return MOTION_REFUSED;

  uint32_t p = irq_save();
  bool clamped = false;
  float lo = joint_min_deg(joint, &s_cur, &s_tgt);   /* current pose AND the pending endpoint */
  float v = clamp_joint_to(joint, deg, lo, &clamped);
  s_tgt.deg[joint] = v;                             /* never rewrites the other joint */
  s_jog_v[joint] = 0.0f;
  s_idle = false;
  irq_restore(p);
  return clamped ? MOTION_CLAMPED : MOTION_OK;
}

void motion_jog(int joint, float deg_per_s)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return;
  float lim = speed_deg_s();
  deg_per_s = clampf(deg_per_s, -lim, lim);
  uint32_t p = irq_save();
  s_jog_v[joint] = deg_per_s;
  s_jog_ms[joint] = HAL_GetTick();
  irq_restore(p);
}

void motion_hold(void)
{
  uint32_t p = irq_save();
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    s_jog_v[j] = 0.0f;
    s_tgt.deg[j] = s_cur.deg[j];
  }
  irq_restore(p);
}

void motion_hold_joint(int joint)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return;
  uint32_t p = irq_save();
  s_jog_v[joint] = 0.0f;
  s_tgt.deg[joint] = s_cur.deg[joint];
  irq_restore(p);
}

void motion_get_current(joint_vec_t *out)
{
  uint32_t p = irq_save();
  *out = s_cur;
  irq_restore(p);
}

void motion_get_target(joint_vec_t *out)
{
  uint32_t p = irq_save();
  *out = s_tgt;
  irq_restore(p);
}

bool          motion_is_idle(void)                { return s_idle; }
bool          motion_started(void)                { return s_started; }
void          motion_set_speed(speed_level_t s)   { s_speed = s; }
speed_level_t motion_get_speed(void)              { return s_speed; }
void          motion_toggle_speed(void)           { s_speed = (s_speed == SPEED_FINE) ? SPEED_FAST : SPEED_FINE; }
