/* servo_cal.c - calibration table, one row per joint
 *
 * owner: servo / motion member. this is the file you edit while calibrating.
 * every row starts from the SG92R bench test (616 / 1523 / 2400 us).
 * TODO(servo): measure each fitted servo, set its safe min/max on the real arm,
 *              and note the servo model (MG996R / SG92R) in the comment.
 */
#include "motion/servo.h"

const servo_cal_t SERVO_CAL[ARM_N_JOINTS] = {
  /* name              0deg  90deg 180deg  min    max    home */
  { "J1 BASE",         616,  1523, 2400,   0.0f, 180.0f, 90.0f },  /* TODO(servo): model? */
  { "J2 SHOULDER",     616,  1523, 2400,   0.0f, 180.0f, 90.0f },
  { "J3 ELBOW",        616,  1523, 2400,   0.0f, 180.0f, 90.0f },
  { "J4 WRIST PITCH",  616,  1523, 2400,   0.0f, 180.0f, 90.0f },
  { "J5 WRIST ROT",    616,  1523, 2400,   0.0f, 180.0f, 90.0f },
  { "J6 GRIPPER",      616,  1523, 2400,   0.0f, 180.0f, 90.0f },
};
