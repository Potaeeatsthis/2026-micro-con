/* mode_joint_group.c - joystick moves two joints, encoder picks the group (guide 7.2)
 *
 * owner: TODO(team) - write the owner's name here and in ProjectDevGuide/docs/TEAM_GUIDE.md section 1
 *   stick left-right (X) / up-down (Y), in the operator's view (input.c turns the module's axes):
 *   group 1: X = J1 base,       Y = J2 shoulder
 *   group 2: X = J4 wrist roll, Y = J3 elbow
 *   group 3: X = J6 gripper,    Y = J5 wrist pitch
 *   directions per axis in GROUP_SIGN, set on the real arm with the operator 28 Oct 2026
 *   encoder turn (or the lcd < / > arrows, EVT_TOUCH_SELECT) -> next / previous group
 */
#include "modes/mode_ops.h"
#include "modes/modes.h"
#include "motion/motion.h"
#include <stdio.h>

#define N_GROUPS 3

/* {joint on the stick's X (left-right), joint on its Y (up-down)} for each group */
static const int GROUP_AXES[N_GROUPS][2] = {
  { J1_BASE,       J2_SHOULDER },
  { J4_WRIST_ROLL, J3_ELBOW },
  { J6_GRIPPER,    J5_WRIST_PITCH },
};

/* {X direction, Y direction}: +1 = stick right / up raises the joint angle, -1 = lowers it */
static const float GROUP_SIGN[N_GROUPS][2] = {
  { -1.0f,  1.0f },   /* right: J1 down,  up: J2 up */
  { -1.0f, -1.0f },   /* right: J4 down,  up: J3 down */
  { -1.0f,  1.0f },   /* right: J6 down (closes), up: J5 up */
};

static int s_group;

int mode_joint_group_selected(void) { return s_group; }

static void enter(void)  { }
static void exit_(void)  { motion_hold(); }

static void on_event(const app_event_t *e)
{
  if (e->type == EVT_ENC_STEP || e->type == EVT_TOUCH_SELECT) {
    int g = s_group + (int)e->v[0];
    while (g < 0) g += N_GROUPS;
    s_group = g % N_GROUPS;
    motion_hold();                                  /* the old pair stops before the new pair moves */
  }
}

static void step(void)
{
  manual_jog_dir(GROUP_AXES[s_group][0], GROUP_SIGN[s_group][0], GROUP_AXES[s_group][1], GROUP_SIGN[s_group][1]);
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  joint_vec_t t;
  motion_get_target(&t);
  int jx = GROUP_AXES[s_group][0], jy = GROUP_AXES[s_group][1];
  if (n > 0) snprintf(lines[0], LCD_COLS, "Group %d", s_group + 1);
  if (n > 1) snprintf(lines[1], LCD_COLS, "X %s %d", manual_joint_name(jx), (int)t.deg[jx]);
  if (n > 2) snprintf(lines[2], LCD_COLS, "Y %s %d", manual_joint_name(jy), (int)t.deg[jy]);
}

const mode_ops_t MODE_JOINT_GROUP_OPS = {
  .name = "JOINT GROUP",
  .manual = true,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
