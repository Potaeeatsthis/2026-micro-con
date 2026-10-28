/* ik_bench_test.h - one supervised IK move on the bench (off by default: APP_USE_IK_BENCH_TEST).
 * bench use only, never enabled on master. it is how the first IK solution is checked on the real arm
 * while IK stays switched off for everything else.
 *
 * IK stays off: this build does NOT change ARM_GEOMETRY_MEASURED. kin_ik() still returns KIN_NOT_READY;
 * only this screen calls kin_ik_bench() (the same solver without the guard), for ONE fixed target.
 *
 * it runs on motion/ like the servo-cal screen: at boot motion sends every joint home (one joint every
 * MOTION_BOOT_STAGGER_MS, gripper first); every move is motion_set_target_exact() at FINE speed, so
 * motion's speed limit, servo_cal min/max and the J2/J5 coupling apply, and a target motion would clamp
 * is not started at all. the gripper (J6) is never moved.
 *
 *   home, screen shows the target, the IK joint angles, the biggest joint change, "PRESS = GO"
 *                              (arming: GO needs a press whose debounced EDGE is at least IK_BENCH_ARM_MS
 *                              after READY, judged by the edge time, not by when the event is handled.
 *                              a knob held across start-up counts only after release + a new press)
 *   KY-040 press            -> move to the IK solution (refused if a joint would move more than
 *                              IK_BENCH_MAX_DELTA_DEG, or motion would clamp it)
 *   any press while moving  -> STOP (motion_hold); KY-023 press counts only with the stick centred
 *   arrived                 -> the screen and USART3 show the FK estimate of the reached pose and its
 *                              error to the target: measure the real tool point and compare
 *   KY-040 press            -> back to home, then ready again
 *
 * app_config_local.h: APP_USE_IK_BENCH_TEST 1, APP_USE_MOTION 1, APP_USE_MODES 0, APP_USE_LCD 0,
 * APP_USE_INPUT 1, APP_USE_KINEMATICS 1, APP_USE_SERVO_CAL_TEST 0 (APP_USE_VOICE / APP_USE_PI may be 0).
 * slower than FINE (10 deg/s): also #define APP_SPEED_FINE_DEG_S 5.0f there.
 * other target: #define IK_BENCH_X_MM / _Y_MM / _Z_MM / _PITCH_DEG there (robot frame, kinematics.h).
 */
#ifndef APP_IK_BENCH_TEST_H
#define APP_IK_BENCH_TEST_H

#include "common/app_types.h"

typedef enum {
  IKB_BOOT = 0,      /* motion start-up, input ignored */
  IKB_READY,         /* solution found and allowed: press = go */
  IKB_NO_SOLUTION,   /* ik refused it, or it is too far / would clamp: press = try again */
  IKB_MOVING,
  IKB_AT_TARGET,     /* press = home */
  IKB_RETURNING,
  IKB_STOPPED,       /* press = home */
  IKB_FAULT          /* fatal fault latched: reset the board */
} ik_bench_state_t;

void ik_bench_test_init(void);   /* after motion_init(), input_init() and kin_init(): lcd on */
void ik_bench_test_step(void);   /* main loop, before the event drain: knob / stick, redraw */

/* read-only state for the host tests */
ik_bench_state_t ik_bench_test_state(void);
const char      *ik_bench_test_status(void);
void             ik_bench_test_solution(joint_vec_t *out);   /* last IK solution (valid in READY and after) */

#endif /* APP_IK_BENCH_TEST_H */
