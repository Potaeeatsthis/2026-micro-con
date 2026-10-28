/* gripper.h - open / close J6 for every mode (a lower angle closes)
 *
 * A command counts only if motion takes it unchanged; a clamped one is cancelled and reported as
 * failed, so a clamped close never counts as holding.
 */
#ifndef APP_GRIPPER_H
#define APP_GRIPPER_H

#include <stdbool.h>

bool gripper_set_deg(float deg);         /* false = refused or clamped */
bool gripper_command(bool close);        /* gripper_set_deg(CLOSED or OPEN) */
bool gripper_is_closed_at(float deg);    /* true if deg is nearer the closed than the open setpoint */

#endif /* APP_GRIPPER_H */
