/* lcd_port.h - the only bridge between our screen code and the Lab08 ILI9341 driver
 *
 * owner: lcd member.
 * copy the Lab08 driver into App/lcd/driver/ (ILI9341_STM32_Driver.c/.h, ILI9341_GFX.c/.h,
 * ILI9341_Touchscreen.c/.h, 5x5_font.h), rename CS_Pin/DC_Pin/RST_Pin to the new labels
 * LCD_CS / LCD_DC / LCD_RST and the touch port defines to the new pins, then fill in
 * lcd_port.c. lcd.c never includes the driver directly, so the driver can be
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

#endif /* APP_LCD_PORT_H */
