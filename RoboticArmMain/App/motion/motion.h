/* motion.h - modes set targets here; the 50 Hz TIM6 tick moves toward them and writes the PWM
 *
 * Two ways to move (main loop only):
 *   absolute target: motion_set_target(), motion_set_joint_target()  (voice, cartesian)
 *   jog speed:       motion_jog(joint, deg_per_s)  (joystick modes). Refresh it every pass,
 *                    it stops after APP_JOG_TIMEOUT_MS without a refresh.
 * J2/J5 coupling: see motion.c. A clamped target returns MOTION_CLAMPED right away.
 */
#ifndef APP_MOTION_H
#define APP_MOTION_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"

/* J2 below MOTION_J2_COUPLED_DEG needs J5 >= MOTION_J5_WRIST_UP_DEG */
#define MOTION_J2_COUPLED_DEG   90.0f
#define MOTION_J5_WRIST_UP_DEG  65.0f

/* boot: one joint gets its home pulse every this many ms, gripper first */
#define MOTION_BOOT_STAGGER_MS  500u

typedef enum {
  MOTION_OK = 0,
  MOTION_CLAMPED,       /* limited to min/max (exact setter: refused, nothing changed) */
  MOTION_REFUSED        /* not accepted (bad joint index, outputs not started) */
} motion_result_t;

void            motion_init(void);
void            motion_tick(void);                 /* TIM6 isr, after input_tick() */

motion_result_t motion_set_target(const joint_vec_t *target);
/* all or nothing: if any joint would be clamped, nothing changes and MOTION_CLAMPED is returned */
motion_result_t motion_set_target_exact(const joint_vec_t *target);
motion_result_t motion_set_joint_target(int joint, float deg);
void            motion_jog(int joint, float deg_per_s);
void            motion_hold(void);                 /* target = current commanded angle, all joints */
void            motion_hold_joint(int joint);

void            motion_get_current(joint_vec_t *out);   /* commanded now (no joint encoders) */
void            motion_get_target(joint_vec_t *out);
bool            motion_is_idle(void);
bool            motion_started(void);              /* every joint has had its first (home) pulse */

void            motion_set_speed(speed_level_t s);
speed_level_t   motion_get_speed(void);
void            motion_toggle_speed(void);

#endif /* APP_MOTION_H */
