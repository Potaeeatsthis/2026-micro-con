/* servo_cal_test.c - see servo_cal_test.h */
#include "servo_cal_test/servo_cal_test.h"
#include "common/app_events.h"
#include "common/app_fault.h"
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

#define STEP_FINE_DEG   1
#define STEP_COARSE_DEG 5
#define SAVE_JOY_Y      (-0.8f)            /* joystick pushed down past this = print pose (down is negative) */
#define REDRAW_MS       100u               /* while a joint is still moving */
#define TEXT_SIZE       2
#define LINE_H          26                 /* 5x5 font at size 2 plus a gap: 9 lines fit in 240 px */
#define TEXT_X          4

static int      s_sel;                     /* selected joint 0..5 */
static int      s_step = STEP_FINE_DEG;
static bool     s_save_armed = true;       /* the stick must come back to centre between prints */
static int      s_prints;
static bool     s_started_shown;
static bool     s_fault_shown;
static char     s_status[40] = "Going home...";
static uint16_t s_status_fg = LCD_YELLOW;
static bool     s_dirty = true;
static uint32_t s_draw_ms;

int         servo_cal_test_selected(void) { return s_sel; }
int         servo_cal_test_step_deg(void) { return s_step; }
const char *servo_cal_test_status(void)   { return s_status; }

static int deg_of(const joint_vec_t *v, int j) { return (int)(v->deg[j] + (v->deg[j] >= 0.0f ? 0.5f : -0.5f)); }

static void set_status(const char *s, uint16_t fg)
{
  snprintf(s_status, sizeof s_status, "%s", s);
  s_status_fg = fg;
  s_dirty = true;
}

static void draw_line(int row, const char *s, uint16_t fg)
{
  char buf[27];                            /* 26 chars fit at size 2: pad so old text is overwritten */
  snprintf(buf, sizeof buf, "%-26.26s", s);
  lcd_port_text(TEXT_X, row * LINE_H + 4, buf, fg, LCD_BLACK, TEXT_SIZE);
}

static void redraw(void)
{
  joint_vec_t cur, tgt;
  motion_get_current(&cur);
  motion_get_target(&tgt);
  char s[40];
  snprintf(s, sizeof s, "LIMIT TEST  %d deg/click", s_step);
  draw_line(0, s, LCD_YELLOW);
  const servo_cal_t *c = servo_cal(s_sel);
  snprintf(s, sizeof s, "%s %d-%d", c->name, (int)c->min_deg, (int)c->max_deg);
  draw_line(1, s, LCD_WHITE);
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    int d = deg_of(&cur, j);
    bool moving = deg_of(&tgt, j) != d;
    snprintf(s, sizeof s, "%s %-14s %3d%s%4u", (j == s_sel) ? ">" : " ", servo_cal(j)->name, d,
             moving ? "*" : " ", (unsigned)servo_deg_to_us(j, cur.deg[j]));
    draw_line(2 + j, s, (j == s_sel) ? LCD_WHITE : LCD_GREEN);
  }
  draw_line(8, s_status, s_status_fg);
}

void servo_cal_test_init(void)
{
  lcd_port_init();                         /* landscape, turned 180 deg like the main screen, black */
  LOG("servo-cal: motion sends home one joint at a time; then press = next joint, turn = %d deg per click "
      "(joystick press: %d <-> %d), joystick down = print pose\r\n", s_step, STEP_FINE_DEG, STEP_COARSE_DEG);
}

static void turn(int clicks)
{
  joint_vec_t tgt;
  motion_get_target(&tgt);
  float want = tgt.deg[s_sel] + (float)(clicks * s_step);
  motion_result_t r = motion_set_joint_target(s_sel, want);
  motion_get_target(&tgt);
  int got = deg_of(&tgt, s_sel);
  const servo_cal_t *c = servo_cal(s_sel);
  char s[40];
  if (r == MOTION_REFUSED) {
    set_status("MOTION REFUSED", LCD_RED);
  } else if (r == MOTION_CLAMPED && want < tgt.deg[s_sel] && tgt.deg[s_sel] > c->min_deg) {
    if (s_sel == J2_SHOULDER) snprintf(s, sizeof s, "J2 MIN %d: raise J5 to %d", got, (int)MOTION_J5_WRIST_UP_DEG);
    else                      snprintf(s, sizeof s, "J5 MIN %d: J2 below %d", got, (int)MOTION_J2_COUPLED_DEG);
    set_status(s, LCD_RED);
    LOG("servo-cal: %s coupled limit, wanted %d, target %d deg\r\n", c->name, (int)want, got);
  } else if (r == MOTION_CLAMPED) {
    snprintf(s, sizeof s, "%s at %s %d", c->name, (want < tgt.deg[s_sel]) ? "MIN" : "MAX", got);
    set_status(s, LCD_RED);
    LOG("servo-cal: %s limit, wanted %d, target %d deg\r\n", c->name, (int)want, got);
  } else {
    set_status("", LCD_GREY);
    LOG("servo-cal: %s target %d deg (%u us)\r\n", c->name, got, (unsigned)servo_deg_to_us(s_sel, tgt.deg[s_sel]));
  }
}

/* min/max are always the servo_cal.c table: the coupled bounds depend on the pose and are enforced in motion.c */
static void print_pose(void)
{
  joint_vec_t cur;
  motion_get_current(&cur);
  s_prints++;
  LOG("servo-cal: POSE #%d as servo_cal.c rows (home = this pose; min/max = the table, unchanged):\r\n", s_prints);
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    const servo_cal_t *c = servo_cal(j);
    LOG("  { \"%s\", %u, %u, %u, %d.0f, %d.0f, %d.0f },\r\n", c->name, (unsigned)c->us_0, (unsigned)c->us_90,
        (unsigned)c->us_180, (int)c->min_deg, (int)c->max_deg, deg_of(&cur, j));
  }
  char s[40];
  snprintf(s, sizeof s, "POSE #%d printed (log)", s_prints);
  set_status(s, LCD_GREEN);
}

void servo_cal_test_step(void)
{
  app_event_t e;
  if (!safety_motion_allowed()) {          /* fatal fault latched: motion is frozen until reset */
    while (app_event_pop(&e)) { }          /* knob and stick ignored: never touch a target now */
    if (!s_fault_shown) {
      s_fault_shown = true;
      char s[40];
      snprintf(s, sizeof s, "FAULT %d - RESET BOARD", (int)app_fault_last());
      set_status(s, LCD_RED);
      LOG("servo-cal: fatal fault %d, motion frozen, knob ignored; reset the board\r\n", (int)app_fault_last());
    }
  } else if (!motion_started()) {
    while (app_event_pop(&e)) { }          /* no knob or stick until every joint has its home pulse */
  } else {
    if (!s_started_shown) {
      s_started_shown = true;
      set_status("Home. Turn to test limits", LCD_GREEN);
    }
    while (app_event_pop(&e)) {
      if (e.type == EVT_ENC_PRESS) {
        s_sel = (s_sel + 1) % ARM_N_JOINTS;
        set_status("", LCD_GREY);
      } else if (e.type == EVT_ENC_STEP) {
        turn((int)e.v[0]);
      } else if (e.type == EVT_JOY_PRESS) {
        s_step = (s_step == STEP_FINE_DEG) ? STEP_COARSE_DEG : STEP_FINE_DEG;
        s_dirty = true;
      }
    }
    float y = input_joy_y();
    if (s_save_armed && y < SAVE_JOY_Y) {
      s_save_armed = false;
      print_pose();
    } else if (!s_save_armed && input_joy_centered()) {
      s_save_armed = true;
    }
  }

  uint32_t now = HAL_GetTick();
  if (s_dirty || (!motion_is_idle() && ms_since(now, s_draw_ms) >= REDRAW_MS)) {
    s_dirty = false;
    s_draw_ms = now;
    redraw();
  }
}
