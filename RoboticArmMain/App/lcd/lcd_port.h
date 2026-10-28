/* lcd_port.h - the only bridge between our screen code and the Lab08 ILI9341 driver (App/lcd/driver/)
 *
 * The driver's pins were renamed to the CubeMX labels (LCD_CS / LCD_DC / LCD_RST, T_CLK ...) and it
 * uses hspi5. Nothing else includes the driver, so it can be swapped without touching lcd.c.
 */
#ifndef APP_LCD_PORT_H
#define APP_LCD_PORT_H

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH_PX 320
#define LCD_HEIGHT_PX 240

/* rgb565 */
#define LCD_BLACK  0x0000u
#define LCD_WHITE  0xFFFFu
#define LCD_RED    0xF800u
#define LCD_GREEN  0x07E0u
#define LCD_YELLOW 0xFFE0u
#define LCD_GREY   0x8410u

void lcd_port_init(void);
void lcd_port_fill(int x, int y, int w, int h, uint16_t color);
void lcd_port_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int size);

/* true while pressed, x / y in screen pixels */
bool lcd_port_touch(int *x, int *y);

/* draw an RLE picture from lcd_assets.h at x, y. blocking: ~30 ms full screen, ~1 ms a button */
void lcd_port_asset(int id, int x, int y);

/* big digits ('0'-'9', '-', ' '), right-aligned at x_right on a field_w wide field */
void lcd_port_digits(int x_right, int y, int field_w, const char *s, uint16_t fg, uint16_t bg);

#endif /* APP_LCD_PORT_H */
