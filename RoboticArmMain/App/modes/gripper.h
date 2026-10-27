/* gripper.h - open / close the J6 gripper for every mode, and read its state
 *
 * owner: modes. the setpoints are APP_GRIPPER_OPEN_DEG / APP_GRIPPER_CLOSED_DEG (app_config.h);
 * on the J6 replacement servo a lower angle closes.
 * a command counts only if motion accepts it unchanged: a clamped one (the setpoint is outside
 * servo_cal's J6 limits) is cancelled with motion_hold_joint(J6) and reported as failed, so a
 * clamped close is never taken as "holding an object".
 */
#ifndef APP_GRIPPER_H
#define APP_GRIPPER_H

#include <stdbool.h>

bool gripper_set_deg(float deg);         /* true = accepted exactly; false = refused or clamped (J6 held) */
bool gripper_command(bool close);        /* gripper_set_deg(CLOSED or OPEN) */
bool gripper_is_closed_at(float deg);    /* true if deg is nearer the closed than the open setpoint */

#endif /* APP_GRIPPER_H */
