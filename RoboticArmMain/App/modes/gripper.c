/* gripper.c - see gripper.h */
#include "modes/gripper.h"
#include "motion/motion.h"
#include "common/app_config.h"
#include "common/app_log.h"
#include "common/app_util.h"

bool gripper_set_deg(float deg)
{
  motion_result_t r = motion_set_joint_target(J6_GRIPPER, deg);
  if (r == MOTION_OK) return true;
  if (r == MOTION_CLAMPED) {
    motion_hold_joint(J6_GRIPPER);       /* do not drive to the clamped end instead */
    LOG("gripper: %d deg is outside the J6 limits, not moved\r\n", (int)deg);
  }
  return false;
}

bool gripper_command(bool close)
{
  return gripper_set_deg(close ? APP_GRIPPER_CLOSED_DEG : APP_GRIPPER_OPEN_DEG);
}

bool gripper_is_closed_at(float deg)
{
  return absf(deg - APP_GRIPPER_CLOSED_DEG) < absf(deg - APP_GRIPPER_OPEN_DEG);
}
