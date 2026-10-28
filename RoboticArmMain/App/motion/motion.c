/* motion.c - joint targets in, smooth PWM out
 *
 * The TIM6 isr steps s_cur toward s_tgt; the main loop only changes them inside a short
 * critical section. Each joint ramps its speed (accel = speed / APP_RAMP_S) and the joints of
 * one move are scaled to arrive together. Braking always uses the full accel.
 * hold, jog release, jog timeout and a safety freeze stop at once (no ramp).
 */
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "main.h"
#include <math.h>

#define DT_S        (1.0f / (float)APP_CONTROL_HZ)

/* J2/J5 coupling: J2 may go below 90 only while J5 >= 65 (current or pending), and J5 may go
 * below 65 only while J2 >= 90. Setters clamp the target once, so an accepted target never
 * changes later. s_cur is the commanded angle, there are no joint sensors. */
#define J2_MIN_DEG        MOTION_J2_COUPLED_DEG
#define J5_WRIST_UP_DEG   MOTION_J5_WRIST_UP_DEG

static joint_vec_t       s_cur;                 /* commanded angle now */
static joint_vec_t       s_tgt;                 /* where the trajectory is heading */
static float             s_vel[ARM_N_JOINTS];   /* deg/s */
static volatile float    s_jog_v[ARM_N_JOINTS];
static volatile uint32_t s_jog_ms[ARM_N_JOINTS];
static volatile speed_level_t s_speed = SPEED_FINE;
static volatile bool     s_started;
/* first pulse one joint at a time: all six at once was too much current for the 6 V supply.
 * gripper first */
static const int         BOOT_ORDER[ARM_N_JOINTS] = { J6_GRIPPER, J1_BASE, J2_SHOULDER, J3_ELBOW,
                                                      J4_WRIST_ROLL, J5_WRIST_PITCH };
static int               s_boot_n;
static uint32_t          s_boot_ms;
static volatile bool     s_idle = true;

static inline float minf(float a, float b) { return (a < b) ? a : b; }

static inline uint32_t irq_save(void)       { uint32_t p = __get_PRIMASK(); __disable_irq(); return p; }
static inline void     irq_restore(uint32_t p) { __set_PRIMASK(p); }

static float speed_deg_s(void)
{
  return (s_speed == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
}

static float joint_min_deg(int j, const joint_vec_t *cur, const joint_vec_t *pending)
{
  const servo_cal_t *c = servo_cal(j);
  float lo = c->min_deg;

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
  servo_init();
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    s_cur.deg[j] = servo_cal(j)->home_deg;
    s_tgt.deg[j] = s_cur.deg[j];
    s_jog_v[j] = 0.0f;
    s_vel[j] = 0.0f;
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
    servo_write_deg(j, s_cur.deg[j]);
    s_boot_ms = t;
    if (s_boot_n == ARM_N_JOINTS) s_started = true;
    return;
  }

  bool allowed = safety_motion_allowed();
  if (!allowed) {
    for (int j = 0; j < ARM_N_JOINTS; j++) {        /* freeze */
      s_tgt.deg[j] = s_cur.deg[j];
      s_jog_v[j] = 0.0f;
      s_vel[j] = 0.0f;
    }
    s_idle = true;
    return;
  }

  uint32_t now = HAL_GetTick();
  const float vmax = speed_deg_s();
  const float acc = vmax / APP_RAMP_S;
  bool idle = true;

  /* the largest remaining move sets the pace */
  float max_err = 0.0f;
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    if (s_jog_v[j] != 0.0f) continue;
    float e = absf(s_tgt.deg[j] - s_cur.deg[j]);
    if (e > max_err) max_err = e;
  }

  for (int j = 0; j < ARM_N_JOINTS; j++) {
    /* re-clamp every tick. J2 goes before J5, so J5 sees J2's new angle */
    s_tgt.deg[j] = clamp_joint(j, s_tgt.deg[j], &s_cur, NULL);

    float v = s_jog_v[j];
    if (v != 0.0f) {
      if (ms_since(now, s_jog_ms[j]) > APP_JOG_TIMEOUT_MS) {
        s_jog_v[j] = 0.0f;
        s_tgt.deg[j] = s_cur.deg[j];
        s_vel[j] = 0.0f;
      } else {
        float lo = joint_min_deg(j, &s_cur, &s_tgt);
        s_tgt.deg[j] = clamp_joint_to(j, s_tgt.deg[j] + v * DT_S, lo, NULL);
        idle = false;
      }
    }

    float err = s_tgt.deg[j] - s_cur.deg[j];
    float share = (max_err > 1e-6f && s_jog_v[j] == 0.0f) ? clampf(absf(err) / max_err, 0.05f, 1.0f) : 1.0f;
    float vlim = vmax * share, alim = acc * share;
    /* fastest speed that can still stop on the target */
    float h = 0.5f * alim * DT_S;
    float vstop = sqrtf(h * h + 2.0f * alim * absf(err)) - h;
    float vdes = (err > 0.0f) ? minf(vlim, vstop) : (err < 0.0f) ? -minf(vlim, vstop) : 0.0f;
    /* speed up at the joint's share, brake at full accel */
    float dv_max = (absf(vdes) < absf(s_vel[j]) || vdes * s_vel[j] < 0.0f) ? acc * DT_S : alim * DT_S;
    float vel = s_vel[j] + clampf(vdes - s_vel[j], -dv_max, dv_max);
    float step = vel * DT_S;
    if (err != 0.0f && ((err > 0.0f && step >= err) || (err < 0.0f && step <= err))) {
      s_cur.deg[j] = s_tgt.deg[j];
      vel = 0.0f;
    } else if (err == 0.0f && absf(vel) < alim * DT_S * 1.01f) {
      vel = 0.0f;
    } else {
      bool clamped = false;
      float next = s_cur.deg[j] + step;
      /* a reversal coasts a bit: don't let it cross into the coupled zone the target avoids */
      float lo_pending = joint_min_deg(j, &s_cur, &s_tgt);
      if (step < 0.0f && next < lo_pending) {
        next = (s_cur.deg[j] > lo_pending) ? lo_pending : s_cur.deg[j];
        clamped = true;
      }
      s_cur.deg[j] = clamp_joint(j, next, &s_cur, &clamped);
      if (clamped) vel = 0.0f;
    }
    s_vel[j] = vel;
    if (s_cur.deg[j] != s_tgt.deg[j] || vel != 0.0f) idle = false;

    servo_write_deg(j, s_cur.deg[j]);
  }

  if (idle && !s_idle) (void)app_event_post0(EVT_MOTION_DONE);
  s_idle = idle;
}

/* ---------------------------------------------------------------- main-loop side */
/* exact = all or nothing: a target that would be clamped is not stored */
static motion_result_t set_target(const joint_vec_t *target, bool exact)
{
  if (target == NULL) return MOTION_REFUSED;

  uint32_t p = irq_save();
  joint_vec_t cur = s_cur;
  joint_vec_t t;
  bool clamped = false;
  for (int j = 0; j < ARM_N_JOINTS; j++) t.deg[j] = clamp_joint(j, target->deg[j], &cur, &clamped);

  /* per-joint checks can still pass an unsafe endpoint (e.g. 125/65 -> 50/25): hold J2 at 90 */
  if (t.deg[J2_SHOULDER] < J2_MIN_DEG && t.deg[J5_WRIST_PITCH] < J5_WRIST_UP_DEG) {
    t.deg[J2_SHOULDER] = J2_MIN_DEG;
    clamped = true;
  }
  if (exact && clamped) {
    irq_restore(p);
    return MOTION_CLAMPED;
  }

  s_tgt = t;
  for (int j = 0; j < ARM_N_JOINTS; j++) s_jog_v[j] = 0.0f;
  s_idle = false;
  irq_restore(p);
  return clamped ? MOTION_CLAMPED : MOTION_OK;
}

motion_result_t motion_set_target(const joint_vec_t *target)       { return set_target(target, false); }
motion_result_t motion_set_target_exact(const joint_vec_t *target) { return set_target(target, true); }

motion_result_t motion_set_joint_target(int joint, float deg)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return MOTION_REFUSED;

  uint32_t p = irq_save();
  bool clamped = false;
  float lo = joint_min_deg(joint, &s_cur, &s_tgt);
  float v = clamp_joint_to(joint, deg, lo, &clamped);
  s_tgt.deg[joint] = v;
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
  if (deg_per_s == 0.0f && s_jog_v[joint] != 0.0f) {
    /* jog released: stop this joint here, it trails the jog target */
    s_tgt.deg[joint] = s_cur.deg[joint];
    s_vel[joint] = 0.0f;
  }
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
    s_vel[j] = 0.0f;
  }
  irq_restore(p);
}

void motion_hold_joint(int joint)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return;
  uint32_t p = irq_save();
  s_jog_v[joint] = 0.0f;
  s_tgt.deg[joint] = s_cur.deg[joint];
  s_vel[joint] = 0.0f;
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
