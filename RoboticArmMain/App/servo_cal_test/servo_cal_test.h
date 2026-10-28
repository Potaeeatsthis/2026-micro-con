/* servo_cal_test.h - bench screen for checking the home pose and joint limits
 * (APP_USE_SERVO_CAL_TEST, local config only). It moves the arm through motion/, so all the normal
 * limits apply; boot goes home.
 *
 * knob press = next joint, knob turn = move it by the step, stick press = step 1 <-> 5 deg.
 * stick down then centre = print the pose on USART3 as servo_cal.c rows.
 * A fatal fault shows FAULT and ignores all input until reset.
 *
 * Local config: APP_USE_SERVO_CAL_TEST 1, MOTION 1, INPUT 1, MODES 0, LCD 0.
 */
#ifndef APP_SERVO_CAL_TEST_H
#define APP_SERVO_CAL_TEST_H

void servo_cal_test_init(void);   /* after motion_init() and input_init(): lcd on, first screen */
void servo_cal_test_step(void);   /* main loop, before the event drain: knob / stick, redraw */

/* for the host tests */
int         servo_cal_test_selected(void);
int         servo_cal_test_step_deg(void);
const char *servo_cal_test_status(void);

#endif /* APP_SERVO_CAL_TEST_H */
