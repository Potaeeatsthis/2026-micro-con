/* lcd_port.c - TODO(lcd): connect these to the Lab08 driver in App/lcd/driver/
 *
 * until then every function is an empty stub, so the project builds and runs without
 * a screen. typical bodies once the driver is in:
 *   lcd_port_init:  ILI9341_Init(); ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1); fill black
 *   lcd_port_fill:  ILI9341_Draw_Filled_Rectangle_Coord / ILI9341_Draw_Rectangle(x, y, w, h, color)
 *   lcd_port_text:  ILI9341_Draw_Text(s, x, y, fg, size, bg)
 *   lcd_port_touch: TP_Touchpad_Pressed() + TP_Read_Coordinates(), then map raw -> pixels
 * keep each call short: lcd_step() must stay well under ~20 ms per pass.
 */
#include "lcd/lcd_port.h"

void lcd_port_init(void)
{
}

void lcd_port_fill(int x, int y, int w, int h, uint16_t color)
{
  (void)x; (void)y; (void)w; (void)h; (void)color;
}

void lcd_port_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int size)
{
  (void)x; (void)y; (void)s; (void)fg; (void)bg; (void)size;
}

bool lcd_port_touch(int *x, int *y)
{
  (void)x; (void)y;
  return false;
}
