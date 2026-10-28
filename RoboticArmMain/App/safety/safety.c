/* safety.c - motion is allowed unless a fatal fault was raised */
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
