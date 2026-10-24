/* servo_test.h - bench test: all servos to 90, then swing one at a time, say "pick" for the next
 * (branch test/servo-power-test, never for master)
 *
 * owner: Supanut (core). a test module, not a feature.
 * it replaces motion/ for this test (APP_USE_MOTION 0) and writes the PWM itself through
 * motion/servo.h. this breaks AGENTS.md rule 4 on purpose (own start-up order, see below).
 *
 * sequence ("pick" moves to the next stage):
 *   ALL 90 -> J1 -> J2 -> J3 -> J4 -> J5 -> J6 -> ALL 90 -> J1 -> ...
 *   ALL 90: J1..J6 switched on one by one, TEST_STAGGER_MS apart, each jumps to 90 deg
 *           (a servo has no position feedback), then all six hold 90 (easy to label).
 *   Jn:     joint n swings 90 -> 60 -> 120 -> 90 deg forever at TEST_SPEED_DEG_S,
 *           TEST_DWELL_MS after each move; the others hold 90.
 *   "pick" during a swing: joint n moves back to 90 and holds, then the next stage starts.
 *
 * timing: all movement runs in servo_test_tick() (TIM6 isr, 50 Hz), one pulse change per 20 ms
 * servo frame, so the 43 ms cnn in the main loop cannot delay or jerk it. servo_test_step()
 * (main loop) only passes "pick" to the isr and prints the log lines the isr posts as events.
 * with modes off this module is the event consumer (app_events.h normally allows only modes_step).
 *
 * log (USART3, 921600): "J1 -> 60 deg (1140 us)" when a move starts, "J1 at 60 deg (520 ms)" when it ends.
 *
 * turn on in App/common/app_config_local.h (gitignored):
 *     #define APP_USE_SERVO_TEST 1
 *     #define APP_USE_MOTION     0     (motion would drive the servos too)
 *     #define APP_USE_MODES      0
 *
 * safety: when safety_motion_allowed() is false all pulses go off; when it is allowed again the
 * test starts over at ALL 90. no hold-to-enable needed. after an e-stop press the restart latch
 * blocks motion until the board is reset (no lcd to confirm).
 */
#ifndef APP_SERVO_TEST_H
#define APP_SERVO_TEST_H

void servo_test_init(void);
void servo_test_tick(void);     /* TIM6 isr, after safety_tick() */
void servo_test_step(void);     /* main loop */

#endif /* APP_SERVO_TEST_H */
