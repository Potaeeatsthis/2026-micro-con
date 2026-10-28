/* lcd.c - pages + text-line screen model
 *
 * pages:  HOME (picture, Start) -> SELECT MODE (picture, four cards) -> STATUS (text lines below).
 *         Single card -> SINGLE page (servo 1..6 selector, arm picture follows, X/Y/Z numbers, Back).
 *         Group card  -> GROUP page (group 1..3 = S1+S2, S3+S4, S5+S6, same layout).
 *         Cartesian card -> CARTESIAN page (arm with axis arrows, X/Y/Z numbers, Z / PITCH encoder button, Back).
 *         a mode card switches the mode at once (no confirm: choosing a mode moves nothing).
 *         Voice card -> voice mode + VOICE page (camera check marks + "Pick Color"). since 28 Oct 2026
 *                       mode_voice drives it (lcd.h hooks): "Pick Color" = the colour said, a tick on each
 *                       colour the pi found, the status pop-ups below. voice WORDS drive the task.
 *                       the older touch flow (Proceed -> confirm -> DESTINATION / CUSTOM pages) is kept
 *                       behind VOICE_TOUCH_FLOW 0: it is not wired to mode_voice.
 *         Back returns to the previous page (SELECT, or the voice page before).
 *
 * pop-ups (drawn on top of the page, only they get touch while open):
 *   confirm   "CONFIRM ACTION ?"  No / Yes: both remove it
 *   status    processing / stopped (holding) / stopped (empty) / complete, for every box colour.
 *             shown with lcd_show_status() by mode_voice. a tap hides the pop-up (the task goes on);
 *             DEBUG_STATUS_TAP 1 instead steps through them (processing -> ... -> complete -> HOME).
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

/* DEBUG helpers from before mode_voice drove the screens. off since 28 Oct 2026: mode_voice now sets the
 * colour, the ticks and the pop-ups (lcd.h hooks), and voice words drive the task */
#define DEBUG_VOICE_TOUCH 0   /* voice page: tap a colour card = toggle its check mark; tap "Pick Color" = next colour */
#define DEBUG_STATUS_TAP  0   /* tap a status pop-up = next status; the last one goes back to HOME */
#define VOICE_TOUCH_FLOW  0   /* voice page Proceed -> confirm -> DEST / CUSTOM pages (touch flow, not wired to
                               * mode_voice yet): off, so a tap cannot show a task that is not happening */

#define LCD_BG           0x0842u   /* rgb565 of the page background (8,11,16) */
#define LCD_ROW_BG       0x0882u   /* rgb565 of the X/Y/Z row fill (13,17,23) */
#define SINGLE_VALUE_X   100       /* last digit of X/Y/Z ends here */
#define SINGLE_VALUE_W   50
#define SINGLE_VALUES_MS 250u
#define SPEED_TAG_X      LCD_POS_BTN_PROCEED_X   /* FINE / FAST tag where Proceed sits on other pages */
#define SPEED_TAG_Y      LCD_POS_BTN_PROCEED_Y
#define SPEED_TAG_W      78
#define SPEED_TAG_H      28
#define CART_ENC_X       100                    /* Cartesian page: Z / PITCH button between Back and speed */
#define CART_ENC_Y       LCD_POS_BTN_BACK_Y
#define CART_ENC_W       118
#define CART_ENC_H       28
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
static int      s_speed_shown = -1;           /* -1 = tag not drawn yet, else 0 FINE / 1 FAST */
static int      s_cart_enc_shown = -1;        /* Cartesian page Z / PITCH button: -1 = redraw */
static int      s_label_x, s_label_y;         /* the line under the X/Y/Z rows (draw_value_label) */
static char     s_label_shown[LCD_COLS];
#if !VOICE_TOUCH_FLOW
static char     s_vinfo_shown[2][40];         /* voice page: state line + message line (draw_voice_info) */
#endif
static uint32_t s_last_values;
static overlay_t    s_ov;                     /* pop-up on top of the page */
static confirm_t    s_conf;
static lcd_status_t s_status;
static int      s_pick = -1;                  /* colour asked for by voice, -1 = none */
static int      s_pick_drawn = -2;
static bool     s_detected[LCD_BOX_COUNT];    /* camera says: this colour is on the table */
static int8_t   s_det_drawn[LCD_BOX_COUNT];
static int      s_dist = 2;                   /* custom page: distance in cm, 1..20 */
static int      s_jog01 = 200;                /* voice page jog step slider, 0.1 mm: 0..200 = 0.00..2.00 cm */
static int      s_vspeed_shown = -1;          /* voice page FINE / FAST tag: -1 = redraw, 0 FINE, 1 FAST */
#define QUEUE_TXT_LEN   160                   /* 10 x ", backward 2.00" + "[]" fits */
static char     s_queue_txt[QUEUE_TXT_LEN];   /* voice page jog queue pop-up text, "" = hidden */
static char     s_queue_drawn[QUEUE_TXT_LEN]; /* what the pop-up on the glass shows */
static bool     s_queue_on_glass;             /* the pop-up covers the camera panel right now */

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
  { CART_ENC_X, CART_ENC_Y, CART_ENC_W, CART_ENC_H, EVT_TOUCH_SELECT, 0, NAV_NONE, ACT_NONE, NULL }, /* Z / PITCH */
  {  14, 201,  78,  28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                              /* Back     */
};
static const touch_zone_t VOICE_ZONES[] = {
  {  23,  73, 131, 35, 0, 0, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* red card   (debug) */
  { 162,  73, 131, 35, 0, 1, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* green card (debug) */
  {  23, 112, 131, 35, 0, 2, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* black card (debug) */
  { 162, 112, 131, 35, 0, 3, NAV_NONE,   ACT_BOX_TOGGLE, NULL },                               /* blue card  (debug) */
  {  14, 157, 289, 40, 0, 0, NAV_NONE,   ACT_PICK_CYCLE, NULL },                               /* Pick Color (debug) */
  {  14, 201,  78, 28, 0, 0, SCR_SELECT, ACT_NONE,       NULL },                               /* Back     */
#if VOICE_TOUCH_FLOW
  { 224, 201,  78, 28, 0, 0, NAV_NONE,   ACT_CONFIRM_PICK, NULL },                             /* Proceed  */
#endif
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
/* the line under the Z row, above the Back button, left of the arm picture: normally "XYZ ESTIMATED, mm"
 * (the numbers are FK from the COMMANDED angles, no joint sensors: an estimate). while a message from
 * lcd_show_message() is active (JOINT LIMIT, OUT OF REACH, ALIGN J4 88 ...) it shows that instead, in
 * yellow, so a refused or blocked move is visible on the picture pages too. redrawn only on change */
#define VALUE_LABEL_W     132                   /* x 14 + 132 stays left of the arm picture (x 150) */
#define VALUE_LABEL_CHARS 21                    /* size-1 text, 6 px per character */
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
  s_label_shown[0] = '\0';                      /* new page: always draw it */
  draw_value_label();
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
  s_speed_shown = -1;                           /* FINE / FAST tag drawn by lcd_step */
}

/* FINE / FAST tag of the joystick speed (KY-023 press). manual pages have no Proceed button, so its
 * spot at the bottom right is free */
static void draw_speed_tag(int fast)
{
  lcd_port_fill(SPEED_TAG_X, SPEED_TAG_Y, SPEED_TAG_W, SPEED_TAG_H, LCD_BG);
  lcd_port_text(SPEED_TAG_X + 15, SPEED_TAG_Y + 6, fast ? "FAST" : "FINE", fast ? LCD_YELLOW : LCD_WHITE, LCD_BG, 2);
  s_speed_shown = fast;
}

/* Cartesian page: the encoder target. The mode owns it; this only draws the getter and redraws when
 * it changes (the touch zone posts EVT_TOUCH_SELECT v[0] = 0). */
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

/* the page shows the joint / group the joystick really moves: the mode owns the selection */
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
  lcd_port_asset(LCD_A_CARD(i), CX[i], CY[i]);                  /* the card without a check mark ... */
  if (s_detected[i]) lcd_port_asset(LCD_A_CHECK, CX[i] + LCD_CHECK_DX, CY[i] + LCD_CHECK_DY);   /* ... then the camera's tick */
  s_det_drawn[i] = s_detected[i] ? 1 : 0;
}

static void draw_pick_text(void)
{
  lcd_port_asset(LCD_A_PICK_TXT(s_pick >= 0 ? s_pick : 4), LCD_POS_PICK_TXT_X, LCD_POS_PICK_TXT_Y);
  s_pick_drawn = s_pick;
}

/* voice page, right of the Back button (the Proceed spot is free while VOICE_TOUCH_FLOW is 0): two size-1
 * lines. line 1 (white) = the voice task state and its detail, e.g. "S2 CONFIRM BOX  260 20 15 mm", so the
 * position can be checked before saying yes. line 2 (yellow) = the latest lcd_show_message() for a few
 * seconds (OUT OF REACH, JOINT LIMIT, BOX NOT FOUND, PI NOT RESPONDING ...). redrawn only on change */
#if !VOICE_TOUCH_FLOW
#define VINFO_X      100
#define VINFO_W      216                        /* to x 316 */
#define VINFO_CHARS  36                         /* 6 px per character at size 1 */
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
#endif /* !VOICE_TOUCH_FLOW */

/* voice page, jog queue pop-up (lcd_voice_set_queue). drawn over the camera panel only (x 14..302,
 * y 49..151): the header (jog step slider, y < 40), "Pick Color" and the Back row stay free. it is NOT an
 * overlay: poll_touch() never sees it, so the slider and the other zones keep working under it. while it is
 * up the camera cards are not drawn (lcd_step); hiding it redraws the panel and the cards. only on change */
#define QPOP_X      LCD_POS_PANEL_CAM_X
#define QPOP_Y      LCD_POS_PANEL_CAM_Y
#define QPOP_W      289                         /* the camera panel picture */
#define QPOP_H      103
#define QPOP_CHARS  40                          /* lcd_port_text() draws at most 40 characters per call */
#define QPOP_LINES  5
static void draw_queue_popup(void)
{
  if (s_queue_txt[0] == '\0') {
    if (!s_queue_on_glass) return;
    s_queue_on_glass = false;
    s_queue_drawn[0] = '\0';
    lcd_port_asset(LCD_A_PANEL_CAM, LCD_POS_PANEL_CAM_X, LCD_POS_PANEL_CAM_Y);   /* what it was covering */
    for (int i = 0; i < LCD_BOX_COUNT; i++) draw_card(i);
    return;
  }
  if (s_queue_on_glass && strcmp(s_queue_drawn, s_queue_txt) == 0) return;
  lcd_port_fill(QPOP_X, QPOP_Y, QPOP_W, QPOP_H, LCD_GREY);                  /* 1 px frame */
  lcd_port_fill(QPOP_X + 1, QPOP_Y + 1, QPOP_W - 2, QPOP_H - 2, LCD_ROW_BG);
  lcd_port_text(QPOP_X + 8, QPOP_Y + 8, "JOG QUEUE   say YES / UNDO / NO", LCD_YELLOW, LCD_ROW_BG, 1);
  /* word wrap: break after ", " so one entry ("forward 1.25") never splits */
  const char *p = s_queue_txt;
  for (int line = 0; line < QPOP_LINES && *p != '\0'; line++) {
    int len = (int)strlen(p);
    if (len > QPOP_CHARS) {
      len = QPOP_CHARS;
      while (len > 0 && p[len] != ' ') len--;     /* last space that still fits */
      if (len == 0) len = QPOP_CHARS;             /* no space at all: hard cut */
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
  int cx = SLIDER_X0 + (s_dist * 29) / 4;                       /* 7.25 px per cm */
  lcd_port_asset(LCD_A_SLIDER, LCD_POS_SLIDER_X, LCD_POS_SLIDER_Y);
  lcd_port_fill(SLIDER_X0, SLIDER_TRACK_Y, cx - SLIDER_X0, 3, LCD_WHITE);   /* the part already chosen */
  lcd_port_asset(LCD_A_KNOB, cx - 6, SLIDER_TRACK_Y - 5);
  lcd_port_asset(LCD_A_DIST(s_dist), LCD_POS_DIST_VAL_X, LCD_POS_DIST_VAL_Y);
}

/* voice page, top right in the header (right of "VOICE MODE"): the jog-queue step, 0.00..2.00 cm in
 * 0.05 cm steps. tap or drag; each change posts EVT_TOUCH_STEP_CM (0.1 mm) to the voice task, which uses
 * it for the next jog words. the header background there is LCD_BG */
#define JOG_SL_X0     176                       /* track: x 176..304 = 0.00..2.00 cm */
#define JOG_SL_W      128
#define JOG_SL_Y      27                        /* track line (3 px) */
#define JOG_SL_TOUCH_X 160                      /* touch area x 160..319, y 0..39 */
#define JOG_SL_MAX    200
static void draw_jog_slider(void)
{
  char t[48];
  int cx = JOG_SL_X0 + (s_jog01 * JOG_SL_W) / JOG_SL_MAX;
  lcd_port_fill(JOG_SL_TOUCH_X, 0, LCD_WIDTH_PX - JOG_SL_TOUCH_X, 38, LCD_BG);
  snprintf(t, sizeof t, "JOG STEP %d.%02d cm", s_jog01 / 100, s_jog01 % 100);
  lcd_port_text(JOG_SL_X0, 8, t, LCD_WHITE, LCD_BG, 1);
  lcd_port_fill(JOG_SL_X0, JOG_SL_Y, JOG_SL_W, 3, LCD_GREY);            /* the whole range */
  if (cx > JOG_SL_X0) lcd_port_fill(JOG_SL_X0, JOG_SL_Y, cx - JOG_SL_X0, 3, LCD_WHITE);   /* chosen part */
  lcd_port_asset(LCD_A_KNOB, cx - 6, JOG_SL_Y - 5);
  s_vspeed_shown = -1;                          /* the fill above cleared the speed tag too */
}

/* voice page, header right end, same row as the slider label: the motion speed the voice moves use
 * (FINE white / FAST yellow; it is toggled in the manual modes with the joystick press) */
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
  int v = ((x - JOG_SL_X0) * (JOG_SL_MAX / 5) + JOG_SL_W / 2) / JOG_SL_W * 5;   /* nearest 0.05 cm */
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
#if APP_USE_MODES
      s_servo = mode_single_joint_selected();   /* open on the joint the joystick moves */
#endif
      begin_value_page();
      lcd_port_asset(LCD_A_HDR_SINGLE, 0, 0);
      lcd_port_asset(LCD_A_SEL_NARROW, LCD_POS_SINGLE_SEL_X, LCD_POS_SINGLE_SEL_Y);
      draw_single_selection();                  /* arm picture + label (they do not overlap the frame) */
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
      s_cart_enc_shown = -1;                    /* the encoder target is drawn from the mode getter */
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
      lcd_port_asset(LCD_A_BTN_BACK, LCD_POS_BTN_BACK_X, LCD_POS_BTN_BACK_Y);   /* no Proceed: words drive */
      memset(s_vinfo_shown, 0, sizeof s_vinfo_shown);
      s_vinfo_shown[0][0] = s_vinfo_shown[1][0] = '\x01';   /* never matches: draw both lines now */
      draw_voice_info();
#endif
      s_queue_on_glass = false;                 /* the page was just drawn under it: draw it again on top */
      draw_queue_popup();
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

/* ESTIMATED X/Y/Z of the tool (FK of the commanded angles, labelled on the page). FK off or degenerate
 * (kin_fk not KIN_OK) -> "---" instead of a made-up number. FK never enables IK or motion.
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
#if !DEBUG_STATUS_TAP
  close_overlay();                   /* a tap hides the pop-up (the page and the other modes stay usable);
                                      * the voice task itself is not changed by it */
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
      /* the mode changes its selection like a knob click; sync_selection() redraws from it */
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
  if (s_screen == SCR_VOICE && down && x >= JOG_SL_TOUCH_X && y < 40) {
    jog_slider_set_from_x(x);                   /* jog step: tap or drag */
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
    draw_queue_popup();                         /* first: hiding it redraws the cards below */
    for (int i = 0; i < LCD_BOX_COUNT && !s_queue_on_glass; i++) {   /* not under the queue pop-up */
      if (s_det_drawn[i] != (s_detected[i] ? 1 : 0)) draw_card(i);
    }
    if (s_pick_drawn != s_pick) draw_pick_text();
    draw_voice_speed();                         /* only when FINE / FAST changed */
#if !VOICE_TOUCH_FLOW
    draw_voice_info();
#endif
    return;
  }
  if (s_screen == SCR_SINGLE || s_screen == SCR_GROUP || s_screen == SCR_CART) {
    sync_selection();                           /* every pass: the picture follows a knob click at once */
    int fast = (motion_get_speed() == SPEED_FAST) ? 1 : 0;
    if (fast != s_speed_shown) draw_speed_tag(fast);
    if (s_screen == SCR_CART) draw_cart_enc_button();
    draw_value_label();                         /* message or the XYZ label, only when it changes */
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
