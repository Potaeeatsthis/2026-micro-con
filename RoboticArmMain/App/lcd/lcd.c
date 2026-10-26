/* lcd.c - pages + text-line screen model
 *
 * pages:  HOME (picture, Start) -> SELECT MODE (picture, four cards) -> STATUS (text lines below).
 *         Single card -> SINGLE page (servo 1..6 selector, arm picture follows, X/Y/Z numbers, Back).
 *         Group card  -> GROUP page (group 1..3 = S1+S2, S3+S4, S5+S6, same layout).
 *         Cartesian card -> CARTESIAN page (arm with axis arrows, X/Y/Z numbers, Back).
 *         a mode card switches the mode at once (no confirm: choosing a mode moves nothing).
 *         Voice card -> VOICE page (camera check marks + "Pick Color") -> Proceed -> confirm pop-up
 *                       -> DESTINATION page (drop in destination / drop now / custom)
 *                       -> confirm pop-up -> PROCESSING pop-up ... see the status pop-ups below
 *                       custom -> CUSTOM page (distance slider, queue) -> Proceed -> confirm pop-up.
 *         confirm pop-ups only before the arm moves on its own (voice pick, drop, custom queue).
 *         Back returns to the previous page (SELECT, or the voice page before).
 *
 * pop-ups (drawn on top of the page, only they get touch while open):
 *   confirm   "CONFIRM ACTION ?"  No / Yes: both remove it
 *   status    processing / stopped (holding) / stopped (empty) / complete, for every box colour.
 *             shown with lcd_show_status() by the voice logic. DEBUG_STATUS_TAP: tapping the pop-up steps
 *             through them (processing -> stopped holding -> stopped empty -> complete -> HOME).
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

typedef enum { SCR_HOME = 0, SCR_SELECT, SCR_STATUS, SCR_SINGLE, SCR_GROUP, SCR_CART, SCR_VOICE, SCR_DEST, SCR_CUSTOM } screen_t;
typedef enum {
  ACT_NONE = 0, ACT_SERVO_PREV, ACT_SERVO_NEXT, ACT_GROUP_PREV, ACT_GROUP_NEXT,
  ACT_CONFIRM_PICK,
  ACT_CONFIRM_DROP_DEST, ACT_CONFIRM_DROP_NOW, ACT_CONFIRM_CUSTOM,
  ACT_BOX_TOGGLE, ACT_PICK_CYCLE
} action_t;
typedef enum { CF_NONE = 0, CF_PICK, CF_DROP_DEST, CF_DROP_NOW, CF_CUSTOM } confirm_t;
typedef enum { OV_NONE = 0, OV_CONFIRM, OV_STATUS } overlay_t;

/* DEBUG helpers until the camera / voice logic drives the screens (set to 0 to switch them off) */
#define DEBUG_VOICE_TOUCH 1   /* voice page: tap a colour card = toggle its check mark; tap "Pick Color" = next colour */
#define DEBUG_STATUS_TAP  1   /* tap a status pop-up = next status; the last one goes back to HOME */

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
static overlay_t    s_ov;                     /* pop-up on top of the page */
static confirm_t    s_conf;
static lcd_status_t s_status;
static int      s_pick = LCD_BOX_BLUE;        /* colour asked for by voice, -1 = none (test colour: blue) */
static int      s_pick_drawn = -2;
static bool     s_detected[LCD_BOX_COUNT];    /* camera says: this colour is on the table */
static int8_t   s_det_drawn[LCD_BOX_COUNT];
static int      s_dist = 2;                   /* custom page: distance in cm, 1..20 */

#define SLIDER_X0        (LCD_POS_SLIDER_X + 6)    /* x of 0 cm on the track */
#define SLIDER_TRACK_Y   (LCD_POS_SLIDER_Y + 9)
#define DIST_MIN         1
#define DIST_MAX         20
/* ---------------------------------------------------------------- touch zones, one table per page
 * a zone can post an event (evt != 0), open another page (nav != NAV_NONE) and/or run an action (act).
 * EXTENSION POINT: new touch buttons = one row here (+ an event in app_events.h if it needs one) */
typedef struct { int x, y, w, h; uint16_t evt; uint16_t arg; uint8_t nav; uint8_t act; const char *label; } touch_zone_t;

/* rectangles measured on the pictures (home / select-mode) */
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
  {  14,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_PREV, NULL },                              /* < servo  */
  {  80,  57,  66,  28, 0, 0, NAV_NONE,   ACT_SERVO_NEXT, NULL },                              /* servo >  */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t GROUP_ZONES[] = {
  {  14,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_PREV, NULL },                              /* < group  */
  {  96,  48,  82,  28, 0, 0, NAV_NONE,   ACT_GROUP_NEXT, NULL },                              /* group >  */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t CART_ZONES[] = {
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t VOICE_ZONES[] = {
  {  23,  73, 131, 35, 0, 0, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* red card   (debug) */
  { 162,  73, 131, 35, 0, 1, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* green card (debug) */
  {  23, 112, 131, 35, 0, 2, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* black card (debug) */
  { 162, 112, 131, 35, 0, 3, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* blue card  (debug) */
  {  14, 157, 289, 40, 0, 0, NAV_NONE,   ACT_PICK_CYCLE, NULL },                               /* Pick Color (debug) */
  {  14, 201,  78, 28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                               /* Back     */
  { 224, 201,  78, 28, 0, 0, NAV_NONE,   ACT_CONFIRM_PICK, NULL },                             /* Proceed  */
};
static const touch_zone_t DEST_ZONES[] = {
  {   8,  95, 100, 75, 0, 0, NAV_NONE,   ACT_CONFIRM_DROP_DEST, NULL },                        /* drop in destination */
  { 113,  95,  94, 75, 0, 0, NAV_NONE,   ACT_CONFIRM_DROP_NOW,  NULL },                        /* drop now            */
  { 219, 110,  92, 33, 0, 0, SCR_CUSTOM, ACT_NONE,       NULL },                               /* custom              */
  {  14, 201,  78, 28, 0, 0, SCR_VOICE,  ACT_NONE,       NULL },                               /* Back     */
};
static const touch_zone_t CUSTOM_ZONES[] = {                                                  /* the slider is handled in poll_touch */
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

static void draw_back(void)          /* manual pages: the mode is already on, nothing to proceed to */
{
  lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);
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

/* ---------------------------------------------------------------- voice pages */
static void draw_card(int i)
{
  static const int CX[LCD_BOX_COUNT] = { LCD_POS_CARD_0_X, LCD_POS_CARD_1_X, LCD_POS_CARD_2_X, LCD_POS_CARD_3_X };
  static const int CY[LCD_BOX_COUNT] = { LCD_POS_CARD_0_Y, LCD_POS_CARD_1_Y, LCD_POS_CARD_2_Y, LCD_POS_CARD_3_Y };
  lcd_port_asset(LCD_A_CARD(i), CX[i], CY[i]);                  /* the card without a check mark ... */
  if (s_detected[i]) lcd_port_asset(LCD_A_CHECK, CX[i] + LCD_CHECK_DX, CY[i] + LCD_CHECK_DY);   /* ... then the camera's tick */
  s_det_drawn[i] = s_detected[i] ? 1 : 0;
}

static void draw_pick_text(void)
{
  lcd_port_asset(LCD_A_PICK_TXT(s_pick >= 0 ? s_pick : 4), LCD_POS_PICK_TXT_X, LCD_POS_PICK_TXT_Y);
  s_pick_drawn = s_pick;
}

static void draw_slider(void)
{
  int cx = SLIDER_X0 + (s_dist * 29) / 4;                       /* 7.25 px per cm */
  lcd_port_asset(LCD_A_SLIDER, LCD_POS_SLIDER_X, LCD_POS_SLIDER_Y);
  lcd_port_fill(SLIDER_X0, SLIDER_TRACK_Y, cx - SLIDER_X0, 3, LCD_WHITE);   /* the part already chosen */
  lcd_port_asset(LCD_A_KNOB, cx - 6, SLIDER_TRACK_Y - 5);
  lcd_port_asset(LCD_A_DIST(s_dist), LCD_POS_DIST_VAL_X, LCD_POS_DIST_VAL_Y);
}

static void slider_set_from_x(int x)
{
  int v = ((x - SLIDER_X0) * 4 + 14) / 29;                      /* nearest whole cm */
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
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_SINGLE, 0, 0);
      lcd_port_asset(LCD_A_SEL_NARROW, LCD_POS_SINGLE_SEL_X, LCD_POS_SINGLE_SEL_Y);
      draw_single_selection();                  /* arm picture + label (they do not overlap the frame) */
      draw_rows(LCD_ROWS_SINGLE_X, SINGLE_Y);
      draw_back();
      break;
    case SCR_GROUP:
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
      break;
    case SCR_VOICE:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_PANEL_CAM, LCD_POS_PANEL_CAM_X, LCD_POS_PANEL_CAM_Y);
      for (int i = 0; i < LCD_BOX_COUNT; i++) draw_card(i);
      lcd_port_asset(LCD_A_PANEL_PICK, LCD_POS_PANEL_PICK_X, LCD_POS_PANEL_PICK_Y);
      draw_pick_text();
      draw_buttons();
      break;
    case SCR_DEST:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_DEST_HEAD(s_pick >= 0 ? s_pick : LCD_BOX_BLUE), LCD_POS_DEST_HEAD_X, LCD_POS_DEST_HEAD_Y);
      lcd_port_asset(LCD_A_DEST_BODY, LCD_POS_DEST_BODY_X, LCD_POS_DEST_BODY_Y);
      lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);     /* no Proceed here */
      break;
    case SCR_CUSTOM:
      lcd_port_fill(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, LCD_BG);
      lcd_port_asset(LCD_A_HDR_VOICE, 0, 0);
      lcd_port_asset(LCD_A_PANEL_A, LCD_POS_PANEL_A_X, LCD_POS_PANEL_A_Y);
      draw_slider();
      lcd_port_asset(LCD_A_PANEL_SAY, LCD_POS_PANEL_SAY_X, LCD_POS_PANEL_SAY_Y);
      lcd_port_asset(LCD_A_PANEL_QUEUE, LCD_POS_PANEL_QUEUE_X, LCD_POS_PANEL_QUEUE_Y);   /* TODO(voice): placeholder queue */
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

/* ---------------------------------------------------------------- pop-ups */
static int conf_sub_index(confirm_t c)
{
  switch (c) {
    case CF_PICK:      return 3 + (s_pick >= 0 ? s_pick : LCD_BOX_BLUE);
    case CF_DROP_DEST: return 7;
    case CF_DROP_NOW:  return 8;
    default:           return 9;                                /* CF_CUSTOM */
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
      default:                                                  /* LCD_STATUS_COMPLETE */
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
  draw_overlay();                    /* same size or bigger than anything below, so it simply covers it */
}

static void close_overlay(void)
{
  s_ov = OV_NONE;
  show_screen(s_screen);             /* redraws the page the pop-up was covering */
}

/* No / Yes of the confirm pop-up. both remove it.
 * TODO(voice): the real actions start where it says so below; the screens only show the result. */
static void confirm_answer(bool yes)
{
  switch (s_conf) {
    case CF_PICK:
      s_ov = OV_NONE;
      if (yes) show_screen(SCR_DEST);   /* TODO(voice): start picking up the colour in s_pick */
      else     show_screen(s_screen);
      break;
    case CF_DROP_DEST:                 /* TODO(voice): drop at default / station / hand */
    case CF_DROP_NOW:                  /* TODO(voice): place / drop here */
    case CF_CUSTOM:                    /* TODO(voice): run the queue (s_dist cm steps) */
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
#if DEBUG_STATUS_TAP
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
      if (x >= x0 + 16 && x < x0 + 122)  confirm_answer(false);       /* No  */
      else if (x >= x0 + 136 && x < x0 + 245) confirm_answer(true);   /* Yes */
    }
  } else if (s_ov == OV_STATUS) {
    if (x >= LCD_POS_OV_STATUS_X && x < LCD_POS_OV_STATUS_X + LCD_OV_ST_W &&
        y >= LCD_POS_OV_STATUS_Y && y < LCD_POS_OV_STATUS_Y + LCD_OV_ST_H) status_tapped();
  }
}

/* ---------------------------------------------------------------- voice hooks (lcd.h) */
void lcd_voice_set_detected(lcd_box_t box, bool detected)
{
  if ((int)box >= 0 && (int)box < LCD_BOX_COUNT) s_detected[box] = detected;   /* drawn by lcd_step */
}

void lcd_voice_set_pick(int box)
{
  s_pick = (box >= 0 && box < LCD_BOX_COUNT) ? box : -1;                       /* drawn by lcd_step */
}

int lcd_voice_pick(void)
{
  return s_pick;
}

void lcd_show_status(lcd_status_t status)
{
  if (status >= LCD_STATUS_PROCESSING && status <= LCD_STATUS_COMPLETE) open_status(status);
}

void lcd_hide_status(void)
{
  if (s_ov != OV_NONE) close_overlay();
}

static void run_action(uint8_t act, uint16_t arg)
{
  switch (act) {
    case ACT_SERVO_PREV:
    case ACT_SERVO_NEXT:
      s_servo = (act == ACT_SERVO_NEXT) ? (s_servo + 1) % SERVO_COUNT : (s_servo + SERVO_COUNT - 1) % SERVO_COUNT;
      draw_single_selection();
      break;
    case ACT_GROUP_PREV:
    case ACT_GROUP_NEXT:
      s_group = (act == ACT_GROUP_NEXT) ? (s_group + 1) % GROUP_COUNT : (s_group + GROUP_COUNT - 1) % GROUP_COUNT;
      draw_group_selection();
      break;
    case ACT_CONFIRM_PICK:      if (s_pick >= 0) open_confirm(CF_PICK); break;     /* needs a colour first */
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
  if (s_ov != OV_NONE) {                        /* a pop-up is open: only it gets touch */
    if (down && !s_touch_down) overlay_touch(x, y);
    s_touch_down = down;
    return;
  }
  if (s_screen == SCR_CUSTOM && down && x >= LCD_POS_SLIDER_X && x < LCD_POS_SLIDER_X + 160 &&
      y >= LCD_POS_SLIDER_Y && y < LCD_POS_SLIDER_Y + 30) {
    slider_set_from_x(x);                       /* while the finger is down: tap or drag */
  }
  if (down && !s_touch_down) {                  /* act on touch-down only: a held finger cannot hit the next page */
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
  if (s_ov != OV_NONE) return;                  /* never draw under a pop-up */
  if (s_screen == SCR_VOICE) {                  /* redraw only what changed */
    for (int i = 0; i < LCD_BOX_COUNT; i++) {
      if (s_det_drawn[i] != (s_detected[i] ? 1 : 0)) draw_card(i);
    }
    if (s_pick_drawn != s_pick) draw_pick_text();
    return;
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
