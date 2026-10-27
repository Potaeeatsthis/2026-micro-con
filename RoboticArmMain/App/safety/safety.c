/* safety.c
 *
 * motion is allowed unless a fatal fault was raised (app_fault_raise(..., true)).
 * removed hardware (core-config.md v1.4, 27 Oct 2026): e-stop, hold-to-enable and MODE buttons
 * (PE7 / PE12 / PE10), servo-rail sense (PC3) and the limit switches (PE8 / PB11 / PE0 / PE2).
 */
#include "safety/safety.h"
#include "common/app_fault.h"

safety_state_t safety_state(void)
{
  return app_fault_is_fatal() ? SAFETY_FATAL_FAULT : SAFETY_OK;
}

bool safety_motion_allowed(void) { return safety_state() == SAFETY_OK; }

const char *safety_state_name(safety_state_t s)
{
  switch (s) {
    case SAFETY_OK:          return "OK";
    case SAFETY_FATAL_FAULT: return "FAULT";
    default:                 return "?";
  }
}
