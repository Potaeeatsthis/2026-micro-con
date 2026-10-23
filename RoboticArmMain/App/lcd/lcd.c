/* lcd.c - text-line screen model
 *
 * layout (size-2 text, 12 px per char, 24 px per line):
 *   line 0      mode tabs  (touch: one tab per mode)
 *   line 1      safety state / speed
 *   line 2..5   mode details (from the active mode's describe())
 *   line 6      last voice word / pi status
 *   line 7      message banner
 *   bottom      touch buttons STOP / UNDO / OK
 *
 * TODO(lcd): nicer layout, graphics, the jog-step slider; keep the pull model and the
 *            per-step line budget.
 */
#include "lcd/lcd.h"
#include "lcd/lcd_port.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "safety/safety.h"
#include "motion/motion.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#if APP_USE_MODES
#include "modes/modes.h"
#endif
#if APP_USE_VOICE
#include "voice/voice.h"
#endif
#if APP_USE_PI
#include "pi_link/pi_link.h"
#endif

#define N_LINES             8
#define LINE_H              24
#define LCD_LINES_PER_STEP  2
#define REFRESH_MS          100u
#define TOUCH_POLL_MS       50u
#define BUTTON_Y            (LCD_HEIGHT_PX - 36)

static char     s_shown[N_LINES][LCD_COLS];   /* what is on the glass */
static char     s_want[N_LINES][LCD_COLS];    /* what should be */
static char     s_msg[LCD_COLS];
static uint32_t s_msg_until;
static uint32_t s_last_refresh, s_last_touch;
static bool     s_touch_down;

/* ---------------------------------------------------------------- touch zones
 * EXTENSION POINT: new touch buttons = one row here + an event in app_events.h */
typedef struct { int x, y, w, h; uint16_t evt; uint16_t arg; const char *label; } touch_zone_t;
static const touch_zone_t ZONES[] = {
  {   0,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_VOICE,        NULL },
  {  80,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_CARTESIAN,    NULL },
  { 160,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_JOINT_GROUP,  NULL },
  { 240,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_SINGLE_JOINT, NULL },
  {   0,  BUTTON_Y, 100, 36, EVT_TOUCH_STOP,    0,                 "STOP" },
  { 110,  BUTTON_Y, 100, 36, EVT_TOUCH_UNDO,    0,                 "UNDO" },
  { 220,  BUTTON_Y, 100, 36, EVT_TOUCH_CONFIRM, 0,                 "OK" },
};

void lcd_show_message(const char *msg, uint32_t duration_ms)
{
  strncpy(s_msg, msg, LCD_COLS - 1);
  s_msg[LCD_COLS - 1] = '\0';
  s_msg_until = HAL_GetTick() + duration_ms;
}

void lcd_init(void)
{
  lcd_port_init();
  lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BLACK);
  for (uint32_t i = 0; i < ARRAY_LEN(ZONES); i++) {
    if (ZONES[i].label != NULL) {
      lcd_port_fill(ZONES[i].x, ZONES[i].y, ZONES[i].w, ZONES[i].h, LCD_GREY);
      lcd_port_text(ZONES[i].x + 8, ZONES[i].y + 10, ZONES[i].label, LCD_WHITE, LCD_GREY, 2);
    }
  }
  memset(s_shown, 0, sizeof(s_shown));
  LOG("lcd: init\r\n");
}

static void build_screen(void)
{
  memset(s_want, 0, sizeof(s_want));

#if APP_USE_MODES
  snprintf(s_want[0], LCD_COLS, "%s%s", modes_name(modes_current()),
           modes_switch_pending() ? " ..." : "");
#else
  snprintf(s_want[0], LCD_COLS, "NO MODES");
#endif

  safety_status_t st;
  safety_get_status(&st);
  snprintf(s_want[1], LCD_COLS, "%s  %s", safety_state_name(st.state),
           motion_get_speed() == SPEED_FAST ? "FAST" : "FINE");

#if APP_USE_MODES
  modes_describe(&s_want[2], LCD_DETAIL_LINES);
#endif

  char voice_txt[14] = "-";
  char pi_txt[12] = "PI -";
#if APP_USE_VOICE
  if (voice_last_word() >= 0) snprintf(voice_txt, sizeof voice_txt, "%s", voice_class_name(voice_last_word()));
#endif
#if APP_USE_PI
  uint32_t rx = pi_link_last_rx_ms();
  snprintf(pi_txt, sizeof pi_txt, "%s", (rx != 0u && ms_since(HAL_GetTick(), rx) < 10000u) ? "PI OK" : "PI ?");
#endif
  snprintf(s_want[6], LCD_COLS, "%s  %s", voice_txt, pi_txt);

  if (s_msg[0] != '\0' && (int32_t)(s_msg_until - HAL_GetTick()) > 0) {
    snprintf(s_want[7], LCD_COLS, "%s", s_msg);
  } else {
    s_msg[0] = '\0';
  }
}

static void draw_changed_lines(void)
{
  int budget = LCD_LINES_PER_STEP;
  for (int i = 0; i < N_LINES && budget > 0; i++) {
    if (strcmp(s_shown[i], s_want[i]) == 0) continue;
    uint16_t fg = (i == 7) ? LCD_YELLOW : LCD_WHITE;
    lcd_port_fill(0, i * LINE_H + (i == 0 ? 0 : 4), LCD_WIDTH_PX, LINE_H, LCD_BLACK);
    lcd_port_text(4, i * LINE_H + (i == 0 ? 4 : 8), s_want[i], fg, LCD_BLACK, 2);
    memcpy(s_shown[i], s_want[i], LCD_COLS);
    budget--;
  }
}

static void poll_touch(void)
{
  int x = 0, y = 0;
  bool down = lcd_port_touch(&x, &y);
  if (down && !s_touch_down) {                  /* act on touch-down only */
    for (uint32_t i = 0; i < ARRAY_LEN(ZONES); i++) {
      const touch_zone_t *z = &ZONES[i];
      if (x >= z->x && x < z->x + z->w && y >= z->y && y < z->y + z->h) {
        (void)app_event_post((evt_type_t)z->evt, z->arg, 0, 0, 0);
        break;
      }
    }
  }
  s_touch_down = down;
}

void lcd_step(void)
{
  uint32_t now = HAL_GetTick();
  if (ms_since(now, s_last_touch) >= TOUCH_POLL_MS) {
    s_last_touch = now;
    poll_touch();
  }
  if (ms_since(now, s_last_refresh) >= REFRESH_MS) {
    s_last_refresh = now;
    build_screen();
  }
  draw_changed_lines();
}
