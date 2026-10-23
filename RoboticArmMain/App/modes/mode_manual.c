/* mode_manual.c - helpers shared by the three joystick modes
 *
 * owner: motion member (changes here affect cartesian, joint group and single joint).
 */
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "common/app_config.h"
#if APP_USE_INPUT
#include "input/input.h"
#endif

void manual_jog(int joint_x, int joint_y)
{
#if APP_USE_INPUT
  float v = (motion_get_speed() == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
  /* sent every pass, also when 0: the jog timeout in motion.c then never fires mid-move */
  if (joint_x >= 0) motion_jog(joint_x, input_joy_x() * v);
  if (joint_y >= 0) motion_jog(joint_y, input_joy_y() * v);
#else
  (void)joint_x;
  (void)joint_y;
#endif
}

const char *manual_joint_name(int joint)
{
  return servo_cal(joint)->name;
}
