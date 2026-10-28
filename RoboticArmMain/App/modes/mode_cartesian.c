/* mode_cartesian.c - joystick moves X/Y, encoder moves Z, encoder press = gripper (guide 7.1)
 *
 * owner: TODO(team) (works together with the ik member)
 * the target pose is only accepted if ik solves it AND motion takes the solution unchanged;
 * otherwise nothing new is requested, the last accepted pose stays (the arm finishes it) and the lcd
 * shows the reason (OUT OF REACH / JOINT LIMIT / IK NOT READY / MOTION REFUSED).
 * Cartesian-only IK is activated by ARM_CARTESIAN_IK_ENABLED (voice has its own, ARM_VOICE_IK_ENABLED).
 * Entering the mode never commands motion. IK fixes J4 at ARM_J4_ZERO_DEG, so the first accepted
 * move from the installed home (J4 90) rolls J4 to the reference (88, the supervised bench move).
 * If J4 is further than CART_MAX_ROLL_DEG from the reference, the mode stays blocked and shows
 * ALIGN J4 <reference>; align it in a manual mode and re-enter. The mode forces SPEED_FINE on entry
 * (the operator can still toggle FAST). s_pose is the last accepted REQUESTED pose = the motion target.
 *
 * smooth joystick (28 Oct 2026, it felt slow and jerky):
 *   - each XY increment is scaled so no joint has to move faster than the motion speed limit
 *     (trial IK: the biggest joint change vs speed * dt); a fixed mm/s used to outrun the joints
 *   - no new increment while the joints trail the target by more than CART_LEAD_S of motion: the
 *     requested pose can never run far ahead, so the arm stops soon after the stick is let go
 *   - a refused request (IK or exact motion) is just not taken: no hold, the arm smoothly finishes the
 *     last accepted target (at most CART_LEAD_S of motion away) and the next request continues from it.
 *     before, each refusal held the arm and the next request started from the held pose: near a limit
 *     that gave move -> refuse -> stop -> retry stutter. motion.c ramps the speed (APP_RAMP_S).
 *   - the knob (Z, 5 mm per detent) is discrete and can queue several detents ahead of the arm, so it
 *     is not covered by the lead bound. a detent against the Z travel still pending starts from where
 *     the arm is now (FK of the current joints), not from the queued pose: turning back always moves
 *     the arm back. a refused detent holds the arm and rebases on the held pose (an unreachable turn
 *     means "stop here"), the old behaviour for discrete requests.
 */
#include "modes/mode_ops.h"
#include "modes/gripper.h"
#include "motion/motion.h"
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"
#include "common/app_config.h"
#include "common/app_util.h"
#include "lcd/lcd.h"
#include "main.h"
#include <stdio.h>
#if APP_USE_INPUT
#include "input/input.h"
#endif

#define XY_FINE_MM_S 20.0f
#define XY_FAST_MM_S 60.0f
#define Z_STEP_MM    5.0f      /* per encoder detent */
#define CART_MAX_ROLL_DEG 2.1f /* home J4 90 vs reference 88: bench-tested 2 deg + 0.1 margin */
#define CART_LEAD_S  0.15f     /* the target may lead the commanded joints by this much motion at most */

static cart_pose_t s_pose;      /* last accepted requested pose, not the current target */
static bool        s_ready;     /* guard, roll and fk all passed on enter */
static bool        s_roll_blocked; /* enter blocked only because J4 is far from the IK reference */
static bool        s_gripper_closed;
static uint32_t    s_last_ms;
static kin_status_t s_last_status = KIN_NOT_READY;

#if APP_USE_INPUT
static float joint_speed_deg_s(void)
{
  return (motion_get_speed() == SPEED_FAST) ? APP_SPEED_FAST_DEG_S : APP_SPEED_FINE_DEG_S;
}

/* the biggest arm-joint difference (deg) between two joint vectors (J6 gripper not included) */
static float max_joint_diff(const joint_vec_t *a, const joint_vec_t *b)
{
  float m = 0.0f;
  for (int j = 0; j < J6_GRIPPER; j++) {
    float d = absf(a->deg[j] - b->deg[j]);
    if (d > m) m = d;
  }
  return m;
}
#endif

/* takes the request or not: on a refusal nothing changes (the arm finishes the last accepted target) */
static bool try_pose(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  kin_status_t st = kin_ik_cartesian(p, &now, &sol);
  s_last_status = st;
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 1500u);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  /* all or nothing: a target motion would clamp is not taken (s_pose would describe a pose the arm is
   * not going to); motion keeps the last accepted target */
  motion_result_t r = motion_set_target_exact(&sol);
  if (r != MOTION_OK) {
    s_last_status = (r == MOTION_CLAMPED) ? KIN_JOINT_LIMIT : KIN_NOT_READY;
    lcd_show_message(r == MOTION_CLAMPED ? "JOINT LIMIT" : "MOTION REFUSED", 1500u);
    return false;
  }
  return true;
}

static void enter(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  motion_set_speed(SPEED_FINE);                         /* safe default; FAST is still a manual toggle */
  const bool enabled = kin_ik_cartesian_enabled();
  const kin_status_t fk = kin_fk(&now, &s_pose);
  s_last_status = fk;                                   /* never show a stale status from the last visit */
  const bool roll_ok = absf(now.deg[J4_WRIST_ROLL] - ARM_J4_ZERO_DEG) <= CART_MAX_ROLL_DEG;
  s_roll_blocked = enabled && !roll_ok;
  s_ready = enabled && roll_ok && (fk == KIN_OK);        /* FK/roll alone do not unlock moves */
  s_gripper_closed = gripper_is_closed_at(now.deg[J6_GRIPPER]);   /* lower angle = closed */
  s_last_ms = HAL_GetTick();
  if (!s_ready) {
    if (!enabled) {
      lcd_show_message("IK DISABLED", 3000u);
    } else if (s_roll_blocked) {
      char msg[24];
      snprintf(msg, sizeof msg, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
      lcd_show_message(msg, 3000u);
    } else {
      lcd_show_message(kin_status_name(fk), 3000u);
    }
  }
}

static void exit_(void) { motion_hold(); }

/* s_pose = FK of the current commanded joints. false = FK failed: the mode blocks (re-enter) */
static bool rebase_on_current(void)
{
  joint_vec_t now;
  motion_get_current(&now);
  kin_status_t fk = kin_fk(&now, &s_pose);
  if (fk != KIN_OK) {
    s_last_status = fk;
    s_ready = false;
    lcd_show_message(kin_status_name(fk), 3000u);
    return false;
  }
  return true;
}

/* knob detent: Z by Z_STEP_MM per detent (see the header: reversal and refusal rebase) */
static void z_detents(int detents)
{
  float dz = (float)detents * Z_STEP_MM;
  joint_vec_t cj, tj;
  cart_pose_t cp, tp;
  motion_get_current(&cj);
  motion_get_target(&tj);
  if (kin_fk(&cj, &cp) == KIN_OK && kin_fk(&tj, &tp) == KIN_OK) {
    float pending = tp.z_mm - cp.z_mm;              /* Z still to travel to the accepted target */
    if ((dz > 0.0f && pending < -0.5f) || (dz < 0.0f && pending > 0.5f)) {
      if (!rebase_on_current()) return;            /* turning back: from where the arm is */
    }
  }
  cart_pose_t p = s_pose;
  p.z_mm += dz;
  if (try_pose(&p)) {
    s_pose = p;
  } else {
    motion_hold();                                  /* refused discrete request: stop here and rebase */
    (void)rebase_on_current();
  }
}

/* blocked mode: a knob turn / press or a stick push shows WHY nothing moves (on the lcd page too),
 * instead of silently doing nothing. it never moves anything */
static void show_block_reason(void)
{
  char msg[24];
  if (!kin_ik_cartesian_enabled())  snprintf(msg, sizeof msg, "IK DISABLED");
  else if (s_roll_blocked)          snprintf(msg, sizeof msg, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
  else                              snprintf(msg, sizeof msg, "%s", kin_status_name(s_last_status));
  lcd_show_message(msg, 2000u);
}

static void on_event(const app_event_t *e)
{
  if (!s_ready) {
    if (e->type == EVT_ENC_STEP || e->type == EVT_ENC_PRESS) show_block_reason();
    return;
  }
  switch ((evt_type_t)e->type) {
    case EVT_ENC_STEP:
      z_detents(e->v[0]);
      break;
    case EVT_ENC_PRESS:
      if (gripper_command(!s_gripper_closed)) s_gripper_closed = !s_gripper_closed;
      else lcd_show_message("GRIPPER LIMIT", 2000u);         /* state unchanged, J6 held */
      break;
    default:
      break;
  }
}

static void step(void)
{
  uint32_t now = HAL_GetTick();
  float dt = (float)ms_since(now, s_last_ms) * 0.001f;
  s_last_ms = now;
  if (!s_ready) {
#if APP_USE_INPUT
    if (input_joy_x() != 0.0f || input_joy_y() != 0.0f) show_block_reason();
#endif
    return;
  }
  dt = clampf(dt, 0.0f, 0.1f);

#if APP_USE_INPUT
  float jx = input_joy_x(), jy = input_joy_y();
  if (jx == 0.0f && jy == 0.0f) return;
  float v = (motion_get_speed() == SPEED_FAST) ? XY_FAST_MM_S : XY_FINE_MM_S;
  joint_vec_t cur_j, tgt, sol;
  motion_get_current(&cur_j);
  motion_get_target(&tgt);
  /* the joints trail the target by more than CART_LEAD_S of motion: let them catch up first */
  if (max_joint_diff(&tgt, &cur_j) > joint_speed_deg_s() * CART_LEAD_S) return;
  /* operator behind the arm (28 Oct 2026): stick up = forward (+X), stick right = the arm's right (-Y).
   * input_joy_x/y are already the operator's right / up (input.c handles the module mounting) */
  float dx = jy * v * dt, dy = -jx * v * dt;
  cart_pose_t p = s_pose;
  p.x_mm += dx;
  p.y_mm += dy;
  /* scale the step to what the joints can do in dt (near some poses 20 mm/s needs > 20 deg/s) */
  if (kin_ik_cartesian(&p, &cur_j, &sol) == KIN_OK) {
    float d = max_joint_diff(&sol, &tgt), budget = joint_speed_deg_s() * dt;
    if (d > budget && d > 1e-6f) {
      float k = budget / d;
      p = s_pose;
      p.x_mm += dx * k;
      p.y_mm += dy * k;
    }
  }
  if (try_pose(&p)) s_pose = p;
#endif
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  if (!s_ready) {
    if (n > 0) {
      if (!kin_ik_cartesian_enabled()) snprintf(lines[0], LCD_COLS, "IK DISABLED");
      else if (s_roll_blocked) snprintf(lines[0], LCD_COLS, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
      else snprintf(lines[0], LCD_COLS, "%s", kin_status_name(s_last_status));
    }
    return;
  }
  if (n > 0) snprintf(lines[0], LCD_COLS, "X: %d mm", (int)s_pose.x_mm);
  if (n > 1) snprintf(lines[1], LCD_COLS, "Y: %d mm", (int)s_pose.y_mm);
  if (n > 2) snprintf(lines[2], LCD_COLS, "Z: %d mm", (int)s_pose.z_mm);
  if (n > 3) snprintf(lines[3], LCD_COLS, "Gripper: %s", s_gripper_closed ? "CLOSED" : "OPEN");
}

const mode_ops_t MODE_CARTESIAN_OPS = {
  .name = "CARTESIAN",
  .manual = true,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
