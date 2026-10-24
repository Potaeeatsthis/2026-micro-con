/* servo_cal.c - calibration table, one row per joint
 *
 * owner: servo / motion member. this is the file you edit while calibrating.
 *
 * us_0 / us_90 / us_180 are measured pulse widths at 0 / 90 / 180 mechanical
 * degrees with the servo UNLOADED (bench test, one unit at a time). They are
 * the endpoints of the two straight-line conversions used by servo.c; they are
 * NOT installed safe limits and NOT a command range for the assembled arm.
 *
 * min_deg / max_deg / home_deg are still the old 0 / 180 / 90 PLACEHOLDERS:
 * NOT measured, NOT safe installed values, and not a joint zero. Do not move
 * the assembled arm from this table. TODO(servo): measure each joint's safe
 * installed range and home pose, then replace the placeholders.
 *
 * Physical labels are preserved: MG996R-1/-2/-3, SG92R-1, SG92R-3 and SG90.
 * SG92R-2 is retired; SG92R-3 keeps its name at joint 5 and J6 uses an SG90.
 *
 * STARTUP STILL SENDS HOME: motion_init() loads home_deg and the first allowed
 * PWM tick writes that pose. Keep every servo signal wire disconnected while
 * bringing up new motion or calibration code.
 */
#include "motion/servo.h"

const servo_cal_t SERVO_CAL[ARM_N_JOINTS] = {
  /* name              0deg  90deg 180deg  min    max    home  (min/max/home = NOT measured) */
  { "J1 BASE",         500,  1460, 2430,   0.0f, 180.0f, 90.0f },  /* MG996R #1 */
  { "J2 SHOULDER",     500,  1430, 2440,   0.0f, 180.0f, 90.0f },  /* MG996R #2 */
  { "J3 ELBOW",        540,  1530, 2500,   0.0f, 180.0f, 90.0f },  /* MG996R #3 */
  { "J4 WRIST PITCH",  500,  1440, 2370,   0.0f, 180.0f, 90.0f },  /* SG92R #1 */
  { "J5 WRIST ROT",    640,  1580, 2470,   0.0f, 180.0f, 90.0f },  /* SG92R #3 (stable label) */
  { "J6 GRIPPER",      580,  1570, 2500,   0.0f, 180.0f, 90.0f },  /* SG90 */
};
