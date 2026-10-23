/* lcd.h - status screen + touch controls (guide section 16)
 *
 * owner: lcd member.
 * the lcd PULLS what it shows from getters (modes, safety, voice, pi) and redraws only
 * lines that changed. other modules never draw; at most they call lcd_show_message().
 * the mode-specific lines come from each mode's describe() function (modes/).
 *
 * time budget: lcd_step() redraws at most LCD_LINES_PER_STEP lines per call, so one
 * pass stays short even with a blocking SPI driver.
 */
#ifndef APP_LCD_H
#define APP_LCD_H

#include <stdint.h>

#define LCD_COLS         27   /* 26 chars + NUL per text line */
#define LCD_DETAIL_LINES 4    /* lines each mode can fill */

void lcd_init(void);
void lcd_step(void);

/* short banner such as "OUT OF REACH", "PI NOT RESPONDING", "RELEASE FIRST" */
void lcd_show_message(const char *msg, uint32_t duration_ms);

#endif /* APP_LCD_H */
