/* host fake of App/lcd/lcd_port.c: no screen on a PC, so every call does nothing.
 * the real lcd_port.c talks to the ILI9341 over SPI (HAL), which cannot build on a PC. */
#include "lcd/lcd_port.h"

void lcd_port_init(void) {}

void lcd_port_fill(int x, int y, int w, int h, uint16_t color)
{
  (void)x; (void)y; (void)w; (void)h; (void)color;
}

/* the last strings drawn, so the tests can check a label (e.g. FINE / FAST) */
char fake_lcd_texts[32][32];
int  fake_lcd_text_n;

void lcd_port_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int size)
{
  (void)x; (void)y; (void)fg; (void)bg; (void)size;
  if (s != 0 && fake_lcd_text_n < 32) {
    int i = 0;
    for (; i < 31 && s[i] != 0; i++) fake_lcd_texts[fake_lcd_text_n][i] = s[i];
    fake_lcd_texts[fake_lcd_text_n][i] = 0;
    fake_lcd_text_n++;
  }
}

/* scripted by the tests: where the "finger" is, and which pictures were drawn since the last clear */
int fake_lcd_touch_x, fake_lcd_touch_y, fake_lcd_touch_down;
int fake_lcd_assets[256], fake_lcd_n;

bool lcd_port_touch(int *x, int *y)
{
  if (!fake_lcd_touch_down) return false;
  *x = fake_lcd_touch_x;
  *y = fake_lcd_touch_y;
  return true;
}

void lcd_port_asset(int id, int x, int y)
{
  (void)x; (void)y;
  if (fake_lcd_n < 256) fake_lcd_assets[fake_lcd_n++] = id;
}

void lcd_port_digits(int x_right, int y, int field_w, const char *s, uint16_t fg, uint16_t bg)
{
  (void)x_right; (void)y; (void)field_w; (void)s; (void)fg; (void)bg;
}
