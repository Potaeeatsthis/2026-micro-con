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
 *
 * J2/J5 coupling is a pose rule. It uses the actual commanded pose (s_cur) plus, for
 * a single-joint request, the pending target of the other joint. J5 >= 65 deg allows
 * J2 down to its absolute 50 deg table floor; otherwise J2 stays >= 90 deg. J2 < 90 deg
 * blocks J5 below 65 deg. A full target that would end below both bounds is clamped
 * deterministically with J2 = 90 deg. Setters report MOTION_CLAMPED immediately and
 * store the clamped endpoint, so reissue after the pose allows instead of expecting a
 * late move. "current" is the commanded angle, not a measured joint sensor.
 */
#ifndef APP_MOTION_H
#define APP_MOTION_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"

/* J2/J5 coupling bounds (see above): J2 below MOTION_J2_COUPLED_DEG needs J5 at or above
 * MOTION_J5_WRIST_UP_DEG */
#define MOTION_J2_COUPLED_DEG   90.0f
#define MOTION_J5_WRIST_UP_DEG  65.0f

/* start-up: each joint gets its first (home) pulse this long after the previous one, gripper first,
 * then J1..J5; trajectories run only after all six have a pulse (about 6 x this after boot) */
#define MOTION_BOOT_STAGGER_MS  500u

typedef enum {
  MOTION_OK = 0,
  MOTION_CLAMPED,       /* accepted, but at least one joint was limited to its min/max
                         * (motion_set_target_exact: NOT accepted, nothing changed) */
  MOTION_REFUSED        /* not accepted (bad joint index, outputs not started) */
} motion_result_t;

void            motion_init(void);
void            motion_tick(void);                 /* TIM6 isr, after input_tick() */

motion_result_t motion_set_target(const joint_vec_t *target);
/* all or nothing (IK callers): if any joint would be clamped, the target is NOT changed and
 * MOTION_CLAMPED is returned, so the arm never starts toward a pose other than the one asked for */
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
