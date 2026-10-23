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

static joint_vec_t       s_cur;                 /* commanded angle now */
static joint_vec_t       s_tgt;                 /* where the trajectory is heading */
static volatile float    s_jog_v[ARM_N_JOINTS]; /* deg/s, 0 = no jog */
static volatile uint32_t s_jog_ms[ARM_N_JOINTS];
static volatile speed_level_t s_speed = SPEED_FINE;
static volatile bool     s_require_enable = true;
static volatile bool     s_started;             /* first pulse sent */
static volatile bool     s_idle = true;

static inline uint32_t irq_save(void)       { uint32_t p = __get_PRIMASK(); __disable_irq(); return p; }
static inline void     irq_restore(uint32_t p) { __set_PRIMASK(p); }

static float speed_deg_s(void)
{
  return (s_speed == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
}

static float clamp_joint(int j, float deg, bool *clamped)
{
  const servo_cal_t *c = servo_cal(j);
  float v = clampf(deg, c->min_deg, c->max_deg);
  if (clamped && v != deg) *clamped = true;
  return v;
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
  LOG("motion: init, home pose loaded, pulses start when safety allows\r\n");
}

/* ---------------------------------------------------------------- isr side */
void motion_tick(void)
{
  if (!s_started) {
    if (!safety_motion_allowed()) return;
    for (int j = 0; j < ARM_N_JOINTS; j++) servo_write_deg(j, s_cur.deg[j]);
    s_started = true;                               /* servos go to home here */
    return;
  }

  bool allowed = safety_motion_allowed() && (!s_require_enable || safety_enable_held());
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
    float v = s_jog_v[j];
    if (v != 0.0f) {
      if (ms_since(now, s_jog_ms[j]) > APP_JOG_TIMEOUT_MS) {
        s_jog_v[j] = 0.0f;                          /* jog not refreshed: stop */
        s_tgt.deg[j] = s_cur.deg[j];
      } else {
        s_tgt.deg[j] = clamp_joint(j, s_tgt.deg[j] + v * DT_S, NULL);
        idle = false;
      }
    }

    float err = s_tgt.deg[j] - s_cur.deg[j];
    float step = clampf(err, -max_step, max_step);
    int dir = (step > 0.0f) ? 1 : ((step < 0.0f) ? -1 : 0);
    if (dir != 0 && safety_limit_blocks(j, dir)) {
      step = 0.0f;
      s_tgt.deg[j] = s_cur.deg[j];
    }
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
  bool clamped = false;
  joint_vec_t t;
  for (int j = 0; j < ARM_N_JOINTS; j++) t.deg[j] = clamp_joint(j, target->deg[j], &clamped);

  uint32_t p = irq_save();
  s_tgt = t;
  for (int j = 0; j < ARM_N_JOINTS; j++) s_jog_v[j] = 0.0f;
  s_idle = false;
  irq_restore(p);
  return clamped ? MOTION_CLAMPED : MOTION_OK;
}

motion_result_t motion_set_joint_target(int joint, float deg)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return MOTION_REFUSED;
  bool clamped = false;
  float v = clamp_joint(joint, deg, &clamped);
  uint32_t p = irq_save();
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
void          motion_set_speed(speed_level_t s)   { s_speed = s; }
speed_level_t motion_get_speed(void)              { return s_speed; }
void          motion_toggle_speed(void)           { s_speed = (s_speed == SPEED_FINE) ? SPEED_FAST : SPEED_FINE; }
void          motion_require_enable(bool required){ s_require_enable = required; }
