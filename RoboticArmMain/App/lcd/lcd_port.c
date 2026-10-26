/* lcd_port.c - ILI9341 on SPI5, written directly against HAL (no Lab08 driver needed for this step)
 *
 * owner: lcd member.
 * does:      panel init (landscape 320x240), fill, packed pictures from flash (lcd_assets), digits, touch (XPT2046).
 * does NOT:  text (TODO(lcd): needs a font).
 * pins:      core-config.md section 3: SPI5 PF7/PF8/PF9, LCD_CS / LCD_DC / LCD_RST labels from main.h.
 *            T_CS is left high by CubeMX, so the touch chip stays deselected.
 * blocking:  plain HAL_SPI_Transmit, no DMA, so no DMA_BUF needed.
 */
#include "lcd/lcd_port.h"
#include "lcd/lcd_assets.h"
#include "common/app_log.h"
#include "main.h"
#include "spi.h"

/* MADCTL: 0x28 = landscape (MV + BGR). if the picture is mirrored / upside down try 0xE8 or 0x48 or 0x88;
 * if red and blue are swapped, toggle bit 3 (0x08). */
#define LCD_MADCTL 0x28u

#define SPI_TIMEOUT_MS 100u
#define SPI_MAX_CHUNK  32768u   /* bytes per HAL call (HAL takes a uint16_t count) */

static inline void cs_low(void)  { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); }
static inline void cs_high(void) { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); }
static inline void dc_cmd(void)  { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); }
static inline void dc_data(void) { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); }

static void write_cmd(uint8_t cmd)
{
  dc_cmd(); cs_low();
  (void)HAL_SPI_Transmit(&hspi5, &cmd, 1, SPI_TIMEOUT_MS);
  cs_high();
}

static void write_data(const uint8_t *d, uint32_t n)
{
  dc_data(); cs_low();
  while (n > 0u) {
    uint16_t c = (uint16_t)(n > SPI_MAX_CHUNK ? SPI_MAX_CHUNK : n);
    (void)HAL_SPI_Transmit(&hspi5, (uint8_t *)d, c, SPI_TIMEOUT_MS);
    d += c; n -= c;
  }
  cs_high();
}

static void cmd_args(uint8_t cmd, const uint8_t *args, uint8_t n)
{
  write_cmd(cmd);
  if (n > 0u) write_data(args, n);
}

#define CMD(c, ...) do { static const uint8_t a_[] = { __VA_ARGS__ }; cmd_args((c), a_, (uint8_t)sizeof a_); } while (0)

static void set_window(int x, int y, int w, int h)
{
  uint16_t x1 = (uint16_t)(x + w - 1), y1 = (uint16_t)(y + h - 1);
  uint8_t col[4] = { (uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x1 >> 8), (uint8_t)x1 };
  uint8_t row[4] = { (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y1 >> 8), (uint8_t)y1 };
  cmd_args(0x2A, col, 4);
  cmd_args(0x2B, row, 4);
  write_cmd(0x2C);                      /* memory write: pixel data follows */
}

static bool clip_ok(int x, int y, int w, int h)
{
  return w > 0 && h > 0 && x >= 0 && y >= 0 && x + w <= LCD_WIDTH_PX && y + h <= LCD_HEIGHT_PX;
}

#define ASSET_BUF 1024u   /* bytes sent per SPI call while decoding */

/* RLE format (see tools/build_lcd_assets.py): header byte h, then
 *   h < 128 : (h + 3) identical pixels, 2 bytes follow
 *   h >= 128: (h - 127) literal pixels, 2 bytes each follow
 * pixels are already big-endian RGB565, i.e. in the order the panel wants them */
void lcd_port_asset(int id, int x, int y)
{
  static uint8_t buf[ASSET_BUF];
  if (id < 0 || id >= (int)LCD_A_COUNT) return;
  const lcd_asset_t *a = &lcd_assets[id];
  if (!clip_ok(x, y, a->w, a->h)) return;

  set_window(x, y, a->w, a->h);
  uint32_t left = (uint32_t)a->w * (uint32_t)a->h;      /* pixels still to send */
  const uint8_t *p = a->rle;
  uint32_t n = 0;                                      /* bytes waiting in buf */
  dc_data(); cs_low();
  while (left > 0u) {
    uint8_t h = *p++;
    uint32_t cnt = (h < 128u) ? (uint32_t)h + 3u : (uint32_t)h - 127u;
    if (cnt > left) cnt = left;
    uint8_t hi = 0, lo = 0;
    if (h < 128u) { hi = p[0]; lo = p[1]; p += 2; }
    for (uint32_t i = 0; i < cnt; i++) {
      if (n + 2u > ASSET_BUF) {
        (void)HAL_SPI_Transmit(&hspi5, buf, (uint16_t)n, SPI_TIMEOUT_MS);
        n = 0;
      }
      if (h >= 128u) { hi = *p++; lo = *p++; }
      buf[n++] = hi;
      buf[n++] = lo;
    }
    left -= cnt;
  }
  if (n > 0u) (void)HAL_SPI_Transmit(&hspi5, buf, (uint16_t)n, SPI_TIMEOUT_MS);
  cs_high();
}

#define DIGITS_MAX_W 64   /* widest field, pixels */

/* blend one 565 colour channel set: fg over bg with alpha 0..255, result already byte-swapped for SPI */
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
  int n = 0;
  while (s[n] != '\0') n++;
  int x0 = x_right - field_w;
  if (field_w <= 0 || field_w > DIGITS_MAX_W || !clip_ok(x0, y, field_w, LCD_DIGIT_H)) return;
  for (int i = 0; i < field_w * LCD_DIGIT_H; i++) buf[i] = blend565(bg, bg, 0u);
  int start = field_w - n * LCD_DIGIT_W;             /* right-aligned; text longer than the field loses its left end */
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
  set_window(x0, y, field_w, LCD_DIGIT_H);
  write_data((const uint8_t *)buf, (uint32_t)field_w * LCD_DIGIT_H * 2u);
}

void lcd_port_init(void)
{
  cs_high();
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);

  write_cmd(0x01);                      /* software reset */
  HAL_Delay(120);
  CMD(0xCF, 0x00, 0xC1, 0x30);
  CMD(0xED, 0x64, 0x03, 0x12, 0x81);
  CMD(0xE8, 0x85, 0x00, 0x78);
  CMD(0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02);
  CMD(0xF7, 0x20);
  CMD(0xEA, 0x00, 0x00);
  CMD(0xC0, 0x23);                      /* power control 1 */
  CMD(0xC1, 0x10);                      /* power control 2 */
  CMD(0xC5, 0x3E, 0x28);                /* VCOM 1 */
  CMD(0xC7, 0x86);                      /* VCOM 2 */
  CMD(0x36, LCD_MADCTL);                /* orientation + colour order */
  CMD(0x3A, 0x55);                      /* 16 bit / pixel (RGB565) */
  CMD(0xB1, 0x00, 0x18);                /* frame rate */
  CMD(0xB6, 0x08, 0x82, 0x27);          /* display function */
  CMD(0xF2, 0x00);                      /* 3-gamma off */
  CMD(0x26, 0x01);                      /* gamma curve 1 */
  CMD(0xE0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00);
  CMD(0xE1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);
  write_cmd(0x11);                      /* sleep out */
  HAL_Delay(120);
  write_cmd(0x29);                      /* display on */
  HAL_Delay(20);

  lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BLACK);
}

void lcd_port_fill(int x, int y, int w, int h, uint16_t color)
{
  static uint8_t row[LCD_WIDTH_PX * 2];
  if (!clip_ok(x, y, w, h)) return;
  for (int i = 0; i < w; i++) {
    row[2 * i]     = (uint8_t)(color >> 8);
    row[2 * i + 1] = (uint8_t)color;
  }
  set_window(x, y, w, h);
  dc_data(); cs_low();
  for (int r = 0; r < h; r++) {
    (void)HAL_SPI_Transmit(&hspi5, row, (uint16_t)(w * 2), SPI_TIMEOUT_MS);
  }
  cs_high();
}

void lcd_port_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int size)
{
  /* TODO(lcd): needs a bitmap font */
  (void)x; (void)y; (void)s; (void)fg; (void)bg; (void)size;
}

/* ---------------------------------------------------------------- touch (XPT2046, bit-banged)
 * wiring (core-config.md section 3): T_CLK PC11, T_CS PC12, T_DIN PD2 (= T_MOSI), T_DO PG2 (= T_MISO),
 * T_IRQ PG3 (low while pressed, internal pull-up).
 *
 * CALIBRATION (do this once): set LCD_TOUCH_DEBUG 1, build, run, open the USART3 log (921600) and tap
 * the four screen corners. each press prints "touch raw A B -> x y". then fill in the four pairs below:
 *   RAW0 = raw value when you touch the left / top edge,  RAW1 = raw value at the right / bottom edge.
 * if x moves when you move your finger up/down, flip LCD_TOUCH_SWAP_XY. a RAW0 bigger than RAW1 is fine
 * (that just means the axis runs backwards). set LCD_TOUCH_DEBUG back to 0 afterwards. */
#define LCD_TOUCH_DEBUG   1
#define LCD_TOUCH_SWAP_XY 1        /* 1: screen x comes from the chip's Y channel (typical for landscape) */
#define LCD_TOUCH_SX_RAW0 3900     /* raw at screen x = 0   (left edge)   */
#define LCD_TOUCH_SX_RAW1 200      /* raw at screen x = 319 (right edge)  */
#define LCD_TOUCH_SY_RAW0 3900     /* raw at screen y = 0   (top edge)    */
#define LCD_TOUCH_SY_RAW1 200      /* raw at screen y = 239 (bottom edge) */
#define LCD_TOUCH_DELAY   40       /* clock half-period (busy loop). XPT2046 max clock is 2 MHz */

#define XPT_CMD_X 0xD0u            /* start | channel X | 12 bit | differential | power-down between reads */
#define XPT_CMD_Y 0x90u

static inline void t_delay(void) { for (volatile int i = 0; i < LCD_TOUCH_DELAY; i++) { } }
static inline void t_clk(int v)  { HAL_GPIO_WritePin(T_CLK_GPIO_Port, T_CLK_Pin, v ? GPIO_PIN_SET : GPIO_PIN_RESET); }
static inline void t_mosi(int v) { HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, v ? GPIO_PIN_SET : GPIO_PIN_RESET); }
static inline int  t_miso(void)  { return HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin) == GPIO_PIN_SET; }

static uint16_t xpt_read(uint8_t cmd)
{
  uint16_t v = 0;
  for (int i = 7; i >= 0; i--) {                         /* control byte, MSB first, chip samples on rising edge */
    t_mosi((cmd >> i) & 1u); t_clk(1); t_delay(); t_clk(0); t_delay();
  }
  t_clk(1); t_delay(); t_clk(0); t_delay();              /* busy / null bit */
  for (int i = 0; i < 12; i++) {                         /* 12 data bits, MSB first */
    t_clk(1); t_delay();
    v = (uint16_t)((v << 1) | (uint16_t)t_miso());
    t_clk(0); t_delay();
  }
  return v;
}

static uint16_t median3(uint16_t a, uint16_t b, uint16_t c)
{
  if (a > b) { uint16_t t = a; a = b; b = t; }
  if (b > c) { b = c; }
  return a > b ? a : b;
}

static int map_axis(int raw, int raw0, int raw1, int size)
{
  int v = (int)(((int32_t)(raw - raw0) * (size - 1)) / (raw1 - raw0));
  return v < 0 ? 0 : (v > size - 1 ? size - 1 : v);
}

bool lcd_port_touch(int *x, int *y)
{
  static bool s_was_down;
  bool down = HAL_GPIO_ReadPin(T_IRQ_GPIO_Port, T_IRQ_Pin) == GPIO_PIN_RESET;   /* T_IRQ low = pressed */
  if (!down) { s_was_down = false; return false; }

  uint16_t rx[3], ry[3];
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);
  for (int i = 0; i < 3; i++) { rx[i] = xpt_read(XPT_CMD_X); ry[i] = xpt_read(XPT_CMD_Y); }
  t_clk(0);
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);

  int ax = median3(rx[0], rx[1], rx[2]);
  int ay = median3(ry[0], ry[1], ry[2]);
  if (ax < 50 || ax > 4050 || ay < 50 || ay > 4050) return false;          /* not a real reading */

#if LCD_TOUCH_SWAP_XY
  int px = map_axis(ay, LCD_TOUCH_SX_RAW0, LCD_TOUCH_SX_RAW1, LCD_WIDTH_PX);
  int py = map_axis(ax, LCD_TOUCH_SY_RAW0, LCD_TOUCH_SY_RAW1, LCD_HEIGHT_PX);
#else
  int px = map_axis(ax, LCD_TOUCH_SX_RAW0, LCD_TOUCH_SX_RAW1, LCD_WIDTH_PX);
  int py = map_axis(ay, LCD_TOUCH_SY_RAW0, LCD_TOUCH_SY_RAW1, LCD_HEIGHT_PX);
#endif
#if LCD_TOUCH_DEBUG
  if (!s_was_down) LOG("touch raw %d %d -> %d %d\r\n", ax, ay, px, py);
#endif
  s_was_down = true;
  *x = px;
  *y = py;
  return true;
}
