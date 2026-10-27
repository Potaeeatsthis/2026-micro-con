/* safety.h - fatal-fault gate for motion
 *
 * owner: core (Supanut). always compiled (no feature flag).
 *
 * motion asks safety_motion_allowed() every 20 ms tick; if the answer becomes "no",
 * motion freezes its targets in the same tick.
 *
 * the arm has no e-stop, no hold-to-enable button, no servo-rail sense and no limit switches
 * (core-config.md v1.4): nothing cuts servo power in hardware. the only stop condition here is
 * a fatal fault; joint limits are enforced in motion/ (servo_cal).
 */
#ifndef APP_SAFETY_H
#define APP_SAFETY_H

#include <stdbool.h>

typedef enum {
  SAFETY_OK = 0,
  SAFETY_FATAL_FAULT        /* app_fault_raise(..., true) was called */
} safety_state_t;

bool           safety_motion_allowed(void);
safety_state_t safety_state(void);
const char    *safety_state_name(safety_state_t s);

#endif /* APP_SAFETY_H */
