/* lcd.c - pages + text-line screen model
 *
 * pages:  HOME (picture, Start) -> SELECT MODE (picture, four cards) -> STATUS (text lines below).
 *         Single card -> SINGLE page (servo 1..6 selector, arm picture follows, X/Y/Z numbers, Back / Proceed).
 *         Group card  -> GROUP page (group 1..3 = S1+S2, S3+S4, S5+S6, same layout).
 *         Cartesian card -> CARTESIAN page (arm with axis arrows, X/Y/Z numbers, Back / Proceed).
 *         Voice card posts EVT_TOUCH_MODE_TAB (modes.c switches mode safely) and opens STATUS;
 *         Proceed on the three pages does the same for its own mode.
 *         Back always returns to the page it came from (SELECT).
 *         touch zones belong to a page, so a button is only active while its page is shown.
 *
 * the pages are built from shared components (lcd_assets: header, Back / Proceed buttons, X/Y/Z rows,
 * selector frame, arm pictures), all RLE-packed in flash. to add a page: add its pictures in
 * tools/build_lcd_assets.py, then write its draw_xxx_page() below, a zone table and one case in
 * show_screen() / poll_touch().
 *
 * STATUS layout (size-2 text, 12 px per char, 24 px per line):
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

typedef enum { SCR_HOME = 0, SCR_SELECT, SCR_STATUS, SCR_SINGLE, SCR_GROUP, SCR_CART } screen_t;
typedef enum { ACT_NONE = 0, ACT_SERVO_PREV, ACT_SERVO_NEXT, ACT_GROUP_PREV, ACT_GROUP_NEXT } action_t;

#define LCD_BG           0x0842u   /* rgb565 of the page background (8,11,16) */
#define LCD_ROW_BG       0x0882u   /* rgb565 of the X/Y/Z row fill (13,17,23) */
#define SINGLE_VALUE_X   100       /* last digit of X/Y/Z ends here */
#define SINGLE_VALUE_W   50
#define SINGLE_VALUES_MS 250u
#define SERVO_COUNT      6
#define GROUP_COUNT      3
#define NAV_NONE 0xFFu

static screen_t s_screen;
static char     s_shown[N_LINES][LCD_COLS];   /* what is on the glass (status page) */
static char     s_want[N_LINES][LCD_COLS];    /* what should be */
static char     s_msg[LCD_COLS];
static uint32_t s_msg_until;
static uint32_t s_last_refresh, s_last_touch;
static bool     s_touch_down;
static int      s_servo;                      /* single page: 0..5 */
static int      s_group;                      /* group page: 0..2 */
static char     s_val_shown[3][8];
static uint32_t s_last_values;

/* ---------------------------------------------------------------- touch zones, one table per page
 * a zone can post an event (evt != 0), open another page (nav != NAV_NONE) and/or run an action (act).
 * EXTENSION POINT: new touch buttons = one row here (+ an event in app_events.h if it needs one) */
typedef struct { int x, y, w, h; uint16_t evt; uint16_t arg; uint8_t nav; uint8_t act; const char *label; } touch_zone_t;

/* rectangles measured on the pictures (home / select-mode) */
static const touch_zone_t HOME_ZONES[] = {
  {  19, 139, 172,  48, 0, 0, SCR_SELECT, ACT_NONE, NULL },                                    /* Start */
};
static const touch_zone_t SELECT_ZONES[] = {
  {  21,  16, 132,  93, 0,                  0,                 SCR_SINGLE, ACT_NONE, NULL },   /* Single    */
  { 170,  16, 132,  93, 0,                  0,                 SCR_GROUP,  ACT_NONE, NULL },   /* Group     */
  {  21, 129, 132,  93, 0,                  0,                 SCR_CART,   ACT_NONE, NULL },   /* Cartesian */
  { 170, 129, 132,  93, EVT_TOUCH_MODE_TAB, MODE_VOICE,        SCR_STATUS, ACT_NONE, NULL },   /* Voice     */
};
static const touch_zone_t SINGLE_ZONES[] = {
  {  14,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_PREV, NULL },                              /* < servo  */
  {  80,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_NEXT, NULL },                              /* servo >  */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
  { 224, 201,  78,  28, EVT_TOUCH_MODE_TAB, MODE_SINGLE_JOINT, SCR_STATUS, ACT_NONE, NULL },   /* Proceed  */
};
static const touch_zone_t GROUP_ZONES[] = {
  {  14,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_PREV, NULL },                              /* < group  */
  {  96,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_NEXT, NULL },                              /* group >  */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
  { 224, 201,  78,  28, EVT_TOUCH_MODE_TAB, MODE_JOINT_GROUP, SCR_STATUS, ACT_NONE, NULL },    /* Proceed  */
};
static const touch_zone_t CART_ZONES[] = {
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
  { 224, 201,  78,  28, EVT_TOUCH_MODE_TAB, MODE_CARTESIAN, SCR_STATUS, ACT_NONE, NULL },      /* Proceed  */
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
static void draw_rows(int x, const int y[3])
{
  lcd_port_asset(LCD_A_ROW_X, x, y[0]);
  lcd_port_asset(LCD_A_ROW_Y, x, y[1]);
  lcd_port_asset(LCD_A_ROW_Z, x, y[2]);
}

static void draw_buttons(void)
{
  lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
  lcd_port_asset(LCD_A_BTN_PROCEED, LCD_POS_BTN_PROCEED_X, LCD_POS_BTN_PROCEED_Y);
}

/* the parts of a page that change with the selector. the group arm picture also covers the right end of the
 * selector frame and the end of the subtitle, so there it goes FIRST and frame / label / subtitle on top */
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
  memset(s_val_shown, 0, sizeof s_val_shown);   /* the numbers are drawn by lcd_step */
  s_last_values = 0;
}

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
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_SINGLE, 0, 0);
      lcd_port_asset(LCD_A_SEL_NARROW, LCD_POS_SINGLE_SEL_X, LCD_POS_SINGLE_SEL_Y);
      draw_single_selection();                  /* arm picture + label (they do not overlap the frame) */
      draw_rows(LCD_ROWS_SINGLE_X, SINGLE_Y);
      draw_buttons();
      break;
    case SCR_GROUP:
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_GROUP, 0, 0);
      draw_group_selection();
      draw_rows(LCD_ROWS_GROUP_X, GROUP_Y);
      draw_buttons();
      break;
    case SCR_CART:
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_CART, 0, 0);
      lcd_port_asset(LCD_A_CART_ARM, LCD_POS_CART_ARM_X, LCD_POS_CART_ARM_Y);
      draw_rows(LCD_ROWS_CART_X, CART_Y);
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
      memset(s_shown, 0, sizeof(s_shown));      /* force a full text redraw */
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

/* X/Y/Z of the tool. no geometry yet (kin_fk returns KIN_NOT_READY) -> "---" instead of a made-up number.
 * row_y = top of each number field, x_right = where the last digit ends (measured on the page designs) */
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
  for (int i = 0; i < 3; i++) y[i] = src[i] + (LCD_ROW_H - LCD_DIGIT_H) / 2;   /* centre the digits in their row */
  page_values(y, (s == SCR_CART) ? 104 : 100);
}

static void run_action(uint8_t act)
{
  if (act == ACT_SERVO_PREV || act == ACT_SERVO_NEXT) {
    s_servo = (act == ACT_SERVO_NEXT) ? (s_servo + 1) % SERVO_COUNT : (s_servo + SERVO_COUNT - 1) % SERVO_COUNT;
    draw_single_selection();
  } else if (act == ACT_GROUP_PREV || act == ACT_GROUP_NEXT) {
    s_group = (act == ACT_GROUP_NEXT) ? (s_group + 1) % GROUP_COUNT : (s_group + GROUP_COUNT - 1) % GROUP_COUNT;
    draw_group_selection();
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
    default:         zones = STATUS_ZONES; n = ARRAY_LEN(STATUS_ZONES); break;
  }
  int x = 0, y = 0;
  bool down = lcd_port_touch(&x, &y);
  if (down && !s_touch_down) {                  /* act on touch-down only: a held finger cannot hit the next page */
    for (uint32_t i = 0; i < n; i++) {
      const touch_zone_t *z = &zones[i];
      if (x >= z->x && x < z->x + z->w && y >= z->y && y < z->y + z->h) {
        if (z->evt != 0u) (void)app_event_post((evt_type_t)z->evt, z->arg, 0, 0, 0);
        if (z->act != ACT_NONE) run_action(z->act);
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
  if (s_screen == SCR_SINGLE || s_screen == SCR_GROUP || s_screen == SCR_CART) {
    if (s_last_values == 0u || ms_since(now, s_last_values) >= SINGLE_VALUES_MS) {
      s_last_values = now | 1u;                 /* never 0, so 0 can mean "draw now" */
      page_values_for(s_screen);
    }
    return;
  }
  if (s_screen != SCR_STATUS) return;           /* picture pages need no refresh */
  if (ms_since(now, s_last_refresh) >= REFRESH_MS) {
    s_last_refresh = now;
    build_screen();
  }
  draw_changed_lines();
}
