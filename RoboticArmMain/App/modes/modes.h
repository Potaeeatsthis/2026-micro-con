/* modes.h - mode state machine + safe mode switching (guide sections 6, 7, 9)
 *
 * owner: core for modes.c; each mode file has its own owner (see TEAM_GUIDE.md).
 * modes_step() is the ONLY consumer of the event queue. it handles the global events
 * (MODE button, mode tabs, fine/fast, restart confirm) and forwards the rest to the
 * active mode's on_event().
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

/* fills `n` text lines for the lcd from the active mode (or the switching status) */
void        modes_describe(char (*lines)[LCD_COLS], int n);

#endif /* APP_MODES_H */
