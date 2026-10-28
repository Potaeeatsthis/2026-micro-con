/* lcd_port.c - lcd_port.h on top of the Lab08 ILI9341 driver
 *
 * Landscape 320x240, rotated 180 deg because the panel is mounted upside down. The driver blocks
 * on SPI, so every call is short.
 * Touch: our own read instead of TP_Read_Coordinates() (too slow, portrait only). Several samples
 * are averaged and dropped if they spread too much; a press needs TOUCH_CONFIRM_POLLS good reads.
 */
#include <stddef.h>

#include "lcd/lcd_port.h"
#include "lcd/lcd_assets.h"
#include "lcd/driver/ILI9341_STM32_Driver.h"
#include "lcd/driver/ILI9341_GFX.h"
#include "lcd/driver/ILI9341_Touchscreen.h"
#include "spi.h"

#define TEXT_MAX            40     /* longest string drawn in one call */
#define FONT_W               6     /* CHAR_WIDTH in driver/5x5_font.h */

#define SPI_TIMEOUT_MS     100u
#define ASSET_BUF         1024u    /* bytes per SPI call while decoding a picture */
#define DIGITS_MAX_W        64

#define TOUCH_SAMPLES        8
#define TOUCH_MAX_SPREAD    80
#define TOUCH_CONFIRM_POLLS  2

/* raw touch value at the screen edges (screen x = controller Y channel in this rotation) */
/* swapped after the screen was turned 180 deg, not re-measured yet */
#define TOUCH_RAW_X0       279
#define TOUCH_RAW_X319    3823
#define TOUCH_RAW_Y0       513
#define TOUCH_RAW_Y239    3774

static volatile uint16_t s_raw_x;  /* last raw averages, for calibration */
static volatile uint16_t s_raw_y;
static int s_good;
static int s_x, s_y;

void lcd_port_init(void)
{
  ILI9341_Init();
  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
  ILI9341_Fill_Screen(BLACK);
}

void lcd_port_fill(int x, int y, int w, int h, uint16_t color)
{
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > LCD_WIDTH_PX)  w = LCD_WIDTH_PX - x;
  if (y + h > LCD_HEIGHT_PX) h = LCD_HEIGHT_PX - y;
  if (w <= 0 || h <= 0) return;
  ILI9341_Draw_Rectangle((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
}

void lcd_port_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int size)
{
  if (s == NULL || size < 1 || x < 0 || y < 0 || x >= LCD_WIDTH_PX || y >= LCD_HEIGHT_PX) return;

  /* font is ' '..'~' only; cut at the right edge */
  char buf[TEXT_MAX + 1];
  int max_chars = (LCD_WIDTH_PX - x) / (FONT_W * size);
  if (max_chars > TEXT_MAX) max_chars = TEXT_MAX;
  int n = 0;
  for (; n < max_chars && s[n] != '\0'; n++) {
    char c = s[n];
    buf[n] = (c >= ' ' && c <= '~') ? c : '?';
  }
  buf[n] = '\0';
  if (n == 0) return;
  ILI9341_Draw_Text(buf, (uint16_t)x, (uint16_t)y, fg, (uint16_t)size, bg);
}

static void pixels_begin(void)
{
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
}

static void pixels_send(const uint8_t *d, uint32_t n)
{
  while (n > 0u) {
    uint16_t c = (uint16_t)(n > 32768u ? 32768u : n);
    (void)HAL_SPI_Transmit(HSPI_INSTANCE, (uint8_t *)d, c, SPI_TIMEOUT_MS);
    d += c;
    n -= c;
  }
}

static void pixels_end(void)
{
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static bool inside(int x, int y, int w, int h)
{
  return w > 0 && h > 0 && x >= 0 && y >= 0 && x + w <= LCD_WIDTH_PX && y + h <= LCD_HEIGHT_PX;
}

/* RLE (tools/build_lcd_assets.py): h < 128 -> h + 3 copies of the next pixel,
 * h >= 128 -> h - 127 literal pixels. big-endian RGB565 */
void lcd_port_asset(int id, int x, int y)
{
  static uint8_t buf[ASSET_BUF];
  if (id < 0 || id >= (int)LCD_A_COUNT) return;
  const lcd_asset_t *a = &lcd_assets[id];
  if (!inside(x, y, a->w, a->h)) return;

  ILI9341_Set_Address((uint16_t)x, (uint16_t)y, (uint16_t)(x + a->w - 1), (uint16_t)(y + a->h - 1));
  uint32_t left = (uint32_t)a->w * (uint32_t)a->h;
  const uint8_t *p = a->rle;
  uint32_t n = 0;
  pixels_begin();
  while (left > 0u) {
    uint8_t h = *p++;
    uint32_t cnt = (h < 128u) ? (uint32_t)h + 3u : (uint32_t)h - 127u;
    if (cnt > left) cnt = left;
    uint8_t hi = 0, lo = 0;
    if (h < 128u) { hi = p[0]; lo = p[1]; p += 2; }
    for (uint32_t i = 0; i < cnt; i++) {
      if (n + 2u > ASSET_BUF) {
        pixels_send(buf, n);
        n = 0;
      }
      if (h >= 128u) { hi = *p++; lo = *p++; }
      buf[n++] = hi;
      buf[n++] = lo;
    }
    left -= cnt;
  }
  if (n > 0u) pixels_send(buf, n);
  pixels_end();
}

/* blend fg over bg, alpha 0..255 */
static uint16_t blend565(uint16_t fg, uint16_t bg, uint32_t a)
{
  uint32_t r = (((fg >> 11) & 31u) * a + ((bg >> 11) & 31u) * (255u - a)) / 255u;
  uint32_t g = (((fg >> 5) & 63u) * a + ((bg >> 5) & 63u) * (255u - a)) / 255u;
  uint32_t b = ((fg & 31u) * a + (bg & 31u) * (255u - a)) / 255u;
  uint16_t v = (uint16_t)((r << 11) | (g << 5) | b);
  return (uint16_t)((v << 8) | (v >> 8));
}

void lcd_port_digits(int x_right, int y, int field_w, const char *s, uint16_t fg, uint16_t bg)
{
  static uint16_t buf[DIGITS_MAX_W * LCD_DIGIT_H];
  if (s == NULL) return;
  int n = 0;
  while (s[n] != '\0') n++;
  int x0 = x_right - field_w;
  if (field_w <= 0 || field_w > DIGITS_MAX_W || !inside(x0, y, field_w, LCD_DIGIT_H)) return;

  for (int i = 0; i < field_w * LCD_DIGIT_H; i++) buf[i] = blend565(bg, bg, 0u);
  int start = field_w - n * LCD_DIGIT_W;               /* right-aligned */
  for (int c = 0; c < n; c++) {
    char ch = s[c];
    int g = (ch >= '0' && ch <= '9') ? ch - '0' : (ch == '-' ? 10 : 11);
    for (int row = 0; row < LCD_DIGIT_H; row++) {
      for (int col = 0; col < LCD_DIGIT_W; col++) {
        int px = start + c * LCD_DIGIT_W + col;
        if (px < 0 || px >= field_w) continue;
        buf[row * field_w + px] = blend565(fg, bg, lcd_digit_font[g][row * LCD_DIGIT_W + col]);
      }
    }
  }
  ILI9341_Set_Address((uint16_t)x0, (uint16_t)y, (uint16_t)(x_right - 1), (uint16_t)(y + LCD_DIGIT_H - 1));
  pixels_begin();
  pixels_send((const uint8_t *)buf, (uint32_t)field_w * LCD_DIGIT_H * 2u);
  pixels_end();
}

static bool pen_down(void)
{
  return HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == GPIO_PIN_RESET;
}

/* one 12-bit conversion: 12 data bits + 4 zeros */
static uint16_t read_channel(uint8_t cmd)
{
  TP_Write(cmd);
  return (uint16_t)(TP_Read() >> 4);
}

static int map_axis(int raw, int raw_at_0, int raw_at_max, int max_px)
{
  int px = (raw - raw_at_0) * max_px / (raw_at_max - raw_at_0);
  if (px < 0) px = 0;
  if (px > max_px) px = max_px;
  return px;
}

static bool read_touch(int *x, int *y)
{
  uint16_t min_x = 0xFFFFu, max_x = 0u, min_y = 0xFFFFu, max_y = 0u;
  uint32_t sum_x = 0u, sum_y = 0u;

  HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_RESET);
  for (int i = 0; i < TOUCH_SAMPLES; i++) {
    uint16_t ry = read_channel(CMD_RDY);
    uint16_t rx = read_channel(CMD_RDX);
    sum_x += rx;
    sum_y += ry;
    if (rx < min_x) min_x = rx;
    if (rx > max_x) max_x = rx;
    if (ry < min_y) min_y = ry;
    if (ry > max_y) max_y = ry;
  }
  HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);

  if (!pen_down()) return false;
  if (max_x - min_x > TOUCH_MAX_SPREAD || max_y - min_y > TOUCH_MAX_SPREAD) return false;

  s_raw_x = (uint16_t)(sum_x / TOUCH_SAMPLES);
  s_raw_y = (uint16_t)(sum_y / TOUCH_SAMPLES);
  *x = map_axis(s_raw_y, TOUCH_RAW_X0, TOUCH_RAW_X319, LCD_WIDTH_PX - 1);
  *y = map_axis(s_raw_x, TOUCH_RAW_Y0, TOUCH_RAW_Y239, LCD_HEIGHT_PX - 1);
  return true;
}

bool lcd_port_touch(int *x, int *y)
{
  if (!pen_down()) {
    s_good = 0;
    return false;
  }

  int px, py;
  if (read_touch(&px, &py)) {
    s_x = px;
    s_y = py;
    if (s_good < TOUCH_CONFIRM_POLLS) s_good++;
  } else if (s_good < TOUCH_CONFIRM_POLLS) {
    s_good = 0;
  }
  if (s_good < TOUCH_CONFIRM_POLLS) return false;

  *x = s_x;
  *y = s_y;
  return true;
}
