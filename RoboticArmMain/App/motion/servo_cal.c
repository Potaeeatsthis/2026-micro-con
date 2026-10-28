/* servo_cal.c - calibration table, one row per joint (edit this file when calibrating)
 *
 * us_0 / us_90 / us_180 are the pulse anchors servo.c interpolates between; they are not limits.
 * min / max / home are the installed values. Home is not the kinematic zero.
 * J2's floor is 50 deg, but motion.c only lets it below 90 while J5 >= 65 (floor contact seen
 * around 90-95 with J5 = 25). These are endpoint limits only, not a collision check for paths.
 * J4 = wrist roll, J5 = wrist pitch.
 *
 * Boot sends every joint home (one at a time, gripper first): keep the servo signal wires
 * unplugged when bringing up new motion code.
 */
#include "motion/servo.h"

const servo_cal_t SERVO_CAL[ARM_N_JOINTS] = {
  /* name              0deg  90deg 180deg  min    max    home */
  { "J1 BASE",         500,  1460, 2430,   0.0f, 180.0f, 85.0f },  /* MG996R #1 */
  { "J2 SHOULDER",     500,  1430, 2440,  50.0f, 180.0f, 125.0f }, /* MG996R #2, dynamic 90 while J5 low */
  { "J3 ELBOW",        540,  1530, 2500,  45.0f, 180.0f, 145.0f }, /* MG996R #3 */
  { "J4 WRIST ROLL",   500,  1440, 2370,   0.0f, 180.0f, 90.0f },  /* SG92R #1 */
  { "J5 WRIST PITCH",  640,  1580, 2470,  25.0f, 140.0f, 25.0f },  /* SG92R #3 (stable label) */
  { "J6 GRIPPER",      500,  1380, 2300,   0.0f, 60.0f, 40.0f },   /* SG90 (maybe SG92R) */
};
