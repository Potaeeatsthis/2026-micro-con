/* mode_cartesian.h - read-only state for the Cartesian LCD page */
#ifndef APP_MODE_CARTESIAN_H
#define APP_MODE_CARTESIAN_H

#include <stdbool.h>

/* true = knob changes pitch (1 deg), false = Z (5 mm) */
bool  mode_cartesian_encoder_pitch(void);

/* ready to take Cartesian requests */
bool  mode_cartesian_ready(void);

/* last accepted pitch, -180..180 */
float mode_cartesian_pitch_deg(void);

#endif /* APP_MODE_CARTESIAN_H */
