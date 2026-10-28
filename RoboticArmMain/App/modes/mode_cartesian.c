/* mode_cartesian.c - joystick moves X/Y, knob moves Z or pitch, knob press = gripper
 *
 * A pose is taken only if IK solves it and motion accepts the solution unchanged; otherwise the
 * screen shows why and the arm finishes the last accepted pose. IK keeps J4 at ARM_J4_ZERO_DEG, so
 * the mode stays blocked (ALIGN J4) if J4 is too far from it. Entry forces FINE speed.
 *
 * Joystick steps are scaled to what the joints can do, and no new step is sent while the arm
 * trails the target by more than CART_LEAD_S. Knob detents turning back start from where the arm
 * is; a refused detent stops the arm there.
 */
#include "modes/mode_ops.h"
#include "modes/gripper.h"
#include "modes/mode_cartesian.h"
#include "motion/motion.h"
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"
#include "common/app_config.h"
#include "common/app_util.h"
#include "lcd/lcd.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#if APP_USE_INPUT
#include "input/input.h"
#endif

#define XY_FINE_MM_S 20.0f
#define XY_FAST_MM_S 60.0f
#define Z_STEP_MM    5.0f      /* per encoder detent */
#define PITCH_STEP_DEG 1.0f    /* per encoder detent while PITCH is selected */
#define PITCH_WRAP_MIN_DEG (-180.0f)
#define PITCH_WRAP_MAX_DEG (180.0f)
#define CART_MAX_ROLL_DEG 2.1f /* home J4 90 vs reference 88 */
#define CART_LEAD_S  0.15f     /* max lead of the target over the arm */

static cart_pose_t s_pose;      /* last accepted pose */
static bool        s_ready;
static bool        s_roll_blocked;
static bool        s_gripper_closed;
static bool        s_enc_pitch;
static uint32_t    s_last_ms;
static kin_status_t s_last_status = KIN_NOT_READY;

bool mode_cartesian_encoder_pitch(void) { return s_enc_pitch; }
bool mode_cartesian_ready(void)         { return s_ready; }
float mode_cartesian_pitch_deg(void)    { return s_pose.pitch_deg; }

static float wrap_pitch_deg(float deg)
{
  if (!isfinite((double)deg)) return deg;
  float w = fmodf(deg - PITCH_WRAP_MIN_DEG, PITCH_WRAP_MAX_DEG - PITCH_WRAP_MIN_DEG);
  if (w < 0.0f) w += PITCH_WRAP_MAX_DEG - PITCH_WRAP_MIN_DEG;
  return w + PITCH_WRAP_MIN_DEG;
}

#if APP_USE_INPUT
static float joint_speed_deg_s(void)
{
  return (motion_get_speed() == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
}

/* largest arm-joint difference, gripper not included */
static float max_joint_diff(const joint_vec_t *a, const joint_vec_t *b)
{
  float m = 0.0f;
  for (int j = 0; j < J6_GRIPPER; j++) {
    float d = absf(a->deg[j] - b->deg[j]);
    if (d > m) m = d;
  }
  return m;
}
#endif

/* refusal message from the IK diagnostic */
static void show_ik_diagnostic(const kin_diag_t *d)
{
  char msg[LCD_COLS];
  switch (d->reason) {
    case KIN_DIAG_JOINT_MIN:
      snprintf(msg, sizeof msg, "LIMIT J%d MIN %d", d->joint + 1, (int)d->limit_deg);
      break;
    case KIN_DIAG_JOINT_MAX:
      snprintf(msg, sizeof msg, "LIMIT J%d MAX %d", d->joint + 1, (int)d->limit_deg);
      break;
    case KIN_DIAG_COUPLING:
      snprintf(msg, sizeof msg, "COUPLING J%d>=%d", d->joint + 1, (int)d->limit_deg);
      break;
    case KIN_DIAG_GEOMETRY:
      snprintf(msg, sizeof msg, "OUT OF REACH");
      break;
    case KIN_DIAG_INVALID:
      snprintf(msg, sizeof msg, "INVALID TARGET");
      break;
    case KIN_DIAG_NOT_READY:
      snprintf(msg, sizeof msg, "IK NOT READY");
      break;
    default:
      snprintf(msg, sizeof msg, "%s", kin_status_name(d->status));
      break;
  }
  lcd_show_message(msg, 1500u);
}

/* send the pose, or show why not */
static bool try_pose(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  kin_diag_t diag;
  motion_get_current(&now);
  kin_status_t st = kin_ik_cartesian_diag(p, &now, &sol, &diag);
  s_last_status = st;
  if (st != KIN_OK) {
    show_ik_diagnostic(&diag);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  /* all or nothing: don't take a target motion would clamp */
  motion_result_t r = motion_set_target_exact(&sol);
  if (r != MOTION_OK) {
    s_last_status = (r == MOTION_CLAMPED) ? KIN_JOINT_LIMIT : KIN_NOT_READY;
    lcd_show_message(r == MOTION_CLAMPED ? "JOINT LIMIT" : "MOTION REFUSED", 1500u);
    return false;
  }
  return true;
}

static void enter(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  motion_set_speed(SPEED_FINE);
  const bool enabled = kin_ik_cartesian_enabled();
  const kin_status_t fk = kin_fk(&now, &s_pose);
  s_last_status = fk;
  const bool roll_ok = absf(now.deg[J4_WRIST_ROLL] - ARM_J4_ZERO_DEG) <= CART_MAX_ROLL_DEG;
  s_roll_blocked = enabled && !roll_ok;
  s_ready = enabled && roll_ok && (fk == KIN_OK);
  s_gripper_closed = gripper_is_closed_at(now.deg[J6_GRIPPER]);
  s_enc_pitch = false;
  s_last_ms = HAL_GetTick();
  if (!s_ready) {
    if (!enabled) {
      lcd_show_message("IK DISABLED", 3000u);
    } else if (s_roll_blocked) {
      char msg[24];
      snprintf(msg, sizeof msg, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
      lcd_show_message(msg, 3000u);
    } else {
      lcd_show_message(kin_status_name(fk), 3000u);
    }
  }
}

static void exit_(void) { motion_hold(); }

/* s_pose = FK of where the arm is. FK fails -> block the mode */
static bool rebase_on_current(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  kin_status_t fk = kin_fk(&now, &s_pose);
  if (fk != KIN_OK) {
    s_last_status = fk;
    s_ready = false;
    lcd_show_message(kin_status_name(fk), 3000u);
    return false;
  }
  return true;
}

/* knob in Z mode */
static void z_detents(int detents)
{
  float dz = (float)detents * Z_STEP_MM;
  joint_vec_t cj, tj;
  cart_pose_t cp, tp;
  motion_get_current(&cj);
  motion_get_target(&tj);
  if (kin_fk(&cj, &cp) == KIN_OK && kin_fk(&tj, &tp) == KIN_OK) {
    float pending = tp.z_mm - cp.z_mm;
    if ((dz > 0.0f && pending < -0.5f) || (dz < 0.0f && pending > 0.5f)) {
      if (!rebase_on_current()) return;            /* turning back: start from where the arm is */
    }
  }
  cart_pose_t p = s_pose;
  p.z_mm += dz;
  if (try_pose(&p)) {
    s_pose = p;
  } else {
    motion_hold();                                  /* refused: stop here */
    (void)rebase_on_current();
  }
}

/* knob in pitch mode: same as Z, 1 deg per detent */
static void pitch_detents(int detents)
{
  joint_vec_t cj, tj;
  cart_pose_t cp, tp;
  float dp = (float)detents * PITCH_STEP_DEG;
  motion_get_current(&cj);
  motion_get_target(&tj);
  if (kin_fk(&cj, &cp) == KIN_OK && kin_fk(&tj, &tp) == KIN_OK) {
    float pending = wrap_pitch_deg(tp.pitch_deg - cp.pitch_deg);
    if ((dp > 0.0f && pending < -0.5f) || (dp < 0.0f && pending > 0.5f)) {
      if (!rebase_on_current()) return;
    }
  }
  cart_pose_t p = s_pose;
  p.pitch_deg = wrap_pitch_deg(p.pitch_deg + dp);
  if (try_pose(&p)) {
    s_pose = p;
  } else {
    motion_hold();
    (void)rebase_on_current();
  }
}

/* blocked: show why on any input, never move */
static void show_block_reason(void)
{
  char msg[24];
  if (!kin_ik_cartesian_enabled())  snprintf(msg, sizeof msg, "IK DISABLED");
  else if (s_roll_blocked)          snprintf(msg, sizeof msg, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
  else                              snprintf(msg, sizeof msg, "%s", kin_status_name(s_last_status));
  lcd_show_message(msg, 2000u);
}

static void on_event(const app_event_t *e)
{
  if (!s_ready) {
    if (e->type == EVT_ENC_STEP || e->type == EVT_ENC_PRESS || e->type == EVT_TOUCH_SELECT) {
      show_block_reason();
    }
    return;
  }
  switch ((evt_type_t)e->type) {
    case EVT_ENC_STEP:
      if (s_enc_pitch) pitch_detents(e->v[0]);
      else             z_detents(e->v[0]);
      break;
    case EVT_TOUCH_SELECT:
      if (e->v[0] == 0) s_enc_pitch = !s_enc_pitch;
      break;
    case EVT_ENC_PRESS:
      if (gripper_command(!s_gripper_closed)) s_gripper_closed = !s_gripper_closed;
      else lcd_show_message("GRIPPER LIMIT", 2000u);
      break;
    default:
      break;
  }
}

static void step(void)
{
  uint32_t now = HAL_GetTick();
  float dt = (float)ms_since(now, s_last_ms) * 0.001f;
  s_last_ms = now;
  if (!s_ready) {
#if APP_USE_INPUT
    if (input_joy_x() != 0.0f || input_joy_y() != 0.0f) show_block_reason();
#endif
    return;
  }
  dt = clampf(dt, 0.0f, 0.1f);

#if APP_USE_INPUT
  float jx = input_joy_x(), jy = input_joy_y();
  if (jx == 0.0f && jy == 0.0f) return;
  float v = (motion_get_speed() == SPEED_FAST) ? XY_FAST_MM_S : XY_FINE_MM_S;
  joint_vec_t cur_j, tgt, sol;
  motion_get_current(&cur_j);
  motion_get_target(&tgt);
  /* let the joints catch up first */
  if (max_joint_diff(&tgt, &cur_j) > joint_speed_deg_s() * CART_LEAD_S) return;
  /* operator behind the arm: stick up = +X, stick right = -Y */
  float dx = jy * v * dt, dy = -jx * v * dt;
  cart_pose_t p = s_pose;
  p.x_mm += dx;
  p.y_mm += dy;
  /* scale the step down if the joints can't keep up */
  if (kin_ik_cartesian(&p, &cur_j, &sol) == KIN_OK) {
    float d = max_joint_diff(&sol, &tgt), budget = joint_speed_deg_s() * dt;
    if (d > budget && d > 1e-6f) {
      float k = budget / d;
      p = s_pose;
      p.x_mm += dx * k;
      p.y_mm += dy * k;
    }
  }
  if (try_pose(&p)) s_pose = p;
#endif
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  if (!s_ready) {
    if (n > 0) {
      if (!kin_ik_cartesian_enabled()) snprintf(lines[0], LCD_COLS, "IK DISABLED");
      else if (s_roll_blocked) snprintf(lines[0], LCD_COLS, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
      else snprintf(lines[0], LCD_COLS, "%s", kin_status_name(s_last_status));
    }
    return;
  }
  if (n > 0) snprintf(lines[0], LCD_COLS, "X: %d mm", (int)s_pose.x_mm);
  if (n > 1) snprintf(lines[1], LCD_COLS, "Y: %d mm", (int)s_pose.y_mm);
  if (n > 2) snprintf(lines[2], LCD_COLS, "Z: %d mm", (int)s_pose.z_mm);
  if (n > 3) snprintf(lines[3], LCD_COLS, "%s  G:%s  P:%d",
                      s_enc_pitch ? "PITCH" : "Z", s_gripper_closed ? "CLOSED" : "OPEN",
                      (int)s_pose.pitch_deg);
}

const mode_ops_t MODE_CARTESIAN_OPS = {
  .name = "CARTESIAN",
  .manual = true,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
