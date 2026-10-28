/* ik_bench_test.h - one supervised IK move on the bench (APP_USE_IK_BENCH_TEST, local config only)
 *
 * Only this screen calls kin_ik_bench(); every normal IK caller stays off in this build.
 * Boot goes home. Knob press = move to the IK solution of one fixed target (refused if a joint
 * would move too far or motion would clamp it), any press while moving = stop, press again = home.
 * On arrival the screen and USART3 show the FK estimate and its error, to compare with a measurement.
 * The gripper never moves.
 *
 * Local config: APP_USE_IK_BENCH_TEST 1, MOTION 1, INPUT 1, KINEMATICS 1, MODES 0, LCD 0,
 * SERVO_CAL_TEST 0. Override IK_BENCH_X_MM / _Y_MM / _Z_MM / _PITCH_DEG for another target.
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

void ik_bench_test_init(void);
void ik_bench_test_step(void);

/* for the host tests */
ik_bench_state_t ik_bench_test_state(void);
const char      *ik_bench_test_status(void);
void             ik_bench_test_solution(joint_vec_t *out);

#endif /* APP_IK_BENCH_TEST_H */
