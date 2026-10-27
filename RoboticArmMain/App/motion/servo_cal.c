/* servo_cal.c - calibration table, one row per joint
 *
 * owner: servo / motion member. this is the file you edit while calibrating.
 *
 * us_0 / us_90 / us_180 are the pulse anchors for the two straight-line
 * conversions used by servo.c. J1-J5 keep their measured unloaded bench
 * endpoints. J6 uses the replacement servo's anchors (500 / 1380 / 2300 us);
 * the earlier SG90 anchors and the reported 2490 us maximum are retired. The
 * anchors are NOT installed safe limits and NOT a command range.
 *
 * min_deg / max_deg / home_deg are the confirmed installed values (27 Oct
 * 2026). motion.c enforces the limits and loads home before the first pulse.
 * The home pose is NOT the kinematic zero; joint zeros and signs are still
 * uncalibrated. The independent per-joint limits give endpoint clearance only;
 * they do NOT prove that every whole-arm pose or path is collision-free. J2's
 * lower limit stays at 100 deg: the observed 50 deg case needs a wrist-coupling
 * rule that is not complete, so 50 deg is not enabled here.
 *
 * Physical labels are preserved: MG996R-1/-2/-3, SG92R-1, SG92R-3 and SG90.
 * SG92R-2 is retired; SG92R-3 keeps its name at joint 5 and J6 uses an SG90.
 * J4 is wrist roll and J5 is wrist pitch; this swap is by joint role only, so
 * the servo unit, channel, pulse calibration and home angle of each row stay
 * unchanged.
 *
 * STARTUP STILL SENDS HOME: motion_init() loads home_deg and the first allowed
 * PWM tick writes that pose. Keep every servo signal wire disconnected while
 * bringing up new motion or calibration code.
 */
#include "motion/servo.h"

const servo_cal_t SERVO_CAL[ARM_N_JOINTS] = {
  /* name              0deg  90deg 180deg  min    max    home */
  { "J1 BASE",         500,  1460, 2430,   0.0f, 180.0f, 85.0f },  /* MG996R #1 */
  { "J2 SHOULDER",     500,  1430, 2440, 100.0f, 180.0f, 125.0f }, /* MG996R #2 */
  { "J3 ELBOW",        540,  1530, 2500,  45.0f, 180.0f, 145.0f }, /* MG996R #3 */
  { "J4 WRIST ROLL",   500,  1440, 2370,   0.0f, 180.0f, 90.0f },  /* SG92R #1 */
  { "J5 WRIST PITCH",  640,  1580, 2470,  25.0f, 140.0f, 25.0f },  /* SG92R #3 (stable label) */
  { "J6 GRIPPER",      500,  1380, 2300,   0.0f, 60.0f, 35.0f },   /* SG90 replacement */
};
