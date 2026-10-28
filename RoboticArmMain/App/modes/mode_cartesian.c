/* mode_cartesian.c - joystick moves X/Y, encoder moves Z, encoder press = gripper (guide 7.1)
 *
 * owner: TODO(team) (works together with the ik member)
 * the target pose is only accepted if ik solves it AND motion takes the solution unchanged;
 * otherwise the arm holds, the last accepted pose stays and the lcd shows the reason
 * (OUT OF REACH / JOINT LIMIT / IK NOT READY / MOTION REFUSED).
 * Cartesian-only IK is activated by ARM_CARTESIAN_IK_ENABLED; voice kin_ik() stays fail-closed.
 * Entering the mode never commands motion. IK fixes J4 at ARM_J4_ZERO_DEG, so the first accepted
 * move from the installed home (J4 90) rolls J4 to the reference (88, the supervised bench move).
 * If J4 is further than CART_MAX_ROLL_DEG from the reference, the mode stays blocked and shows
 * ALIGN J4 <reference>; align it in a manual mode and re-enter. The mode forces SPEED_FINE on entry
 * (the operator can still toggle FAST). s_pose is the last accepted REQUESTED pose, not the current
 * target; an IK or exact-motion failure holds motion, leaves s_pose for the UI, and rebases the next
 * control increment on the held FK pose so a reverse input cannot resume a cancelled target.
 */
#include "modes/mode_ops.h"
#include "modes/gripper.h"
#include "motion/motion.h"
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"
#include "common/app_config.h"
#include "common/app_util.h"
#include "lcd/lcd.h"
#include "main.h"
#include <stdio.h>
#if APP_USE_INPUT
#include "input/input.h"
#endif

#define XY_FINE_MM_S 20.0f
#define XY_FAST_MM_S 60.0f
#define Z_STEP_MM    5.0f      /* per encoder detent */
#define CART_MAX_ROLL_DEG 2.1f /* home J4 90 vs reference 88: bench-tested 2 deg + 0.1 margin */

static cart_pose_t s_pose;      /* last accepted requested pose, not the current target */
static bool        s_ready;     /* guard, roll and fk all passed on enter */
static bool        s_roll_blocked; /* enter blocked only because J4 is far from the IK reference */
static bool        s_rebase_needed; /* a failure held the arm; next request starts from held current */
static bool        s_gripper_closed;
static uint32_t    s_last_ms;
static kin_status_t s_last_status = KIN_NOT_READY;

/* After a failure holds motion, s_pose can still describe a request that was ahead of the arm.
 * The next control increment must start where the arm actually held, not from that cancelled
 * target, or a reverse input can move the wrong way. UI keeps s_pose; control uses this helper. */
static bool control_origin(cart_pose_t *out)
{
  if (!s_rebase_needed) {
    *out = s_pose;
    return true;
  }
  joint_vec_t now;
  motion_get_current(&now);
  kin_status_t st = kin_fk(&now, out);
  if (st != KIN_OK) {
    s_last_status = st;
    s_ready = false;                 /* block until the mode is re-entered */
    lcd_show_message(kin_status_name(st), 3000u);
    return false;
  }
  return true;
}

static bool try_pose(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  kin_status_t st = kin_ik_cartesian(p, &now, &sol);
  s_last_status = st;
  if (st != KIN_OK) {
    s_rebase_needed = true;
    motion_hold();     /* stop an earlier pending target; s_pose stays the last accepted request */
    lcd_show_message(kin_status_name(st), 1500u);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  /* all or nothing: a clamped / refused target would leave s_pose describing a pose the arm is not
   * going to, so it is a failure. the arm stops where it is and s_pose keeps the last accepted pose */
  motion_result_t r = motion_set_target_exact(&sol);
  if (r != MOTION_OK) {
    s_rebase_needed = true;
    s_last_status = (r == MOTION_CLAMPED) ? KIN_JOINT_LIMIT : KIN_NOT_READY;
    motion_hold();
    lcd_show_message(r == MOTION_CLAMPED ? "JOINT LIMIT" : "MOTION REFUSED", 1500u);
    return false;
  }
  s_rebase_needed = false;          /* only a successful new request clears the rebase */
  return true;
}

static void enter(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  motion_set_speed(SPEED_FINE);                         /* safe default; FAST is still a manual toggle */
  const bool enabled = kin_ik_cartesian_enabled();
  const kin_status_t fk = kin_fk(&now, &s_pose);
  s_last_status = fk;                                   /* never show a stale status from the last visit */
  const bool roll_ok = absf(now.deg[J4_WRIST_ROLL] - ARM_J4_ZERO_DEG) <= CART_MAX_ROLL_DEG;
  s_roll_blocked = enabled && !roll_ok;
  s_ready = enabled && roll_ok && (fk == KIN_OK);        /* FK/roll alone do not unlock moves */
  s_rebase_needed = false;                              /* s_pose is the FK of the current pose here */
  s_gripper_closed = gripper_is_closed_at(now.deg[J6_GRIPPER]);   /* lower angle = closed */
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

/* blocked mode: a knob turn / press or a stick push shows WHY nothing moves (on the lcd page too),
 * instead of silently doing nothing. it never moves anything */
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
    if (e->type == EVT_ENC_STEP || e->type == EVT_ENC_PRESS) show_block_reason();
    return;
  }
  switch ((evt_type_t)e->type) {
    case EVT_ENC_STEP: {
      cart_pose_t p;
      if (!control_origin(&p)) break;
      p.z_mm += (float)e->v[0] * Z_STEP_MM;
      if (try_pose(&p)) s_pose = p;
      break;
    }
    case EVT_ENC_PRESS:
      if (gripper_command(!s_gripper_closed)) s_gripper_closed = !s_gripper_closed;
      else lcd_show_message("GRIPPER LIMIT", 2000u);         /* state unchanged, J6 held */
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
  cart_pose_t p;
  if (!control_origin(&p)) return;
  /* operator behind the arm (28 Oct 2026): stick up = forward (+X), stick right = the arm's right (-Y).
   * input_joy_x/y are already the operator's right / up (input.c handles the module mounting) */
  p.x_mm += jy * v * dt;
  p.y_mm -= jx * v * dt;
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
  if (n > 3) snprintf(lines[3], LCD_COLS, "Gripper: %s", s_gripper_closed ? "CLOSED" : "OPEN");
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
