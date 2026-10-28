/* modes.h - active mode + safe mode switching
 *
 * modes_step() is the only consumer of the event queue: it handles the global events (mode tabs,
 * fine/fast) and passes the rest to the active mode.
 */
#ifndef APP_MODES_H
#define APP_MODES_H

#include <stdbool.h>
#include "common/app_types.h"
#include "lcd/lcd.h"

void        modes_init(void);
void        modes_step(void);

arm_mode_t  modes_current(void);
const char *modes_name(arm_mode_t m);
bool        modes_switch_pending(void);
void        modes_request(arm_mode_t m);     /* starts the safe switching procedure */

/* the joint / group the joystick moves (knob and LCD arrows change it) */
int         mode_single_joint_selected(void);   /* 0..5 = J1..J6 */
int         mode_joint_group_selected(void);    /* 0..2 = J1/J2, J3/J4, J5/J6 */

/* LCD text lines from the active mode */
void        modes_describe(char (*lines)[LCD_COLS], int n);

#endif /* APP_MODES_H */
