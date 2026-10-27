/* motion.h - the shared motion pipeline (guide section 8)
 *
 * owner: motion member.
 * every mode only sets TARGETS here. the TIM6 tick (50 Hz) moves the commanded angle
 * toward the target at a limited speed, checks joint limits and safety, then writes PWM.
 *
 * two ways to command motion (main loop only):
 *   1. absolute target:  motion_set_target(), motion_set_joint_target()     (voice, cartesian)
 *   2. jog velocity:     motion_jog(joint, deg_per_s)                       (joystick modes)
 *      a jog must be refreshed every main-loop pass; if it is not refreshed within
 *      APP_JOG_TIMEOUT_MS it drops to 0, so a hung main loop cannot keep the arm moving.
 */
#ifndef APP_MOTION_H
#define APP_MOTION_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"

typedef enum {
  MOTION_OK = 0,
  MOTION_CLAMPED,       /* accepted, but at least one joint was limited to its min/max */
  MOTION_REFUSED        /* not accepted (bad joint index, outputs not started) */
} motion_result_t;

void            motion_init(void);
void            motion_tick(void);                 /* TIM6 isr, after input_tick() */

motion_result_t motion_set_target(const joint_vec_t *target);
motion_result_t motion_set_joint_target(int joint, float deg);
void            motion_jog(int joint, float deg_per_s);
void            motion_hold(void);                 /* target = current commanded angle, all joints */
void            motion_hold_joint(int joint);

void            motion_get_current(joint_vec_t *out);   /* commanded now (no joint encoders) */
void            motion_get_target(joint_vec_t *out);
bool            motion_is_idle(void);

void            motion_set_speed(speed_level_t s);
speed_level_t   motion_get_speed(void);
void            motion_toggle_speed(void);

#endif /* APP_MOTION_H */
