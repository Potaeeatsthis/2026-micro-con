/* host fake of App/lcd/lcd_port.c: no screen on a PC, so every call does nothing.
 * the real lcd_port.c includes the Lab08 ILI9341 driver (HAL, SPI5), which cannot build on a PC. */
#include "lcd/lcd_port.h"

void lcd_port_init(void) {}

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
