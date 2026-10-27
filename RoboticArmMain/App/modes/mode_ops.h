/* mode_ops.h - the contract every mode file implements (internal to modes/)
 *
 * a mode never touches PWM. it reads inputs / events and sets motion targets.
 * all functions run in the main loop.
 */
#ifndef APP_MODE_OPS_H
#define APP_MODE_OPS_H

#include <stdbool.h>
#include "common/app_events.h"
#include "lcd/lcd.h"

typedef struct {
  const char *name;            /* shown on the lcd */
  bool manual;                 /* manual modes: joystick press = fine/fast */
  void (*enter)(void);         /* targets already = current pose when this is called */
  void (*exit)(void);          /* stop accepting input, cancel tasks / pi requests */
  void (*on_event)(const app_event_t *e);
  void (*step)(void);          /* every main-loop pass while active, keep it short */
  void (*describe)(char (*lines)[LCD_COLS], int n);
} mode_ops_t;

extern const mode_ops_t MODE_VOICE_OPS;
extern const mode_ops_t MODE_CARTESIAN_OPS;
extern const mode_ops_t MODE_JOINT_GROUP_OPS;
extern const mode_ops_t MODE_SINGLE_JOINT_OPS;

/* shared helpers for the joystick modes (mode_manual.c) */
void        manual_jog(int joint_x, int joint_y);   /* joystick x/y -> jog; -1 = axis unused */
const char *manual_joint_name(int joint);

#endif /* APP_MODE_OPS_H */
