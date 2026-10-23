/* mode_joint_group.c - joystick moves two joints, encoder picks the group (guide 7.2)
 *
 * owner: TODO(team) - write the owner's name here and in ProjectDevGuide/docs/TEAM_GUIDE.md section 1
 *   group 1: X = J1 base,      Y = J2 shoulder
 *   group 2: X = J3 elbow,     Y = J4 wrist pitch
 *   group 3: X = J5 wrist rot, Y = J6 gripper
 */
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "safety/safety.h"
#include <stdio.h>

#define N_GROUPS 3

static int s_group;

static void enter(void)  { }
static void exit_(void)  { motion_hold(); }

static void on_event(const app_event_t *e)
{
  if (e->type == EVT_ENC_STEP) {
    int g = s_group + (int)e->v[0];
    while (g < 0) g += N_GROUPS;
    s_group = g % N_GROUPS;
    motion_hold();                                  /* the old pair stops before the new pair moves */
  }
}

static void step(void)
{
  manual_jog(2 * s_group, 2 * s_group + 1);
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  joint_vec_t t;
  motion_get_target(&t);
  int jx = 2 * s_group, jy = jx + 1;
  if (n > 0) snprintf(lines[0], LCD_COLS, "Group %d", s_group + 1);
  if (n > 1) snprintf(lines[1], LCD_COLS, "X %s %d", manual_joint_name(jx), (int)t.deg[jx]);
  if (n > 2) snprintf(lines[2], LCD_COLS, "Y %s %d", manual_joint_name(jy), (int)t.deg[jy]);
  if (n > 3) snprintf(lines[3], LCD_COLS, "Enable: %s", safety_enable_held() ? "HELD" : "RELEASED");
}

const mode_ops_t MODE_JOINT_GROUP_OPS = {
  .name = "JOINT GROUP",
  .requires_enable = true,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
