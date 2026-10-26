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
#include <stdbool.h>

#define LCD_COLS         27   /* 26 chars + NUL per text line */
#define LCD_DETAIL_LINES 4    /* lines each mode can fill */

void lcd_init(void);
void lcd_step(void);

/* short banner such as "OUT OF REACH", "PI NOT RESPONDING", "RELEASE FIRST" */
void lcd_show_message(const char *msg, uint32_t duration_ms);

/* ---------------------------------------------------------------- voice mode hooks (for the voice / camera logic)
 * the screens only DRAW what these functions say; the logic that decides lives elsewhere.
 * all of them are cheap and may be called every main-loop pass (the lcd redraws only on a change). */
typedef enum { LCD_BOX_RED = 0, LCD_BOX_GREEN, LCD_BOX_BLACK, LCD_BOX_BLUE, LCD_BOX_COUNT } lcd_box_t;

/* camera result: draws / removes the check mark on that colour's card (voice mode, camera page) */
void lcd_voice_set_detected(lcd_box_t box, bool detected);

/* colour the user asked for by voice ("pick the blue box"); -1 = nothing picked yet */
void lcd_voice_set_pick(int box);
int  lcd_voice_pick(void);

/* status pop-ups, written for every colour: the box colour comes from lcd_voice_set_pick() */
typedef enum {
  LCD_STATUS_PROCESSING = 1,     /* "Processing ...  Holding : <colour> box   Say Stop"            */
  LCD_STATUS_STOPPED_HOLDING,    /* "Stopped  Holding : <colour> box   Place the block ...  Say Place/drop" */
  LCD_STATUS_STOPPED_EMPTY,      /* "Stopped  Gripper : Empty   Ready to return home   Say Home"   */
  LCD_STATUS_COMPLETE            /* "Complete  Gripper : Empty  Ready to return home   Say Home"   */
} lcd_status_t;
void lcd_show_status(lcd_status_t status);
void lcd_hide_status(void);      /* removes the pop-up and redraws the page below it */

#endif /* APP_LCD_H */
