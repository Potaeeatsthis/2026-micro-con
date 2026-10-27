/* servo_cal_test.c - see servo_cal_test.h */
#include "servo_cal_test/servo_cal_test.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_types.h"
#include "common/app_util.h"
#include "input/input.h"
#include "lcd/lcd_port.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "main.h"
#include <stdbool.h>
#include <stdio.h>

#define STEP_DEG      5
#define BOOT_STEP_MS  MOTION_BOOT_STAGGER_MS  /* same spacing as the main start-up (motion.h) */
#define SAVE_JOY_Y    (-0.8f)              /* joystick pushed down past this = print pose (down is negative) */
#define TEXT_SIZE     2
#define LINE_H        26                   /* 5x5 font at size 2 plus a gap: 9 lines fit in 240 px */
#define TEXT_X        4

static int      s_sel;                     /* selected joint 0..5 */
static bool     s_on[ARM_N_JOINTS];        /* false = no pulse yet */
static int      s_boot_next;               /* index into BOOT_ORDER; ARM_N_JOINTS = done */
/* gripper first: its signal floats during flashing / reset and it may have been driven to an end
 * (stalled), so it gets its home pulse as early as possible; then J1..J5 */
static const int BOOT_ORDER[ARM_N_JOINTS] = { 5, 0, 1, 2, 3, 4 };
static uint32_t s_boot_ms;
static bool     s_save_armed = true;       /* the stick must come back to centre between prints */
static int      s_saves;
static char     s_status[40] = "Going home...";
static uint16_t s_status_fg = LCD_YELLOW;
static bool     s_dirty = true;

/* J2/J5 coupling, the same rule and numbers as motion.c (motion.h): J2 may use its table floor only
 * while J5 >= MOTION_J5_WRIST_UP_DEG, otherwise J2 >= MOTION_J2_COUPLED_DEG; while J2 is below that,
 * J5 >= MOTION_J5_WRIST_UP_DEG. the test moves one joint at a time, so the other joint's current angle
 * is the whole pose. */
#define J2_MIN_DEG       ((int)MOTION_J2_COUPLED_DEG)
#define J5_WRIST_UP_DEG  ((int)MOTION_J5_WRIST_UP_DEG)

static int s_deg[ARM_N_JOINTS];            /* commanded angle (declared early: the coupling reads it) */

static int table_min(int j) { return (int)servo_cal(j)->min_deg; }

static int min_deg(int j)
{
  int lo = table_min(j);
  if (j == J2_SHOULDER && s_deg[J5_WRIST_PITCH] < J5_WRIST_UP_DEG && lo < J2_MIN_DEG) lo = J2_MIN_DEG;
  if (j == J5_WRIST_PITCH && s_deg[J2_SHOULDER] < J2_MIN_DEG && lo < J5_WRIST_UP_DEG) lo = J5_WRIST_UP_DEG;
  return lo;
}
static int max_deg(int j) { return (int)servo_cal(j)->max_deg; }

static void draw_line(int row, const char *s, uint16_t fg)
{
  char buf[27];                            /* 26 chars fit at size 2: pad so old text is overwritten */
  snprintf(buf, sizeof buf, "%-26.26s", s);
  lcd_port_text(TEXT_X, row * LINE_H + 4, buf, fg, LCD_BLACK, TEXT_SIZE);
}

static void redraw(void)
{
  char s[40];
  snprintf(s, sizeof s, "LIMIT TEST  %d deg/click", STEP_DEG);
  draw_line(0, s, LCD_YELLOW);
  snprintf(s, sizeof s, "%s %d-%d", servo_cal(s_sel)->name, min_deg(s_sel), max_deg(s_sel));
  draw_line(1, s, LCD_WHITE);
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    const char *mark = (j == s_sel) ? ">" : " ";
    if (s_on[j]) snprintf(s, sizeof s, "%s %-14s %3d %4u", mark, servo_cal(j)->name, s_deg[j],
                          (unsigned)servo_deg_to_us(j, (float)s_deg[j]));
    else         snprintf(s, sizeof s, "%s %-14s  --", mark, servo_cal(j)->name);
    draw_line(2 + j, s, (j == s_sel) ? LCD_WHITE : (s_on[j] ? LCD_GREEN : LCD_GREY));
  }
  draw_line(8, s_status, s_status_fg);
}

static void set_status(const char *s, uint16_t fg)
{
  snprintf(s_status, sizeof s_status, "%s", s);
  s_status_fg = fg;
}

static void write_joint(int j, int deg)
{
  s_deg[j] = deg;
  s_on[j] = true;
  servo_write_us(j, servo_deg_to_us(j, (float)deg));
}

void servo_cal_test_init(void)
{
  servo_init();                            /* CCR = 0 on all six: no pulse until the home sequence */
  s_boot_ms = HAL_GetTick();
  lcd_port_init();                         /* landscape, turned 180 deg like the main screen, black */
  LOG("servo-cal: home one joint every %lu ms, then press = next joint, turn = %d deg per click within limits\r\n",
      (unsigned long)BOOT_STEP_MS, STEP_DEG);
}

static void boot_step(void)
{
  if (!safety_motion_allowed() || ms_since(HAL_GetTick(), s_boot_ms) < BOOT_STEP_MS) return;
  s_boot_ms = HAL_GetTick();
  int j = BOOT_ORDER[s_boot_next++];
  write_joint(j, (int)servo_cal(j)->home_deg);
  LOG("servo-cal: %s home %d deg (%u us)\r\n", servo_cal(j)->name, s_deg[j],
      (unsigned)servo_deg_to_us(j, (float)s_deg[j]));
  if (s_boot_next == ARM_N_JOINTS) {
    set_status("Home. Turn to test limits", LCD_GREEN);
    LOG("servo-cal: home pose reached\r\n");
  }
  s_dirty = true;
}

static void turn(int clicks)
{
  if (!safety_motion_allowed()) {
    LOG("servo-cal: fatal fault latched, no move\r\n");
    return;
  }
  int want = s_deg[s_sel] + clicks * STEP_DEG;
  int d = want;
  if (d < min_deg(s_sel)) d = min_deg(s_sel);
  if (d > max_deg(s_sel)) d = max_deg(s_sel);
  write_joint(s_sel, d);
  char s[40];
  if (want < d && d > table_min(s_sel)) {                 /* held by the J2/J5 coupling, not the table */
    if (s_sel == J2_SHOULDER) snprintf(s, sizeof s, "J2 MIN %d: raise J5 to %d", d, J5_WRIST_UP_DEG);
    else                      snprintf(s, sizeof s, "J5 MIN %d: J2 is below %d", d, J2_MIN_DEG);
    set_status(s, LCD_RED);
    LOG("servo-cal: %s COUPLED LIMIT, wanted %d, held at %d deg\r\n", servo_cal(s_sel)->name, want, d);
  } else if (want != d) {
    snprintf(s, sizeof s, "%s at %s %d", servo_cal(s_sel)->name, (want < d) ? "MIN" : "MAX", d);
    set_status(s, LCD_RED);
    LOG("servo-cal: %s LIMIT, wanted %d, held at %d deg\r\n", servo_cal(s_sel)->name, want, d);
  } else {
    set_status("", LCD_GREY);
    LOG("servo-cal: %s %d deg (%u us)\r\n", servo_cal(s_sel)->name, d, (unsigned)servo_deg_to_us(s_sel, (float)d));
  }
}

static void print_pose(void)
{
  s_saves++;
  LOG("servo-cal: POSE #%d, servo_cal.c rows with this pose as home (min/max unchanged):\r\n", s_saves);
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    const servo_cal_t *c = servo_cal(j);
    LOG("  { \"%s\", %u, %u, %u, %d.0f, %d.0f, %d.0f },\r\n", c->name, (unsigned)c->us_0, (unsigned)c->us_90,
        (unsigned)c->us_180, min_deg(j), max_deg(j), s_deg[j]);
  }
  char s[40];
  snprintf(s, sizeof s, "POSE #%d printed (log)", s_saves);
  set_status(s, LCD_GREEN);
}

void servo_cal_test_step(void)
{
  app_event_t e;
  if (s_boot_next < ARM_N_JOINTS) {
    boot_step();
    while (app_event_pop(&e)) { }          /* no knob or stick until every joint is home */
  } else {
    while (app_event_pop(&e)) {
      if (e.type == EVT_ENC_PRESS) {
        s_sel = (s_sel + 1) % ARM_N_JOINTS;
        set_status("", LCD_GREY);
        s_dirty = true;
      } else if (e.type == EVT_ENC_STEP) {
        turn((int)e.v[0]);
        s_dirty = true;
      }
    }
    float y = input_joy_y();
    if (s_save_armed && y < SAVE_JOY_Y) {
      s_save_armed = false;
      print_pose();
      s_dirty = true;
    } else if (!s_save_armed && input_joy_centered()) {
      s_save_armed = true;
    }
  }

  if (s_dirty) {
    s_dirty = false;
    redraw();
  }
}
