/* servo_cal_test.h - bench test of the installed home pose and joint limits (off by default: APP_USE_SERVO_CAL_TEST)
 *
 * home and limits come from servo_cal.c (installed values, 27 Oct 2026).
 * at boot each joint goes to its home_deg, one joint every MOTION_BOOT_STAGGER_MS (J6 gripper first, then J1..J5).
 * KY-040 press = next joint (J1..J6, wraps); KY-040 turn = move the selected joint 5 deg per click
 * (clockwise = more), clamped to that joint's min_deg..max_deg plus the J2/J5 coupling from motion.c
 * (J2 below 90 only while J5 >= 65; J5 below 65 only while J2 >= 90); a clamped click shows on the lcd.
 * KY-023 pushed down (then back to centre) = print the current pose on USART3 as servo_cal.c rows.
 * the screen uses lcd_port (turned 180 deg like the main screen). bench test only: never enable it on master.
 *
 * enable in app_config_local.h: APP_USE_SERVO_CAL_TEST 1 with APP_USE_MOTION 0, APP_USE_MODES 0,
 * APP_USE_LCD 0 (this test draws the screen itself) and APP_USE_INPUT 1.
 */
#ifndef APP_SERVO_CAL_TEST_H
#define APP_SERVO_CAL_TEST_H

void servo_cal_test_init(void);   /* after input_init(): servos off, lcd on, first screen */
void servo_cal_test_step(void);   /* main loop: home sequence, encoder events, joystick, redraw on change */

#endif /* APP_SERVO_CAL_TEST_H */
