/* ik_bench_test.c - see ik_bench_test.h. only built with APP_USE_IK_BENCH_TEST 1 */
#include "common/app_config.h"
#if APP_USE_IK_BENCH_TEST
#include "ik_bench_test/ik_bench_test.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "kinematics/arm_geometry.h"
#include "kinematics/kinematics.h"
#include "lcd/lcd_port.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "main.h"
#include <stdbool.h>
#include <stdio.h>

/* the target (robot frame). default: 27 mm below home, about 12.5 deg on J3 */
#ifndef IK_BENCH_X_MM
#define IK_BENCH_X_MM          250.0f
#endif
#ifndef IK_BENCH_Y_MM
#define IK_BENCH_Y_MM          13.0f
#endif
#ifndef IK_BENCH_Z_MM
#define IK_BENCH_Z_MM          120.0f
#endif
#ifndef IK_BENCH_PITCH_DEG
#define IK_BENCH_PITCH_DEG     150.0f
#endif
#ifndef IK_BENCH_MAX_DELTA_DEG
#define IK_BENCH_MAX_DELTA_DEG 30.0f    /* refuse a solution that moves any joint further than this */
#endif

#ifndef IK_BENCH_ARM_MS
#define IK_BENCH_ARM_MS        500u     /* GO needs a press at least this long after READY */
#endif

#define REDRAW_MS  100u                 /* while the arm is moving */
#define TEXT_SIZE  2
#define LINE_H     26
#define TEXT_X     4

static const cart_pose_t TARGET = { IK_BENCH_X_MM, IK_BENCH_Y_MM, IK_BENCH_Z_MM, IK_BENCH_PITCH_DEG };

static ik_bench_state_t s_state = IKB_BOOT;
static joint_vec_t      s_sol;
static bool             s_have_sol;
static uint32_t         s_ready_ms;
static int              s_max_j;        /* joint with the biggest change, and how much */
static float            s_max_d;
static bool             s_have_err;     /* after arriving: FK of the reached pose minus the target */
static cart_pose_t      s_reached;
static char             s_status[40] = "Going home...";
static uint16_t         s_status_fg = LCD_YELLOW;
static bool             s_dirty = true;
static uint32_t         s_draw_ms;

ik_bench_state_t ik_bench_test_state(void)  { return s_state; }
const char      *ik_bench_test_status(void) { return s_status; }
void             ik_bench_test_solution(joint_vec_t *out) { *out = s_sol; }

static int rnd(float v) { return (int)(v + (v >= 0.0f ? 0.5f : -0.5f)); }

static void set_status(ik_bench_state_t st, const char *s, uint16_t fg)
{
  s_state = st;
  snprintf(s_status, sizeof s_status, "%s", s);
  s_status_fg = fg;
  s_dirty = true;
}

static void draw_line(int row, const char *s, uint16_t fg)
{
  char buf[27];                         /* pad so old text is overwritten */
  snprintf(buf, sizeof buf, "%-26.26s", s);
  lcd_port_text(TEXT_X, row * LINE_H + 4, buf, fg, LCD_BLACK, TEXT_SIZE);
}

static void redraw(void)
{
  joint_vec_t cur;
  cart_pose_t est;
  char s[48];
  motion_get_current(&cur);
  draw_line(0, "IK BENCH  ONE TARGET", LCD_YELLOW);
  snprintf(s, sizeof s, "T X%d Y%d Z%d P%d", rnd(TARGET.x_mm), rnd(TARGET.y_mm), rnd(TARGET.z_mm), rnd(TARGET.pitch_deg));
  draw_line(1, s, LCD_WHITE);
  if (s_have_sol) {
    snprintf(s, sizeof s, "IK  %d %d %d %d %d", rnd(s_sol.deg[0]), rnd(s_sol.deg[1]), rnd(s_sol.deg[2]),
             rnd(s_sol.deg[3]), rnd(s_sol.deg[4]));
  } else {
    snprintf(s, sizeof s, "IK  -");
  }
  draw_line(2, s, LCD_GREEN);
  snprintf(s, sizeof s, "NOW %d %d %d %d %d", rnd(cur.deg[0]), rnd(cur.deg[1]), rnd(cur.deg[2]),
           rnd(cur.deg[3]), rnd(cur.deg[4]));
  draw_line(3, s, LCD_WHITE);
  if (kin_fk(&cur, &est) == KIN_OK) {
    snprintf(s, sizeof s, "EST X%d Y%d Z%d P%d", rnd(est.x_mm), rnd(est.y_mm), rnd(est.z_mm), rnd(est.pitch_deg));
  } else {
    snprintf(s, sizeof s, "EST -");
  }
  draw_line(4, s, LCD_WHITE);
  if (s_have_sol) snprintf(s, sizeof s, "MAX MOVE %d deg (J%d)", rnd(s_max_d), s_max_j + 1);
  else            s[0] = '\0';
  draw_line(5, s, LCD_GREY);
  if (s_have_err) {
    snprintf(s, sizeof s, "ERR %d %d %d mm", rnd(s_reached.x_mm - TARGET.x_mm), rnd(s_reached.y_mm - TARGET.y_mm),
             rnd(s_reached.z_mm - TARGET.z_mm));
  } else {
    s[0] = '\0';
  }
  draw_line(6, s, LCD_GREY);
  draw_line(8, s_status, s_status_fg);
}

static const char *joint_line(const joint_vec_t *v, char *buf, int n)
{
  snprintf(buf, (size_t)n, "%d %d %d %d %d", rnd(v->deg[0]), rnd(v->deg[1]), rnd(v->deg[2]), rnd(v->deg[3]),
           rnd(v->deg[4]));
  return buf;
}

/* solve from where the arm is */
static void plan(void)
{
  joint_vec_t now;
  char b[40];
  motion_get_current(&now);
  s_have_sol = false;
  s_have_err = false;
  kin_status_t st = kin_ik_bench(&TARGET, &now, &s_sol);
  if (st != KIN_OK) {
    LOG("ik-bench: target %d %d %d pitch %d: %s\r\n", rnd(TARGET.x_mm), rnd(TARGET.y_mm), rnd(TARGET.z_mm),
        rnd(TARGET.pitch_deg), kin_status_name(st));
    set_status(IKB_NO_SOLUTION, kin_status_name(st), LCD_RED);
    return;
  }
  s_have_sol = true;
  s_max_d = 0.0f;
  s_max_j = 0;
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    float d = absf(s_sol.deg[j] - now.deg[j]);
    if (d > s_max_d) { s_max_d = d; s_max_j = j; }
  }
  LOG("ik-bench: target %d %d %d pitch %d -> J1..J5 %s, biggest change J%d %d deg\r\n", rnd(TARGET.x_mm),
      rnd(TARGET.y_mm), rnd(TARGET.z_mm), rnd(TARGET.pitch_deg), joint_line(&s_sol, b, (int)sizeof b),
      s_max_j + 1, rnd(s_max_d));
  if (s_max_d > IK_BENCH_MAX_DELTA_DEG) {
    snprintf(b, sizeof b, "TOO FAR: J%d %d deg", s_max_j + 1, rnd(s_max_d));
    set_status(IKB_NO_SOLUTION, b, LCD_RED);
    return;
  }
  /* GO must be a new press: compare the press edge time, not when the event is handled.
   * a knob held through start-up needs a release and a new press */
  app_event_t e;
  while (app_event_pop(&e)) { }
  s_ready_ms = HAL_GetTick();
  set_status(IKB_READY, "KNOB PRESS = GO", LCD_GREEN);
}

/* all or nothing: a target motion would clamp is not started */
static bool go(const joint_vec_t *t, ik_bench_state_t moving, const char *msg)
{
  motion_set_speed(SPEED_FINE);
  motion_result_t r = motion_set_target_exact(t);
  if (r != MOTION_OK) {
    motion_hold();
    LOG("ik-bench: motion %s, nothing moved\r\n", r == MOTION_CLAMPED ? "would clamp" : "refused");
    set_status(IKB_NO_SOLUTION, r == MOTION_CLAMPED ? "JOINT LIMIT - not moved" : "MOTION REFUSED", LCD_RED);
    return false;
  }
  set_status(moving, msg, LCD_YELLOW);
  return true;
}

static void go_home(void)
{
  joint_vec_t home, now;
  motion_get_current(&now);
  for (int j = 0; j < ARM_N_JOINTS; j++) home.deg[j] = servo_cal(j)->home_deg;
  home.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];      /* the gripper is never moved here */
  s_have_err = false;
  if (go(&home, IKB_RETURNING, "HOME. ANY PRESS = STOP")) LOG("ik-bench: going home\r\n");
}

static void arrived(void)
{
  joint_vec_t now;
  char b[40];
  motion_get_current(&now);
  s_have_err = (kin_fk(&now, &s_reached) == KIN_OK);
  LOG("ik-bench: arrived, J1..J5 %s\r\n", joint_line(&now, b, (int)sizeof b));
  if (s_have_err) {
    LOG("ik-bench: FK estimate %d %d %d pitch %d (target %d %d %d pitch %d): measure the real tool point\r\n",
        rnd(s_reached.x_mm), rnd(s_reached.y_mm), rnd(s_reached.z_mm), rnd(s_reached.pitch_deg), rnd(TARGET.x_mm),
        rnd(TARGET.y_mm), rnd(TARGET.z_mm), rnd(TARGET.pitch_deg));
  }
  set_status(IKB_AT_TARGET, "MEASURE. PRESS = HOME", LCD_GREEN);
}

void ik_bench_test_init(void)
{
  lcd_port_init();                      /* landscape, rotated 180 like the main screen */
  LOG("ik-bench: kin_ik stays off (ARM_GEOMETRY_MEASURED %d); this screen solves ONE target with "
      "kin_ik_bench. home first, then knob press = go, any press = stop, press = home\r\n", ARM_GEOMETRY_MEASURED);
}

void ik_bench_test_step(void)
{
  app_event_t e;
  if (!safety_motion_allowed()) {
    while (app_event_pop(&e)) { }
    if (s_state != IKB_FAULT) {
      char s[40];
      snprintf(s, sizeof s, "FAULT %d - RESET BOARD", (int)app_fault_last());
      set_status(IKB_FAULT, s, LCD_RED);
      LOG("ik-bench: fatal fault %d, motion frozen; reset the board\r\n", (int)app_fault_last());
    }
  } else if (!motion_started()) {
    while (app_event_pop(&e)) { }
  } else {
    if (s_state == IKB_BOOT) {
      motion_set_speed(SPEED_FINE);
      plan();
    }
    while (app_event_pop(&e)) {
      bool press = (e.type == EVT_ENC_PRESS);
      bool any = press || e.type == EVT_JOY_PRESS;
      switch (s_state) {
        case IKB_READY:
          if (press && (int32_t)(e.tick_ms - s_ready_ms) < (int32_t)IK_BENCH_ARM_MS) {
            LOG("ik-bench: press ignored (pressed %ld ms after READY, press again)\r\n",
                (long)(int32_t)(e.tick_ms - s_ready_ms));
          } else if (press) {
            (void)go(&s_sol, IKB_MOVING, "MOVING. ANY PRESS = STOP");
          }
          break;
        case IKB_NO_SOLUTION:
          if (press) plan();
          break;
        case IKB_MOVING:
        case IKB_RETURNING:
          if (any) {
            motion_hold();
            LOG("ik-bench: STOP\r\n");
            set_status(IKB_STOPPED, "STOPPED. PRESS = HOME", LCD_RED);
          }
          break;
        case IKB_AT_TARGET:
        case IKB_STOPPED:
          if (press) go_home();
          break;
        default:
          break;
      }
    }
    if (motion_is_idle()) {
      if (s_state == IKB_MOVING)    arrived();
      if (s_state == IKB_RETURNING) plan();
    }
  }

  uint32_t now = HAL_GetTick();
  if (s_dirty || (!motion_is_idle() && ms_since(now, s_draw_ms) >= REDRAW_MS)) {
    s_dirty = false;
    s_draw_ms = now;
    redraw();
  }
}
#endif /* APP_USE_IK_BENCH_TEST */
