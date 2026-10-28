/* mode_manual.c - helpers shared by the three joystick modes */
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "common/app_config.h"
#if APP_USE_INPUT
#include "input/input.h"
#endif

void manual_jog_dir(int joint_x, float sign_x, int joint_y, float sign_y)
{
#if APP_USE_INPUT
  float v = (motion_get_speed() == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
  /* send every pass, also 0, so the jog timeout never fires mid-move */
  if (joint_x >= 0) motion_jog(joint_x, sign_x * input_joy_x() * v);
  if (joint_y >= 0) motion_jog(joint_y, sign_y * input_joy_y() * v);
#else
  (void)joint_x;
  (void)sign_x;
  (void)joint_y;
  (void)sign_y;
#endif
}

void manual_jog(int joint_x, int joint_y)
{
  manual_jog_dir(joint_x, 1.0f, joint_y, 1.0f);
}

const char *manual_joint_name(int joint)
{
  return servo_cal(joint)->name;
}
