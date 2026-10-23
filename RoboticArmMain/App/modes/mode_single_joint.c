/* mode_single_joint.c - service mode: one joint at a time (guide 6.2, 7)
 *
 * owner: TODO(team)
 *   encoder turn  -> select J1..J6
 *   joystick X    -> move the selected joint (hold-to-enable required)
 *   encoder press -> reset the selected target to the current commanded position
 * this is also the calibration mode: the lcd shows the target angle.
 */
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "common/app_types.h"
#include <stdio.h>

static int s_joint;

static void enter(void)  { }
static void exit_(void)  { motion_hold(); }

static void on_event(const app_event_t *e)
{
  switch ((evt_type_t)e->type) {
    case EVT_ENC_STEP: {
      int j = s_joint + (int)e->v[0];
      while (j < 0) j += ARM_N_JOINTS;
      motion_hold_joint(s_joint);
      s_joint = j % ARM_N_JOINTS;
      break;
    }
    case EVT_ENC_PRESS:
      motion_hold_joint(s_joint);
      break;
    default:
      break;
  }
}

static void step(void)
{
  manual_jog(s_joint, -1);
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  joint_vec_t t;
  motion_get_target(&t);
  if (n > 0) snprintf(lines[0], LCD_COLS, "Joint: %s", manual_joint_name(s_joint));
  if (n > 1) snprintf(lines[1], LCD_COLS, "Target: %d deg", (int)t.deg[s_joint]);
  if (n > 2) snprintf(lines[2], LCD_COLS, "Pulse: %u us", servo_deg_to_us(s_joint, t.deg[s_joint]));
  if (n > 3) snprintf(lines[3], LCD_COLS, "Enable: %s", safety_enable_held() ? "HELD" : "RELEASED");
}

const mode_ops_t MODE_SINGLE_JOINT_OPS = {
  .name = "SINGLE JOINT",
  .requires_enable = true,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
