/* modes.c - active mode, event loop and safe switching
 *
 * switching: old mode exit(), motion_hold(), wait until idle and the stick is centred, new mode enter()
 */
#include "modes/modes.h"
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "lcd/lcd.h"
#include <stdio.h>
#if APP_USE_INPUT
#include "input/input.h"
#endif

static const mode_ops_t *const MODE_TABLE[MODE_COUNT] = {
  [MODE_VOICE]        = &MODE_VOICE_OPS,
  [MODE_CARTESIAN]    = &MODE_CARTESIAN_OPS,
  [MODE_JOINT_GROUP]  = &MODE_JOINT_GROUP_OPS,
  [MODE_SINGLE_JOINT] = &MODE_SINGLE_JOINT_OPS,
};

static arm_mode_t s_cur = MODE_VOICE;
static arm_mode_t s_next = MODE_VOICE;
static bool       s_switching;
static const char *s_wait_reason = "";

static const mode_ops_t *ops(arm_mode_t m) { return MODE_TABLE[m]; }

static void enter_mode(arm_mode_t m)
{
  s_cur = m;
  ops(m)->enter();
  LOG("modes: -> %s\r\n", ops(m)->name);
}

void modes_init(void)
{
  enter_mode(MODE_VOICE);      /* boot in the mode that never moves without a spoken command */
}

void modes_request(arm_mode_t m)
{
  if (m >= MODE_COUNT || m == s_cur || s_switching) return;
  ops(s_cur)->exit();
  motion_hold();
  s_next = m;
  s_switching = true;
}

static bool joystick_centred(void)
{
#if APP_USE_INPUT
  return input_joy_centered();
#else
  return true;
#endif
}

static void switch_step(void)
{
  if (!motion_is_idle())         { s_wait_reason = "STOPPING";       return; }
  if (!joystick_centred())       { s_wait_reason = "CENTRE STICK";   return; }
  motion_hold();
  s_switching = false;
  s_wait_reason = "";
  enter_mode(s_next);
}

/* true = handled here, do not forward */
static bool handle_global(const app_event_t *e)
{
  switch ((evt_type_t)e->type) {
    case EVT_TOUCH_MODE_TAB:
      modes_request((arm_mode_t)e->arg);
      return true;
    case EVT_JOY_PRESS:
      /* fine/fast in the manual modes and voice */
      if (ops(s_cur)->manual || s_cur == MODE_VOICE) motion_toggle_speed();
      return true;
    case EVT_FAULT:
      if (e->v[0] != 0) lcd_show_message("FAULT - RESET BOARD", 10000u);
      return false;
    default:
      return false;
  }
}

void modes_step(void)
{
  app_event_t e;
  while (app_event_pop(&e)) {
    if (handle_global(&e)) continue;
    if (s_switching) continue;
    ops(s_cur)->on_event(&e);
  }

  if (app_events_dropped() != 0u) {
    static uint32_t s_dropped_seen;
    if (app_events_dropped() != s_dropped_seen) {
      s_dropped_seen = app_events_dropped();
      app_fault_raise(FAULT_EVENT_DROPPED, false);
    }
  }

  if (s_switching) {
    switch_step();
  } else {
    ops(s_cur)->step();
  }
}

arm_mode_t  modes_current(void)        { return s_cur; }
bool        modes_switch_pending(void) { return s_switching; }
const char *modes_name(arm_mode_t m)   { return (m < MODE_COUNT) ? ops(m)->name : "?"; }

void modes_describe(char (*lines)[LCD_COLS], int n)
{
  if (s_switching) {
    snprintf(lines[0], LCD_COLS, "-> %s", ops(s_next)->name);
    if (n > 1) snprintf(lines[1], LCD_COLS, "%s", s_wait_reason);
    return;
  }
  ops(s_cur)->describe(lines, n);
}
