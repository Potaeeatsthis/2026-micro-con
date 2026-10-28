/* lcd.h - touch screen
 *
 * The LCD pulls what it shows from getters and redraws only what changed. Other modules don't
 * draw; they call lcd_show_message() or the voice hooks below.
 */
#ifndef APP_LCD_H
#define APP_LCD_H

#include <stdint.h>
#include <stdbool.h>

#define LCD_COLS         27   /* 26 chars + NUL per text line */
#define LCD_DETAIL_LINES 4    /* lines each mode can fill */

void lcd_init(void);
void lcd_step(void);

/* short banner like "OUT OF REACH" */
void lcd_show_message(const char *msg, uint32_t duration_ms);

/* ---------------------------------------------------------------- voice page hooks
 * cheap, can be called every pass */
typedef enum { LCD_BOX_RED = 0, LCD_BOX_GREEN, LCD_BOX_BLACK, LCD_BOX_BLUE, LCD_BOX_COUNT } lcd_box_t;

/* tick / no tick on a colour's card (camera result) */
void lcd_voice_set_detected(lcd_box_t box, bool detected);

/* colour asked for by voice, -1 = none */
void lcd_voice_set_pick(int box);
int  lcd_voice_pick(void);

/* jog queue pop-up text, e.g. "[forward 1.25, right 1.10]". "" or NULL hides it */
void lcd_voice_set_queue(const char *text);
const char *lcd_voice_queue(void);   /* for the host tests */

/* status pop-ups (colour from lcd_voice_set_pick()) */
typedef enum {
  LCD_STATUS_PROCESSING = 1,     /* "Processing ...  Holding : <colour> box   Say Stop"            */
  LCD_STATUS_STOPPED_HOLDING,    /* "Stopped  Holding : <colour> box   Place the block ...  Say Place/drop" */
  LCD_STATUS_STOPPED_EMPTY,      /* "Stopped  Gripper : Empty   Ready to return home   Say Home"   */
  LCD_STATUS_COMPLETE            /* "Complete  Gripper : Empty  Ready to return home   Say Home"   */
} lcd_status_t;
void lcd_show_status(lcd_status_t status);
void lcd_hide_status(void);

/* for the host tests */
int  lcd_status_shown(void);     /* 0 = none */
bool lcd_voice_detected(lcd_box_t box);

#endif /* APP_LCD_H */
