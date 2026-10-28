/* mode_cartesian.h - read-only state for the Cartesian LCD page
 *
 * owner: modes / IK (same owner as mode_cartesian.c).
 * The mode owns the encoder target and the accepted pitch; the LCD only draws what these return.
 */
#ifndef APP_MODE_CARTESIAN_H
#define APP_MODE_CARTESIAN_H

#include <stdbool.h>

/* true = encoder detents adjust pitch (1 deg each); false = Z (5 mm each). */
bool  mode_cartesian_encoder_pitch(void);

/* true = the mode is ready to accept Cartesian requests (guard, roll and FK passed). */
bool  mode_cartesian_ready(void);

/* the last accepted requested pitch, degrees, wrapped to [-180, 180). */
float mode_cartesian_pitch_deg(void);

#endif /* APP_MODE_CARTESIAN_H */
