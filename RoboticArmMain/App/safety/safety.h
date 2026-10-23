/* safety.h - e-stop status, hold-to-enable, servo-rail sense, limit switches, restart latch
 *
 * owner: core (Supanut). always compiled and always running (no feature flag).
 *
 * motion asks two questions every 20 ms tick:
 *   safety_motion_allowed()  - nothing forbids motion at all
 *   safety_enable_held()     - the hold-to-enable button is held (manual modes need it)
 * if the answer becomes "no", motion freezes its targets in the same tick.
 *
 * the hardware e-stop cuts servo power by itself; this module only reports and freezes.
 */
#ifndef APP_SAFETY_H
#define APP_SAFETY_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  SAFETY_OK = 0,
  SAFETY_ESTOP,             /* e-stop button latched */
  SAFETY_RAIL_LOST,         /* servo rail below APP_RAIL_LOST_MV */
  SAFETY_RESTART_REQUIRED,  /* power came back: inspect the arm, then confirm */
  SAFETY_FATAL_FAULT        /* app_fault_raise(..., true) was called */
} safety_state_t;

typedef struct {
  safety_state_t state;
  bool     estop_pressed;
  bool     enable_held;
  bool     rail_sense_used;
  bool     rail_ok;
  uint32_t rail_mv;
  uint8_t  limit_mask;      /* bit i = limit switch i triggered (only if fitted) */
} safety_status_t;

void           safety_init(void);

void           safety_tick(void);                /* TIM6 isr, before motion_tick() */
void           safety_on_exti(uint16_t gpio_pin);/* EXTI isr: enable button, e-stop, limits */
void           safety_step(void);                /* main loop: posts EVT_* on changes */

bool           safety_motion_allowed(void);
bool           safety_enable_held(void);
safety_state_t safety_state(void);
void           safety_get_status(safety_status_t *out);
const char    *safety_state_name(safety_state_t s);

/* true if limit switch logic forbids moving `joint` in direction `dir` (+1 / -1) */
bool           safety_limit_blocks(int joint, int dir);

/* user confirmed the arm was inspected after e-stop / power loss (touch button) */
void           safety_confirm_restart(void);

#endif /* APP_SAFETY_H */
