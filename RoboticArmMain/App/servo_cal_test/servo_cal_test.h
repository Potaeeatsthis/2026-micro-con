/* servo_cal_test.h - bench screen for checking the installed home pose and joint limits
 * (off by default: APP_USE_SERVO_CAL_TEST). bench use only, never enabled on master.
 *
 * NOT a passive measurement mode: it runs on motion/, so at boot motion's normal start-up sends every
 * joint to its home_deg (one joint every MOTION_BOOT_STAGGER_MS, gripper first).
 * every move goes through motion_set_joint_target(): motion's speed limit, servo_cal min/max and the
 * J2/J5 coupling apply exactly as in the real firmware; this screen never writes PWM itself.
 *
 * KY-040 press = next joint (J1..J6, wraps). KY-040 turn = the selected joint's target moves by the
 * step per click (clockwise = more). KY-023 press (stick centred) = step 1 <-> 5 deg (starts at 1).
 * a click that motion clamps shows the reason (table MIN/MAX, or the J2/J5 coupling).
 * a latched fatal fault (start-up or later) shows "FAULT <code> - RESET BOARD" and every knob / stick
 * input is ignored, so no target or status changes until the board is reset.
 * KY-023 pushed down, then back to centre = print the current pose on USART3 as servo_cal.c rows:
 * home_deg = the pose, min_deg / max_deg = the servo_cal.c table values (never the coupled bound).
 *
 * app_config_local.h: APP_USE_SERVO_CAL_TEST 1, APP_USE_MOTION 1, APP_USE_MODES 0, APP_USE_LCD 0,
 * APP_USE_INPUT 1 (APP_USE_VOICE / APP_USE_PI may be 0 on the bench).
 */
#ifndef APP_SERVO_CAL_TEST_H
#define APP_SERVO_CAL_TEST_H

void servo_cal_test_init(void);   /* after motion_init() and input_init(): lcd on, first screen */
void servo_cal_test_step(void);   /* main loop, before the event drain: knob / stick, redraw */

/* read-only state for the host tests */
int         servo_cal_test_selected(void);
int         servo_cal_test_step_deg(void);
const char *servo_cal_test_status(void);

#endif /* APP_SERVO_CAL_TEST_H */
