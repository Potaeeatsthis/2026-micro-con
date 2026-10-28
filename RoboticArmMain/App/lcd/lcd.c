/* lcd.c - touch screen pages
 *
 * HOME -> SELECT MODE -> one page per mode (single, group, cartesian, voice). A mode card switches
 * the mode right away. The voice page is driven by mode_voice through the lcd.h hooks.
 * Pop-ups (confirm, status) sit on top of the page and take the touch while open.
 * Pages are built from the RLE pictures in lcd_assets; to add one, add its pictures in
 * tools/build_lcd_assets.py, a draw function, a zone table and a case in show_screen() / poll_touch().
 */
#include "lcd/lcd.h"
#include "lcd/lcd_port.h"
#include "lcd/lcd_assets.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "safety/safety.h"
#include "motion/motion.h"
#include "kinematics/kinematics.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#if APP_USE_MODES
#include "modes/modes.h"
#include "modes/mode_cartesian.h"
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

typedef enum { SCR_HOME = 0, SCR_SELECT, SCR_STATUS, SCR_SINGLE, SCR_GROUP, SCR_CART, SCR_VOICE, SCR_DEST, SCR_CUSTOM } screen_t;
typedef enum {
  ACT_NONE = 0, ACT_SERVO_PREV, ACT_SERVO_NEXT, ACT_GROUP_PREV, ACT_GROUP_NEXT,
  ACT_CONFIRM_PICK,
  ACT_CONFIRM_DROP_DEST, ACT_CONFIRM_DROP_NOW, ACT_CONFIRM_CUSTOM,
  ACT_BOX_TOGGLE, ACT_PICK_CYCLE
} action_t;
typedef enum { CF_NONE = 0, CF_PICK, CF_DROP_DEST, CF_DROP_NOW, CF_CUSTOM } confirm_t;
typedef enum { OV_NONE = 0, OV_CONFIRM, OV_STATUS } overlay_t;

/* old debug / touch-flow switches, off: mode_voice drives the voice page now */
#define DEBUG_VOICE_TOUCH 0
#define DEBUG_STATUS_TAP  0
#define VOICE_TOUCH_FLOW  0

#define LCD_BG           0x0842u
#define LCD_ROW_BG       0x0882u
#define SINGLE_VALUE_X   100
#define SINGLE_VALUE_W   50
#define SINGLE_VALUES_MS 250u
#define SPEED_TAG_X      LCD_POS_BTN_PROCEED_X
#define SPEED_TAG_Y      LCD_POS_BTN_PROCEED_Y
#define SPEED_TAG_W      78
#define SPEED_TAG_H      28
#define CART_ENC_X       100
#define CART_ENC_Y       LCD_POS_BTN_BACK_Y
#define CART_ENC_W       118
#define CART_ENC_H       28
#define SERVO_COUNT      6
#define GROUP_COUNT      3
#define NAV_NONE 0xFFu

static screen_t s_screen;
static char     s_shown[N_LINES][LCD_COLS];
static char     s_want[N_LINES][LCD_COLS];
static char     s_msg[LCD_COLS];
static uint32_t s_msg_until;
static uint32_t s_last_refresh, s_last_touch;
static bool     s_touch_down;
static int      s_servo;
static int      s_group;
static char     s_val_shown[3][8];
static int      s_speed_shown = -1;           /* -1 = not drawn */
static int      s_cart_enc_shown = -1;
static int      s_label_x, s_label_y;
static char     s_label_shown[LCD_COLS];
#if !VOICE_TOUCH_FLOW
static char     s_vinfo_shown[2][40];
#endif
static uint32_t s_last_values;
static overlay_t    s_ov;
static confirm_t    s_conf;
static lcd_status_t s_status;
static int      s_pick = -1;                  /* -1 = none */
static int      s_pick_drawn = -2;
static bool     s_detected[LCD_BOX_COUNT];
static int8_t   s_det_drawn[LCD_BOX_COUNT];
static int      s_dist = 2;
static int      s_jog01 = 200;                /* 0.1 mm */
static int      s_vspeed_shown = -1;
#define QUEUE_TXT_LEN   160
static char     s_queue_txt[QUEUE_TXT_LEN];   /* "" = hidden */
static char     s_queue_drawn[QUEUE_TXT_LEN];
static bool     s_queue_on_glass;

#define SLIDER_X0        (LCD_POS_SLIDER_X + 6)
#define SLIDER_TRACK_Y   (LCD_POS_SLIDER_Y + 9)
#define DIST_MIN         1
#define DIST_MAX         20
/* ---------------------------------------------------------------- touch zones, one table per page
 * a zone can post an event, open a page and/or run an action */
typedef struct { int x, y, w, h; uint16_t evt; uint16_t arg; uint8_t nav; uint8_t act; const char *label; } touch_zone_t;

static const touch_zone_t HOME_ZONES[] = {
  {  19, 139, 172,  48, 0, 0, SCR_SELECT, ACT_NONE, NULL },                                    /* Start */
};
static const touch_zone_t SELECT_ZONES[] = {
  {  21,  16, 132,  93, EVT_TOUCH_MODE_TAB, MODE_SINGLE_JOINT, SCR_SINGLE, ACT_NONE, NULL },   /* Single    */
  { 170,  16, 132,  93, EVT_TOUCH_MODE_TAB, MODE_JOINT_GROUP,  SCR_GROUP,  ACT_NONE, NULL },   /* Group     */
  {  21, 129, 132,  93, EVT_TOUCH_MODE_TAB, MODE_CARTESIAN,    SCR_CART,   ACT_NONE, NULL },   /* Cartesian */
  { 170, 129, 132,  93, EVT_TOUCH_MODE_TAB, MODE_VOICE,        SCR_VOICE,  ACT_NONE, NULL },   /* Voice     */
};
static const touch_zone_t SINGLE_ZONES[] = {
  {  14,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_PREV, NULL },
  {  80,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_NEXT, NULL },
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t GROUP_ZONES[] = {
  {  14,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_PREV, NULL },
  {  96,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_NEXT, NULL },
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t CART_ZONES[] = {
  { CART_ENC_X, CART_ENC_Y, CART_ENC_W, CART_ENC_H, EVT_TOUCH_SELECT, 0, NAV_NONE, ACT_NONE, NULL }, /* Z / PITCH */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t VOICE_ZONES[] = {
  {  23,  73, 131, 35, 0, 0, NAV_NONE,   ACT_BOX_TOGGLE, NULL },
  { 162,  73, 131, 35, 0, 1, NAV_NONE,   ACT_BOX_TOGGLE, NULL },
  {  23, 112, 131, 35, 0, 2, NAV_NONE,   ACT_BOX_TOGGLE, NULL },
  { 162, 112, 131, 35, 0, 3, NAV_NONE,   ACT_BOX_TOGGLE, NULL },
  {  14, 157, 289, 40, 0, 0, NAV_NONE,   ACT_PICK_CYCLE, NULL },
  {  14, 201,  78, 28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                               /* Back     */
#if VOICE_TOUCH_FLOW
  { 224, 201,  78, 28, 0, 0, NAV_NONE,   ACT_CONFIRM_PICK, NULL },                             /* Proceed  */
#endif
};
static const touch_zone_t DEST_ZONES[] = {
  {   8,  95, 100, 75, 0, 0, NAV_NONE,   ACT_CONFIRM_DROP_DEST, NULL },
  { 113,  95,  94, 75, 0, 0, NAV_NONE,   ACT_CONFIRM_DROP_NOW,  NULL },
  { 219, 110,  92, 33, 0, 0, SCR_CUSTOM, ACT_NONE,       NULL },
  {  14, 201,  78, 28, 0, 0, SCR_VOICE,  ACT_NONE,       NULL },                               /* Back     */
};
static const touch_zone_t CUSTOM_ZONES[] = {
  {  14, 201,  78, 28, 0, 0, SCR_DEST,   ACT_NONE,       NULL },                               /* Back     */
  { 224, 201,  78, 28, 0, 0, NAV_NONE,   ACT_CONFIRM_CUSTOM, NULL },                           /* Proceed  */
};
static const touch_zone_t STATUS_ZONES[] = {
  {   0,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_VOICE,        NAV_NONE, ACT_NONE, NULL },
  {  80,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_CARTESIAN,    NAV_NONE, ACT_NONE, NULL },
  { 160,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_JOINT_GROUP,  NAV_NONE, ACT_NONE, NULL },
  { 240,         0, 80, 24, EVT_TOUCH_MODE_TAB, MODE_SINGLE_JOINT, NAV_NONE, ACT_NONE, NULL },
  {   0,  BUTTON_Y, 100, 36, EVT_TOUCH_STOP,    0,                 NAV_NONE, ACT_NONE, "STOP" },
  { 110,  BUTTON_Y, 100, 36, EVT_TOUCH_UNDO,    0,                 NAV_NONE, ACT_NONE, "UNDO" },
  { 220,  BUTTON_Y, 100, 36, EVT_TOUCH_CONFIRM, 0,                 NAV_NONE, ACT_NONE, "OK" },
};

void lcd_show_message(const char *msg, uint32_t duration_ms)
{
  strncpy(s_msg, msg, LCD_COLS - 1);
  s_msg[LCD_COLS - 1] = '\0';
  s_msg_until = HAL_GetTick() + duration_ms;
}

/* ---------------------------------------------------------------- page components */
/* line under the X/Y/Z rows: "XYZ ESTIMATED, mm", or the latest message in yellow */
#define VALUE_LABEL_W     132
#define VALUE_LABEL_CHARS 21
static void draw_value_label(void)
{
  bool msg = s_msg[0] != '\0' && (int32_t)(s_msg_until - HAL_GetTick()) > 0;
  char want[LCD_COLS];
  if (msg) {
    snprintf(want, sizeof want, "%.*s", VALUE_LABEL_CHARS, s_msg);
  } else {
    bool cart_ready = false;
    bool cart_pitch = false;
    float pitch = 0.0f;
#if APP_USE_MODES
    if (s_screen == SCR_CART && mode_cartesian_ready()) {
      cart_ready = true;
      cart_pitch = mode_cartesian_encoder_pitch();
      pitch = mode_cartesian_pitch_deg();
    }
#endif
    if (cart_ready) {
      if (cart_pitch) snprintf(want, sizeof want, "PITCH %d deg", (int)pitch);
      else            snprintf(want, sizeof want, "Z 5mm  PITCH %d", (int)pitch);
    } else {
      snprintf(want, sizeof want, "XYZ ESTIMATED, mm");
    }
  }
  if (strcmp(s_label_shown, want) == 0) return;
  lcd_port_fill(s_label_x, s_label_y, VALUE_LABEL_W, 8, LCD_BG);
  lcd_port_text(s_label_x, s_label_y, want, msg ? LCD_YELLOW : LCD_GREY, LCD_BG, 1);
  memcpy(s_label_shown, want, sizeof s_label_shown);
}

static void draw_rows(int x, const int y[3])
{
  lcd_port_asset(LCD_A_ROW_X, x, y[0]);
  lcd_port_asset(LCD_A_ROW_Y, x, y[1]);
  lcd_port_asset(LCD_A_ROW_Z, x, y[2]);
  s_label_x = x;
  s_label_y = y[2] + LCD_ROW_H + 3;
  s_label_shown[0] = '\0';
  draw_value_label();
}

static void draw_buttons(void)
{
  lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
  lcd_port_asset(LCD_A_BTN_PROCEED, LCD_POS_BTN_PROCEED_X, LCD_POS_BTN_PROCEED_Y);
}

static void draw_back(void)
{
  lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
}

/* the group arm picture overlaps the frame, so draw it first */
static void draw_single_selection(void)
{
  lcd_port_asset(LCD_A_SINGLE_ARM(s_servo), LCD_POS_SINGLE_ARM_X, LCD_POS_SINGLE_ARM_Y);
  lcd_port_asset(LCD_A_SINGLE_LABEL(s_servo), LCD_POS_SINGLE_LABEL_X, LCD_POS_SINGLE_LABEL_Y);
}

static void draw_group_selection(void)
{
  lcd_port_asset(LCD_A_GROUP_ARM(s_group), LCD_POS_GROUP_ARM_X, LCD_POS_GROUP_ARM_Y);
  lcd_port_asset(LCD_A_SEL_WIDE, LCD_POS_GROUP_SEL_X, LCD_POS_GROUP_SEL_Y);
  lcd_port_asset(LCD_A_GROUP_LABEL(s_group), LCD_POS_GROUP_LABEL_X, LCD_POS_GROUP_LABEL_Y);
  lcd_port_asset(LCD_A_GROUP_SUB(s_group), LCD_POS_GROUP_SUB_X, LCD_POS_GROUP_SUB_Y);
}

static void begin_value_page(void)
{
  lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
  memset(s_val_shown, 0, sizeof s_val_shown);
  s_last_values = 0;
  s_speed_shown = -1;
}

/* FINE / FAST tag */
static void draw_speed_tag(int fast)
{
  lcd_port_fill(SPEED_TAG_X, SPEED_TAG_Y, SPEED_TAG_W, SPEED_TAG_H, LCD_BG);
  lcd_port_text(SPEED_TAG_X + 15, SPEED_TAG_Y + 6, fast ? "FAST" : "FINE", fast ? LCD_YELLOW : LCD_WHITE, LCD_BG, 2);
  s_speed_shown = fast;
}

/* cartesian page: Z / PITCH button */
static void draw_cart_enc_button(void)
{
  int pitch = 0;
#if APP_USE_MODES
  pitch = mode_cartesian_encoder_pitch() ? 1 : 0;
#endif
  if (pitch == s_cart_enc_shown) return;
  lcd_port_fill(CART_ENC_X, CART_ENC_Y, CART_ENC_W, CART_ENC_H, LCD_GREY);
  lcd_port_text(CART_ENC_X + 6, CART_ENC_Y + 6, pitch ? "PITCH" : "Z",
                pitch ? LCD_YELLOW : LCD_WHITE, LCD_GREY, 2);
  s_cart_enc_shown = pitch;
}

static void sync_selection(void)
{
#if APP_USE_MODES
  if (s_screen == SCR_SINGLE && mode_single_joint_selected() != s_servo) {
    s_servo = mode_single_joint_selected();
    draw_single_selection();
  } else if (s_screen == SCR_GROUP && mode_joint_group_selected() != s_group) {
    s_group = mode_joint_group_selected();
    draw_group_selection();
  }
#endif
}

/* ---------------------------------------------------------------- voice pages */
static void draw_card(int i)
{
  static const int CX[LCD_BOX_COUNT] = { LCD_POS_CARD_0_X, LCD_POS_CARD_1_X, LCD_POS_CARD_2_X, LCD_POS_CARD_3_X };
  static const int CY[LCD_BOX_COUNT] = { LCD_POS_CARD_0_Y, LCD_POS_CARD_1_Y, LCD_POS_CARD_2_Y, LCD_POS_CARD_3_Y };
  lcd_port_asset(LCD_A_CARD(i), CX[i], CY[i]);
  if (s_detected[i]) lcd_port_asset(LCD_A_CHECK, CX[i] + LCD_CHECK_DX, CY[i] + LCD_CHECK_DY);
  s_det_drawn[i] = s_detected[i] ? 1 : 0;
}

static void draw_pick_text(void)
{
  lcd_port_asset(LCD_A_PICK_TXT(s_pick >= 0 ? s_pick : 4), LCD_POS_PICK_TXT_X, LCD_POS_PICK_TXT_Y);
  s_pick_drawn = s_pick;
}

/* voice page info: task state (+ position to check) and the latest message */
#if !VOICE_TOUCH_FLOW
#define VINFO_X      100
#define VINFO_W      216
#define VINFO_CHARS  36
static void draw_voice_info(void)
{
  char want[2][40] = { "", "" };
#if APP_USE_MODES
  if (modes_current() == MODE_VOICE) {
    char d[2][LCD_COLS];
    memset(d, 0, sizeof d);
    modes_describe(d, 2);
    snprintf(want[0], sizeof want[0], "%.*s", VINFO_CHARS, d[0]);
    if (d[1][0] != '\0' && (int)strlen(want[0]) + 2 < VINFO_CHARS) {
      size_t n = strlen(want[0]);
      snprintf(&want[0][n], sizeof want[0] - n, "  %.*s", VINFO_CHARS - (int)n - 2, d[1]);
    }
  }
#endif
  if (s_msg[0] != '\0' && (int32_t)(s_msg_until - HAL_GetTick()) > 0) {
    snprintf(want[1], sizeof want[1], "%.*s", VINFO_CHARS, s_msg);
  }
  for (int i = 0; i < 2; i++) {
    if (strcmp(s_vinfo_shown[i], want[i]) == 0) continue;
    int y = LCD_POS_BTN_BACK_Y + 2 + i * 13;
    lcd_port_fill(VINFO_X, y, VINFO_W, 10, LCD_BG);
    if (want[i][0] != '\0') lcd_port_text(VINFO_X, y, want[i], i ? LCD_YELLOW : LCD_WHITE, LCD_BG, 1);
    memcpy(s_vinfo_shown[i], want[i], sizeof s_vinfo_shown[i]);
  }
}
#endif

/* jog queue pop-up over the camera panel. not an overlay, so the slider still works under it */
#define QPOP_X      LCD_POS_PANEL_CAM_X
#define QPOP_Y      LCD_POS_PANEL_CAM_Y
#define QPOP_W      289
#define QPOP_H      103
#define QPOP_CHARS  40                          /* lcd_port_text() max */
#define QPOP_LINES  5
static void draw_queue_popup(void)
{
  if (s_queue_txt[0] == '\0') {
    if (!s_queue_on_glass) return;
    s_queue_on_glass = false;
    s_queue_drawn[0] = '\0';
    lcd_port_asset(LCD_A_PANEL_CAM, LCD_POS_PANEL_CAM_X, LCD_POS_PANEL_CAM_Y);
    for (int i = 0; i < LCD_BOX_COUNT; i++) draw_card(i);
    return;
  }
  if (s_queue_on_glass && strcmp(s_queue_drawn, s_queue_txt) == 0) return;
  lcd_port_fill(QPOP_X, QPOP_Y, QPOP_W, QPOP_H, LCD_GREY);
  lcd_port_fill(QPOP_X + 1, QPOP_Y + 1, QPOP_W - 2, QPOP_H - 2, LCD_ROW_BG);
  lcd_port_text(QPOP_X + 8, QPOP_Y + 8, "JOG QUEUE   say YES / UNDO / NO", LCD_YELLOW, LCD_ROW_BG, 1);
  /* wrap after ", " so an entry never splits */
  const char *p = s_queue_txt;
  for (int line = 0; line < QPOP_LINES && *p != '\0'; line++) {
    int len = (int)strlen(p);
    if (len > QPOP_CHARS) {
      len = QPOP_CHARS;
      while (len > 0 && p[len] != ' ') len--;
      if (len == 0) len = QPOP_CHARS;
    }
    char buf[QPOP_CHARS + 1];
    memcpy(buf, p, (size_t)len);
    buf[len] = '\0';
    lcd_port_text(QPOP_X + 8, QPOP_Y + 26 + line * 13, buf, LCD_WHITE, LCD_ROW_BG, 1);
    p += len;
    while (*p == ' ') p++;
  }
  memcpy(s_queue_drawn, s_queue_txt, sizeof s_queue_drawn);
  s_queue_on_glass = true;
}

static void draw_slider(void)
{
  int cx = SLIDER_X0 + (s_dist * 29) / 4;
  lcd_port_asset(LCD_A_SLIDER, LCD_POS_SLIDER_X, LCD_POS_SLIDER_Y);
  lcd_port_fill(SLIDER_X0, SLIDER_TRACK_Y, cx - SLIDER_X0, 3, LCD_WHITE);
  lcd_port_asset(LCD_A_KNOB, cx - 6, SLIDER_TRACK_Y - 5);
  lcd_port_asset(LCD_A_DIST(s_dist), LCD_POS_DIST_VAL_X, LCD_POS_DIST_VAL_Y);
}

/* jog step slider in the header, 0.00..2.00 cm in 0.05 cm steps */
#define JOG_SL_X0     176
#define JOG_SL_W      128
#define JOG_SL_Y      27
#define JOG_SL_TOUCH_X 160
#define JOG_SL_MAX    200
static void draw_jog_slider(void)
{
  char t[48];
  int cx = JOG_SL_X0 + (s_jog01 * JOG_SL_W) / JOG_SL_MAX;
  lcd_port_fill(JOG_SL_TOUCH_X, 0, LCD_WIDTH_PX - JOG_SL_TOUCH_X, 38, LCD_BG);
  snprintf(t, sizeof t, "JOG STEP %d.%02d cm", s_jog01 / 100, s_jog01 % 100);
  lcd_port_text(JOG_SL_X0, 8, t, LCD_WHITE, LCD_BG, 1);
  lcd_port_fill(JOG_SL_X0, JOG_SL_Y, JOG_SL_W, 3, LCD_GREY);
  if (cx > JOG_SL_X0) lcd_port_fill(JOG_SL_X0, JOG_SL_Y, cx - JOG_SL_X0, 3, LCD_WHITE);
  lcd_port_asset(LCD_A_KNOB, cx - 6, JOG_SL_Y - 5);
  s_vspeed_shown = -1;                          /* the fill also cleared the speed tag */
}

/* voice page FINE / FAST tag */
static void draw_voice_speed(void)
{
  int fast = (motion_get_speed() == SPEED_FAST) ? 1 : 0;
  if (fast == s_vspeed_shown) return;
  lcd_port_fill(LCD_WIDTH_PX - 30, 6, 30, 10, LCD_BG);
  lcd_port_text(LCD_WIDTH_PX - 28, 8, fast ? "FAST" : "FINE", fast ? LCD_YELLOW : LCD_WHITE, LCD_BG, 1);
  s_vspeed_shown = fast;
}

static void jog_slider_set_from_x(int x)
{
  int v = ((x - JOG_SL_X0) * (JOG_SL_MAX / 5) + JOG_SL_W / 2) / JOG_SL_W * 5;
  if (x < JOG_SL_X0) v = 0;
  if (v < 0) v = 0;
  if (v > JOG_SL_MAX) v = JOG_SL_MAX;
  if (v != s_jog01) {
    s_jog01 = v;
    draw_jog_slider();
    (void)app_event_post(EVT_TOUCH_STEP_CM, 0, v, 0, 0);
  }
}

static void slider_set_from_x(int x)
{
  int v = ((x - SLIDER_X0) * 4 + 14) / 29;
  if (x < SLIDER_X0) v = DIST_MIN;
  if (v < DIST_MIN) v = DIST_MIN;
  if (v > DIST_MAX) v = DIST_MAX;
  if (v != s_dist) {
    s_dist = v;
    draw_slider();
  }
}

static void show_screen(screen_t s);

static void show_screen(screen_t s)
{
  static const int SINGLE_Y[3] = LCD_ROWS_SINGLE_Y;
  static const int GROUP_Y[3]  = LCD_ROWS_GROUP_Y;
  static const int CART_Y[3]   = LCD_ROWS_CART_Y;
  s_screen = s;
  switch (s) {
    case SCR_HOME:   lcd_port_asset(LCD_A_HOME, 0, 0); break;
    case SCR_SELECT: lcd_port_asset(LCD_A_SELECT, 0, 0); break;
    case SCR_SINGLE:
#if APP_USE_MODES
      s_servo = mode_single_joint_selected();
#endif
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_SINGLE, 0, 0);
      lcd_port_asset(LCD_A_SEL_NARROW, LCD_POS_SINGLE_SEL_X, LCD_POS_SINGLE_SEL_Y);
      draw_single_selection();
      draw_rows(LCD_ROWS_SINGLE_X, SINGLE_Y);
      draw_back();
      break;
    case SCR_GROUP:
#if APP_USE_MODES
      s_group = mode_joint_group_selected();
#endif
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_GROUP, 0, 0);
      draw_group_selection();
      draw_rows(LCD_ROWS_GROUP_X, GROUP_Y);
      draw_back();
      break;
    case SCR_CART:
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_CART, 0, 0);
      lcd_port_asset(LCD_A_CART_ARM, LCD_POS_CART_ARM_X, LCD_POS_CART_ARM_Y);
      draw_rows(LCD_ROWS_CART_X, CART_Y);
      draw_back();
      s_cart_enc_shown = -1;
      draw_cart_enc_button();
      break;
    case SCR_VOICE:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_PANEL_CAM, LCD_POS_PANEL_CAM_X, LCD_POS_PANEL_CAM_Y);
      for (int i = 0; i < LCD_BOX_COUNT; i++) draw_card(i);
      lcd_port_asset(LCD_A_PANEL_PICK, LCD_POS_PANEL_PICK_X, LCD_POS_PANEL_PICK_Y);
      draw_pick_text();
      draw_jog_slider();
      draw_voice_speed();
#if VOICE_TOUCH_FLOW
      draw_buttons();
#else
      lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
      memset(s_vinfo_shown, 0, sizeof s_vinfo_shown);
      s_vinfo_shown[0][0] = s_vinfo_shown[1][0] = '\x01';
      draw_voice_info();
#endif
      s_queue_on_glass = false;                 /* page redrawn under it: draw it again */
      draw_queue_popup();
      break;
    case SCR_DEST:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_DEST_HEAD(s_pick >= 0 ? s_pick : LCD_BOX_BLUE), LCD_POS_DEST_HEAD_X, LCD_POS_DEST_HEAD_Y);
      lcd_port_asset(LCD_A_DEST_BODY, LCD_POS_DEST_BODY_X, LCD_POS_DEST_BODY_Y);
      lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
      break;
    case SCR_CUSTOM:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_PANEL_A, LCD_POS_PANEL_A_X, LCD_POS_PANEL_A_Y);
      draw_slider();
      lcd_port_asset(LCD_A_PANEL_SAY, LCD_POS_PANEL_SAY_X, LCD_POS_PANEL_SAY_Y);
      lcd_port_asset(LCD_A_PANEL_QUEUE, LCD_POS_PANEL_QUEUE_X, LCD_POS_PANEL_QUEUE_Y);   /* TODO: placeholder */
      draw_buttons();
      break;
    default:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BLACK);
      for (uint32_t i = 0; i < ARRAY_LEN(STATUS_ZONES); i++) {
        if (STATUS_ZONES[i].label != NULL) {
          lcd_port_fill(STATUS_ZONES[i].x, STATUS_ZONES[i].y, STATUS_ZONES[i].w, STATUS_ZONES[i].h, LCD_GREY);
          lcd_port_text(STATUS_ZONES[i].x + 8, STATUS_ZONES[i].y + 10, STATUS_ZONES[i].label, LCD_WHITE, LCD_GREY, 2);
        }
      }
      memset(s_shown, 0, sizeof(s_shown));
      break;
  }
}

void lcd_init(void)
{
  lcd_port_init();
  show_screen(SCR_HOME);
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

  snprintf(s_want[1], LCD_COLS, "%s  %s", safety_state_name(safety_state()),
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

/* estimated X/Y/Z from FK of the commanded angles, "---" if FK fails */
static void page_values(const int row_y[3], int x_right)
{
  char want[3][8] = { "---", "---", "---" };
#if APP_USE_KINEMATICS
  joint_vec_t cur;
  cart_pose_t p;
  motion_get_current(&cur);
  if (kin_fk(&cur, &p) == KIN_OK) {
    snprintf(want[0], sizeof want[0], "%d", (int)p.x_mm);
    snprintf(want[1], sizeof want[1], "%d", (int)p.y_mm);
    snprintf(want[2], sizeof want[2], "%d", (int)p.z_mm);
  }
#endif
  for (int i = 0; i < 3; i++) {
    if (strcmp(s_val_shown[i], want[i]) == 0) continue;
    lcd_port_digits(x_right, row_y[i], SINGLE_VALUE_W, want[i], LCD_WHITE, LCD_ROW_BG);
    memcpy(s_val_shown[i], want[i], sizeof s_val_shown[i]);
  }
}

static void page_values_for(screen_t s)
{
  static const int SINGLE_Y[3] = LCD_ROWS_SINGLE_Y;
  static const int GROUP_Y[3]  = LCD_ROWS_GROUP_Y;
  static const int CART_Y[3]   = LCD_ROWS_CART_Y;
  int y[3];
  const int *src = (s == SCR_SINGLE) ? SINGLE_Y : (s == SCR_GROUP ? GROUP_Y : CART_Y);
  for (int i = 0; i < 3; i++) y[i] = src[i] + (LCD_ROW_H - LCD_DIGIT_H) / 2;
  page_values(y, (s == SCR_CART) ? 104 : 100);
}

/* ---------------------------------------------------------------- pop-ups */
static int conf_sub_index(confirm_t c)
{
  switch (c) {
    case CF_PICK:      return 3 + (s_pick >= 0 ? s_pick : LCD_BOX_BLUE);
    case CF_DROP_DEST: return 7;
    case CF_DROP_NOW:  return 8;
    default:           return 9;
  }
}

static void draw_overlay(void)
{
  int box = (s_pick >= 0) ? s_pick : LCD_BOX_BLUE;
  if (s_ov == OV_CONFIRM) {
    lcd_port_asset(LCD_A_OV_CONF_FRAME, LCD_POS_OV_CONF_X, LCD_POS_OV_CONF_Y);
    lcd_port_asset(LCD_A_OV_CONF_SUB(conf_sub_index(s_conf)), LCD_POS_OV_CONF_SUB_X, LCD_POS_OV_CONF_SUB_Y);
  } else if (s_ov == OV_STATUS) {
    switch (s_status) {
      case LCD_STATUS_PROCESSING:
        lcd_port_asset(LCD_A_OV_PROC_FRAME, LCD_POS_OV_STATUS_X, LCD_POS_OV_STATUS_Y);
        lcd_port_asset(LCD_A_OV_PROC_LINE(box), LCD_POS_OV_PROC_LINE_X, LCD_POS_OV_PROC_LINE_Y);
        break;
      case LCD_STATUS_STOPPED_HOLDING:
        lcd_port_asset(LCD_A_OV_STOPH_FRAME, LCD_POS_OV_STATUS_X, LCD_POS_OV_STATUS_Y);
        lcd_port_asset(LCD_A_OV_STOPH_LINE(box), LCD_POS_OV_STOPH_LINE_X, LCD_POS_OV_STOPH_LINE_Y);
        break;
      case LCD_STATUS_STOPPED_EMPTY:
        lcd_port_asset(LCD_A_OV_STOPE, LCD_POS_OV_STATUS_X, LCD_POS_OV_STATUS_Y);
        break;
      default:
        lcd_port_asset(LCD_A_OV_COMPLETE, LCD_POS_OV_STATUS_X, LCD_POS_OV_STATUS_Y);
        break;
    }
  }
}

static void open_confirm(confirm_t c)
{
  s_conf = c;
  s_ov = OV_CONFIRM;
  draw_overlay();
}

static void open_status(lcd_status_t st)
{
  s_status = st;
  s_ov = OV_STATUS;
  draw_overlay();
}

static void close_overlay(void)
{
  s_ov = OV_NONE;
  show_screen(s_screen);
}

/* confirm pop-up No / Yes (old touch flow, the actions are not wired) */
static void confirm_answer(bool yes)
{
  switch (s_conf) {
    case CF_PICK:
      s_ov = OV_NONE;
      if (yes) show_screen(SCR_DEST);
      else     show_screen(s_screen);
      break;
    case CF_DROP_DEST:
    case CF_DROP_NOW:
    case CF_CUSTOM:
      if (yes) open_status(LCD_STATUS_PROCESSING);
      else     close_overlay();
      break;
    default:
      close_overlay();
      break;
  }
}

static void status_tapped(void)
{
#if !DEBUG_STATUS_TAP
  close_overlay();                   /* a tap hides the pop-up, the task goes on */
#else
  switch (s_status) {
    case LCD_STATUS_PROCESSING:       open_status(LCD_STATUS_STOPPED_HOLDING); break;
    case LCD_STATUS_STOPPED_HOLDING:  open_status(LCD_STATUS_STOPPED_EMPTY);   break;
    case LCD_STATUS_STOPPED_EMPTY:    open_status(LCD_STATUS_COMPLETE);        break;
    default:                          s_ov = OV_NONE; show_screen(SCR_HOME);   break;
  }
#endif
}

static void overlay_touch(int x, int y)
{
  if (s_ov == OV_CONFIRM) {
    int x0 = LCD_POS_OV_CONF_X, y0 = LCD_POS_OV_CONF_Y;
    if (y >= y0 + 103 && y < y0 + 137) {
      if (x >= x0 + 16 && x < x0 + 122)  confirm_answer(false);
      else if (x >= x0 + 136 && x < x0 + 245) confirm_answer(true);
    }
  } else if (s_ov == OV_STATUS) {
    if (x >= LCD_POS_OV_STATUS_X && x < LCD_POS_OV_STATUS_X + LCD_OV_ST_W &&
        y >= LCD_POS_OV_STATUS_Y && y < LCD_POS_OV_STATUS_Y + LCD_OV_ST_H) status_tapped();
  }
}

/* ---------------------------------------------------------------- voice hooks (lcd.h) */
void lcd_voice_set_detected(lcd_box_t box, bool detected)
{
  if ((int)box >= 0 && (int)box < LCD_BOX_COUNT) s_detected[box] = detected;
}

void lcd_voice_set_pick(int box)
{
  s_pick = (box >= 0 && box < LCD_BOX_COUNT) ? box : -1;
}

int lcd_voice_pick(void)
{
  return s_pick;
}

void lcd_voice_set_queue(const char *text)
{
  snprintf(s_queue_txt, sizeof s_queue_txt, "%s", text != NULL ? text : "");
}

const char *lcd_voice_queue(void)
{
  return s_queue_txt;
}

void lcd_show_status(lcd_status_t status)
{
  if (status >= LCD_STATUS_PROCESSING && status <= LCD_STATUS_COMPLETE) open_status(status);
}

void lcd_hide_status(void)
{
  if (s_ov != OV_NONE) close_overlay();
}

int lcd_status_shown(void)
{
  return (s_ov == OV_STATUS) ? (int)s_status : 0;
}

bool lcd_voice_detected(lcd_box_t box)
{
  return ((int)box >= 0 && (int)box < LCD_BOX_COUNT) ? s_detected[box] : false;
}

static void run_action(uint8_t act, uint16_t arg)
{
  switch (act) {
    case ACT_SERVO_PREV:
    case ACT_SERVO_NEXT:
#if APP_USE_MODES
      (void)app_event_post(EVT_TOUCH_SELECT, 0, (act == ACT_SERVO_NEXT) ? 1 : -1, 0, 0);
#else
      s_servo = (act == ACT_SERVO_NEXT) ? (s_servo + 1) % SERVO_COUNT : (s_servo + SERVO_COUNT - 1) % SERVO_COUNT;
      draw_single_selection();
#endif
      break;
    case ACT_GROUP_PREV:
    case ACT_GROUP_NEXT:
#if APP_USE_MODES
      (void)app_event_post(EVT_TOUCH_SELECT, 0, (act == ACT_GROUP_NEXT) ? 1 : -1, 0, 0);
#else
      s_group = (act == ACT_GROUP_NEXT) ? (s_group + 1) % GROUP_COUNT : (s_group + GROUP_COUNT - 1) % GROUP_COUNT;
      draw_group_selection();
#endif
      break;
    case ACT_CONFIRM_PICK:      if (s_pick >= 0) open_confirm(CF_PICK); break;
    case ACT_CONFIRM_DROP_DEST: open_confirm(CF_DROP_DEST); break;
    case ACT_CONFIRM_DROP_NOW:  open_confirm(CF_DROP_NOW);  break;
    case ACT_CONFIRM_CUSTOM:    open_confirm(CF_CUSTOM);    break;
#if DEBUG_VOICE_TOUCH
    case ACT_BOX_TOGGLE:
      if (arg < LCD_BOX_COUNT) s_detected[arg] = !s_detected[arg];
      break;
    case ACT_PICK_CYCLE:
      s_pick = (s_pick + 1) % LCD_BOX_COUNT;
      break;
#endif
    default:
      (void)arg;
      break;
  }
}

static void poll_touch(void)
{
  const touch_zone_t *zones;
  uint32_t n;
  switch (s_screen) {
    case SCR_HOME:   zones = HOME_ZONES;   n = ARRAY_LEN(HOME_ZONES);   break;
    case SCR_SELECT: zones = SELECT_ZONES; n = ARRAY_LEN(SELECT_ZONES); break;
    case SCR_SINGLE: zones = SINGLE_ZONES; n = ARRAY_LEN(SINGLE_ZONES); break;
    case SCR_GROUP:  zones = GROUP_ZONES;  n = ARRAY_LEN(GROUP_ZONES);  break;
    case SCR_CART:   zones = CART_ZONES;   n = ARRAY_LEN(CART_ZONES);   break;
    case SCR_VOICE:  zones = VOICE_ZONES;  n = ARRAY_LEN(VOICE_ZONES);  break;
    case SCR_DEST:   zones = DEST_ZONES;   n = ARRAY_LEN(DEST_ZONES);   break;
    case SCR_CUSTOM: zones = CUSTOM_ZONES; n = ARRAY_LEN(CUSTOM_ZONES); break;
    default:         zones = STATUS_ZONES; n = ARRAY_LEN(STATUS_ZONES); break;
  }
  int x = 0, y = 0;
  bool down = lcd_port_touch(&x, &y);
  if (s_ov != OV_NONE) {                        /* pop-up open: only it gets touch */
    if (down && !s_touch_down) overlay_touch(x, y);
    s_touch_down = down;
    return;
  }
  if (s_screen == SCR_CUSTOM && down && x >= LCD_POS_SLIDER_X && x < LCD_POS_SLIDER_X + 160 &&
      y >= LCD_POS_SLIDER_Y && y < LCD_POS_SLIDER_Y + 30) {
    slider_set_from_x(x);
  }
  if (s_screen == SCR_VOICE && down && x >= JOG_SL_TOUCH_X && y < 40) {
    jog_slider_set_from_x(x);
  }
  if (down && !s_touch_down) {                  /* touch-down only, so a held finger does not hit the next page */
    for (uint32_t i = 0; i < n; i++) {
      const touch_zone_t *z = &zones[i];
      if (x >= z->x && x < z->x + z->w && y >= z->y && y < z->y + z->h) {
        if (z->evt != 0u) (void)app_event_post((evt_type_t)z->evt, z->arg, 0, 0, 0);
        if (z->act != ACT_NONE) run_action(z->act, z->arg);
        if (z->nav != NAV_NONE) show_screen((screen_t)z->nav);
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
  if (s_ov != OV_NONE) return;
  if (s_screen == SCR_VOICE) {
    draw_queue_popup();
    for (int i = 0; i < LCD_BOX_COUNT && !s_queue_on_glass; i++) {
      if (s_det_drawn[i] != (s_detected[i] ? 1 : 0)) draw_card(i);
    }
    if (s_pick_drawn != s_pick) draw_pick_text();
    draw_voice_speed();
#if !VOICE_TOUCH_FLOW
    draw_voice_info();
#endif
    return;
  }
  if (s_screen == SCR_SINGLE || s_screen == SCR_GROUP || s_screen == SCR_CART) {
    sync_selection();
    int fast = (motion_get_speed() == SPEED_FAST) ? 1 : 0;
    if (fast != s_speed_shown) draw_speed_tag(fast);
    if (s_screen == SCR_CART) draw_cart_enc_button();
    draw_value_label();
    if (s_last_values == 0u || ms_since(now, s_last_values) >= SINGLE_VALUES_MS) {
      s_last_values = now | 1u;                 /* never 0, 0 means draw now */
      page_values_for(s_screen);
    }
    return;
  }
  if (s_screen != SCR_STATUS) return;
  if (ms_since(now, s_last_refresh) >= REFRESH_MS) {
    s_last_refresh = now;
    build_screen();
  }
  draw_changed_lines();
}
