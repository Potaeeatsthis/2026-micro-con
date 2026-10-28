/* safety.h - fatal-fault gate for motion (always compiled)
 *
 * motion checks safety_motion_allowed() every tick and freezes if it says no. There is no e-stop
 * or limit switch in hardware; joint limits are in motion/.
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
