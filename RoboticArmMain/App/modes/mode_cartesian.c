/* mode_cartesian.c - joystick moves X/Y, encoder moves Z, encoder press = gripper (guide 7.1)
 *
 * owner: TODO(team) (works together with the ik member)
 * the target pose is only accepted if ik solves it; otherwise the last reachable pose
 * stays and the lcd shows the reason (OUT OF REACH / IK NOT READY).
 * while IK is not activated (kin_ik_enabled() false) the whole mode stays blocked, gripper
 * included, even though FK is available for the estimated X/Y/Z display.
 */
#include "modes/mode_ops.h"
#include "modes/gripper.h"
#include "motion/motion.h"
#include "kinematics/kinematics.h"
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

static cart_pose_t s_pose;
static bool        s_ready;     /* fk worked on enter, so s_pose is meaningful */
static bool        s_gripper_closed;
static uint32_t    s_last_ms;
static kin_status_t s_last_status = KIN_NOT_READY;

static bool try_pose(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  kin_status_t st = kin_ik(p, &now, &sol);
  s_last_status = st;
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 1500u);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  (void)motion_set_target(&sol);
  return true;
}

static void enter(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  s_ready = kin_ik_enabled() && (kin_fk(&now, &s_pose) == KIN_OK);   /* FK alone does not unlock moves */
  s_gripper_closed = gripper_is_closed_at(now.deg[J6_GRIPPER]);   /* lower angle = closed */
  s_last_ms = HAL_GetTick();
  if (!s_ready) lcd_show_message(kin_ik_enabled() ? "IK NOT READY" : "IK DISABLED", 3000u);
}

static void exit_(void) { motion_hold(); }

static void on_event(const app_event_t *e)
{
  if (!s_ready) return;
  switch ((evt_type_t)e->type) {
    case EVT_ENC_STEP: {
      cart_pose_t p = s_pose;
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
  if (!s_ready) return;
  dt = clampf(dt, 0.0f, 0.1f);

#if APP_USE_INPUT
  float jx = input_joy_x(), jy = input_joy_y();
  if (jx == 0.0f && jy == 0.0f) return;
  float v = (motion_get_speed() == SPEED_FAST) ? XY_FAST_MM_S : XY_FINE_MM_S;
  cart_pose_t p = s_pose;
  p.x_mm += jx * v * dt;
  p.y_mm += jy * v * dt;
  if (try_pose(&p)) s_pose = p;
#endif
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  if (!s_ready) {
    if (n > 0) snprintf(lines[0], LCD_COLS, "%s", kin_ik_enabled() ? kin_status_name(s_last_status) : "IK DISABLED");
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
