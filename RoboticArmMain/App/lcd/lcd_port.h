/* lcd_port.h - the only bridge between our screen code and the Lab08 ILI9341 driver
 *
 * owner: lcd member.
 * copy the Lab08 driver (from the Lab08 class project) into App/lcd/driver/:
 * ILI9341_STM32_Driver.c/.h, ILI9341_GFX.c/.h, ILI9341_Touchscreen.c/.h, 5x5_font.h. then
 *   - rename CS_Pin / DC_Pin / RST_Pin (and their _GPIO_Port) to the labels LCD_CS / LCD_DC / LCD_RST
 *   - change the touch port defines from GPIOE to the labels T_CLK, T_CS, T_MOSI, T_MISO, T_IRQ
 *   - make sure the driver uses hspi5
 *   - fill in lcd_port.c; nothing else in App/ includes the driver directly.
 * CI does not count warnings inside App/lcd/driver/ (third-party code); lcd_port.c and lcd.c must have none. lcd.c never includes the driver directly, so the driver can be
 * swapped (e.g. for a DMA version) without touching the screen logic.
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

/* raw touch: true while pressed, x/y already mapped to screen pixels */
bool lcd_port_touch(int *x, int *y);

/* draw a packed picture from lcd_assets.h (id = LCD_A_xxx) with its top-left corner at x, y.
 * pictures are RLE-compressed in flash and decoded while they stream to the panel (no RAM buffer).
 * blocking: about 30 ms for the whole screen, about 1 ms for a button. */
void lcd_port_asset(int id, int x, int y);

/* big bold numbers: s may hold 0-9, '-' and ' '. drawn right-aligned so the last char ends at x_right,
 * on a field_w pixel wide field filled with bg first (anti-aliased against fg / bg) */
void lcd_port_digits(int x_right, int y, int field_w, const char *s, uint16_t fg, uint16_t bg);

#endif /* APP_LCD_PORT_H */
