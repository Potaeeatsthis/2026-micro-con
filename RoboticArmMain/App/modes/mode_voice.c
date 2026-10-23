/* mode_voice.c - Auto/Voice mode: the S0..S10 task state machine (guide section 11.3)
 *
 * owner: voice (Supanut) + ik member for the motion scripts.
 *
 * words arrive as EVT_VOICE_WORD (class id). ids are looked up BY NAME on enter, so the
 * order of the model's classes does not matter here.
 *
 * multi-step moves (grasp, place, go home, jog queue) are "scripts": a short list of
 * steps run one after another, the next step starts on EVT_MOTION_DONE. a stop, an
 * e-stop or a rail loss aborts the script and goes to S10.
 *
 * still TODO (marked TODO(voice) / TODO(ik)):
 *   - destination poses (hand / station / base) from calibration
 *   - jog direction vectors in the robot-base frame, queue travel limit
 *   - grasp / place heights, approach distance
 *   - "holding for a long time" warning on the lcd
 */
#include "modes/mode_ops.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "kinematics/kinematics.h"
#include "common/app_config.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "lcd/lcd.h"
#include "main.h"
#include <stdio.h>
#if APP_USE_VOICE
#include "voice/voice.h"
#endif
#if APP_USE_PI
#include "pi_link/pi_link.h"
#endif

/* ---------------------------------------------------------------- tunables */
#define APPROACH_MM        50.0f   /* TODO(ik): height above the object before descending */
#define JOG_MAX_ITEMS      10      /* guide: bounded queue */
#define JOG_MAX_TRAVEL_MM  300.0f  /* TODO(voice): validate on the real arm */
#define JOG_DEFAULT_MM     20
#define SCRIPT_MAX         8

typedef enum { DEST_HAND = 0, DEST_STATION, DEST_BASE, DEST_HERE, DEST_COUNT } dest_t;

/* TODO(voice): set during calibration, then set DEST_CALIBRATED to 1 */
#define DEST_CALIBRATED 0
static const cart_pose_t DEST_POSE[3] = {
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* hand */
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* station */
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* base */
};

/* TODO(ik): confirm which way is +x / +y for the person speaking */
typedef enum { JOG_FWD = 0, JOG_BACK, JOG_LEFT, JOG_RIGHT } jog_dir_t;
static const float JOG_VEC[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
static const char *const JOG_NAME[4] = { "FWD", "BACK", "LEFT", "RIGHT" };

/* ---------------------------------------------------------------- states */
typedef enum {
  S0_IDLE = 0, S1_WAIT_OBJECT, S2_CONFIRM_OBJECT, S3_GRASPING, S4_HOLDING, S5_DEST_CHOSEN,
  S6_JOG_QUEUE, S7_CONFIRM_PLACE, S8A_RUN_QUEUE, S8B_PLACING, S9_RELEASE_HOME, S10_STOPPED
} vstate_t;

static const char *const STATE_NAME[] = {
  "S0 IDLE", "S1 WHICH BOX?", "S2 CONFIRM BOX", "S3 GRASPING", "S4 HOLDING", "S5 DEST",
  "S6 JOG QUEUE", "S7 CONFIRM PLACE", "S8 RUN QUEUE", "S8 PLACING", "S9 GOING HOME", "S10 STOPPED"
};

/* word ids, resolved by name */
typedef struct {
  int move, pick, grab, box[OBJ_COUNT], hand, station, base;
  int fwd, back, left, right, undo, place, drop, yes, no, home, stop;
} words_t;

static words_t     W;
static vstate_t    s_state = S0_IDLE;
static object_id_t s_obj = OBJ_NONE;
static bool        s_pi_wait;
static cart_pose_t s_obj_pose;           /* fixed when the user says yes in S2 */
static dest_t      s_dest = DEST_HERE;
static bool        s_holding;            /* gripper closed on an object */
static int         s_jog_step_mm = JOG_DEFAULT_MM;
static struct { jog_dir_t dir; int step_mm; } s_queue[JOG_MAX_ITEMS];
static int         s_queue_n;

/* ---------------------------------------------------------------- scripts */
typedef enum { STEP_POSE, STEP_GRIPPER, STEP_HOME } step_kind_t;
typedef struct { step_kind_t kind; cart_pose_t pose; bool close; } script_step_t;

static script_step_t s_script[SCRIPT_MAX];
static int  s_script_len, s_script_pos;
static bool s_script_running;

static void script_clear(void) { s_script_len = 0; s_script_pos = 0; s_script_running = false; }

static void script_add(step_kind_t k, const cart_pose_t *p, bool close)
{
  if (s_script_len >= SCRIPT_MAX) return;
  script_step_t *s = &s_script[s_script_len++];
  s->kind = k;
  if (p) s->pose = *p;
  s->close = close;
}

static bool pose_to_target(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  kin_status_t st = kin_ik(p, &now, &sol);
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 3000u);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  return motion_set_target(&sol) != MOTION_REFUSED;
}

static bool script_run_current(void)
{
  const script_step_t *s = &s_script[s_script_pos];
  switch (s->kind) {
    case STEP_POSE:
      return pose_to_target(&s->pose);
    case STEP_GRIPPER:
      return motion_set_joint_target(J6_GRIPPER, s->close ? APP_GRIPPER_CLOSED_DEG : APP_GRIPPER_OPEN_DEG)
             != MOTION_REFUSED;
    case STEP_HOME: {
      joint_vec_t home, now;
      motion_get_current(&now);
      for (int j = 0; j < ARM_N_JOINTS; j++) home.deg[j] = servo_cal(j)->home_deg;
      home.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];          /* home never opens the gripper itself */
      return motion_set_target(&home) != MOTION_REFUSED;
    }
    default:
      return false;
  }
}

/* every pose of the script must be solvable BEFORE the first step runs, so a script
 * never starts (e.g. opens the gripper) and then gets stuck halfway */
static bool script_check(void)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  for (int i = 0; i < s_script_len; i++) {
    if (s_script[i].kind != STEP_POSE) continue;
    kin_status_t st = kin_ik(&s_script[i].pose, &now, &sol);
    if (st != KIN_OK) {
      lcd_show_message(kin_status_name(st), 3000u);
      return false;
    }
  }
  return true;
}

static bool script_start(void)
{
  if (s_script_len == 0 || !script_check()) {
    script_clear();
    return false;
  }
  s_script_pos = 0;
  s_script_running = script_run_current();
  return s_script_running;
}

typedef enum { SCRIPT_BUSY = 0, SCRIPT_FINISHED, SCRIPT_FAILED, SCRIPT_IDLE } script_result_t;

static script_result_t script_on_motion_done(void)
{
  if (!s_script_running) return SCRIPT_IDLE;
  if (++s_script_pos >= s_script_len) {
    s_script_running = false;
    return SCRIPT_FINISHED;
  }
  if (!script_run_current()) {
    s_script_running = false;
    return SCRIPT_FAILED;
  }
  return SCRIPT_BUSY;
}

/* ---------------------------------------------------------------- helpers */
static void set_state(vstate_t s)
{
  if (s != s_state) LOG("voice-mode: %s -> %s\r\n", STATE_NAME[s_state], STATE_NAME[s]);
  s_state = s;
}

static bool is_moving_state(vstate_t s)
{
  return s == S3_GRASPING || s == S8A_RUN_QUEUE || s == S8B_PLACING || s == S9_RELEASE_HOME;
}

static bool current_pose(cart_pose_t *out)
{
  joint_vec_t now;
  motion_get_current(&now);
  return kin_fk(&now, out) == KIN_OK;
}

static void cancel_pi(void)
{
#if APP_USE_PI
  pi_link_cancel();
#endif
  s_pi_wait = false;
}

static void controlled_stop(void)
{
  motion_hold();
  script_clear();
  cancel_pi();
  set_state(S10_STOPPED);
}

static void go_home_then_idle(bool open_first)
{
  script_clear();
  if (open_first) script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_HOME, NULL, false);
  if (script_start()) {
    if (open_first) s_holding = false;
    set_state(S9_RELEASE_HOME);
  }
}

/* ---------------------------------------------------------------- per-state word handling */
static int word_to_object(int w)
{
  for (int i = 0; i < (int)OBJ_COUNT; i++) if (w == W.box[i]) return i;
  return -1;
}

static void start_grasp(void)
{
  cart_pose_t above = s_obj_pose;
  above.z_mm += APPROACH_MM;
  script_clear();
  script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_POSE, &above, false);
  script_add(STEP_POSE, &s_obj_pose, false);
  script_add(STEP_GRIPPER, NULL, true);
  script_add(STEP_POSE, &above, false);
  if (script_start()) set_state(S3_GRASPING);
  else set_state(S2_CONFIRM_OBJECT);
}

static void start_place(void)
{
  cart_pose_t target;
  if (s_dest == DEST_HERE) {
    if (!current_pose(&target)) { lcd_show_message("IK NOT READY", 3000u); return; }
    target.z_mm -= APPROACH_MM;                     /* TODO(ik): lower to the table, not a fixed amount */
  } else {
    if (!DEST_CALIBRATED) { lcd_show_message("DEST NOT SET", 3000u); return; }
    target = DEST_POSE[s_dest];
  }
  cart_pose_t above = target;
  above.z_mm += APPROACH_MM;
  script_clear();
  script_add(STEP_POSE, &above, false);
  script_add(STEP_POSE, &target, false);
  script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_POSE, &above, false);
  if (script_start()) set_state(S8B_PLACING);
}

static void run_queue(void)
{
  cart_pose_t p;
  if (!current_pose(&p)) { lcd_show_message("IK NOT READY", 3000u); return; }
  joint_vec_t now, sol;
  motion_get_current(&now);
  float travel = 0.0f;
  script_clear();
  for (int i = 0; i < s_queue_n; i++) {
    p.x_mm += JOG_VEC[s_queue[i].dir][0] * (float)s_queue[i].step_mm;
    p.y_mm += JOG_VEC[s_queue[i].dir][1] * (float)s_queue[i].step_mm;
    travel += (float)s_queue[i].step_mm;
    if (kin_ik(&p, &now, &sol) != KIN_OK) {          /* whole path checked before moving */
      lcd_show_message("OUT OF REACH", 3000u);
      script_clear();
      return;
    }
    script_add(STEP_POSE, &p, false);
  }
  if (travel > JOG_MAX_TRAVEL_MM) { lcd_show_message("QUEUE TOO LONG", 3000u); script_clear(); return; }
  if (script_start()) set_state(S8A_RUN_QUEUE);
}

static void on_word(int w)
{
  if (w == W.stop) {
    if (is_moving_state(s_state)) { controlled_stop(); return; }
    /* waiting state: cancel the pending choice */
    switch (s_state) {
      case S1_WAIT_OBJECT: case S2_CONFIRM_OBJECT: cancel_pi(); set_state(S0_IDLE); break;
      case S5_DEST_CHOSEN: case S7_CONFIRM_PLACE:  set_state(S4_HOLDING); break;
      case S6_JOG_QUEUE:   s_queue_n = 0;          set_state(S4_HOLDING); break;
      default: break;
    }
    return;
  }

  int obj;
  switch (s_state) {
    case S0_IDLE:
      if (w == W.move || w == W.pick || w == W.grab) set_state(S1_WAIT_OBJECT);
      else if (w == W.home) go_home_then_idle(false);
      break;

    case S1_WAIT_OBJECT:
      obj = word_to_object(w);
      if (obj >= 0 && !s_pi_wait) {
#if APP_USE_PI
        if (pi_link_find((object_id_t)obj)) { s_obj = (object_id_t)obj; s_pi_wait = true; }
        else lcd_show_message("PI BUSY", 2000u);
#else
        lcd_show_message("PI DISABLED", 2000u);
#endif
      }
      break;

    case S2_CONFIRM_OBJECT:
      if (w == W.yes) start_grasp();                 /* s_obj_pose is fixed from here on */
      else if (w == W.no) set_state(S1_WAIT_OBJECT);
      break;

    case S4_HOLDING:
      if (w == W.hand)         { s_dest = DEST_HAND;    set_state(S7_CONFIRM_PLACE); }
      else if (w == W.station) { s_dest = DEST_STATION; set_state(S7_CONFIRM_PLACE); }
      else if (w == W.base)    { s_dest = DEST_BASE;    set_state(S7_CONFIRM_PLACE); }
      else if (w == W.place || w == W.drop) { s_dest = DEST_HERE; set_state(S7_CONFIRM_PLACE); }
      else if (w == W.fwd || w == W.back || w == W.left || w == W.right) {
        s_queue_n = 0;
        set_state(S6_JOG_QUEUE);
        on_word(w);                                  /* the first jog word also goes in the queue */
      }
      else if (w == W.home && s_holding) lcd_show_message("RELEASE FIRST", 3000u);
      break;

    case S6_JOG_QUEUE: {
      int d = (w == W.fwd) ? JOG_FWD : (w == W.back) ? JOG_BACK : (w == W.left) ? JOG_LEFT
            : (w == W.right) ? JOG_RIGHT : -1;
      if (d >= 0) {
        if (s_queue_n < JOG_MAX_ITEMS) {
          s_queue[s_queue_n].dir = (jog_dir_t)d;
          s_queue[s_queue_n].step_mm = s_jog_step_mm;
          s_queue_n++;
        } else {
          lcd_show_message("QUEUE FULL", 2000u);
        }
      } else if (w == W.undo) {
        if (s_queue_n > 0) s_queue_n--;
      } else if (w == W.yes) {
        if (s_queue_n > 0) run_queue();
      } else if (w == W.no) {
        s_queue_n = 0;
        set_state(S4_HOLDING);
      }
      break;
    }

    case S7_CONFIRM_PLACE:
      if (w == W.yes) start_place();
      else if (w == W.no) set_state(S4_HOLDING);
      break;

    case S10_STOPPED:
      if (w == W.place || w == W.drop) { s_dest = DEST_HERE; start_place(); }
      else if (w == W.home) go_home_then_idle(true);  /* guide: home from S10 opens the gripper */
      break;

    default:
      break;                                          /* a word not valid here does nothing */
  }
}

/* ---------------------------------------------------------------- mode ops */
static int id(const char *name)
{
#if APP_USE_VOICE
  return voice_class_id(name);
#else
  (void)name;
  return -2;                                          /* never matches an event */
#endif
}

static void enter(void)
{
  W.move = id("move"); W.pick = id("pick"); W.grab = id("grab");
  W.box[OBJ_BLUE_BOX] = id("blue box"); W.box[OBJ_GREEN_BOX] = id("green box");
  W.box[OBJ_RED_BOX] = id("red box");   W.box[OBJ_BLACK_BOX] = id("black box");
  W.hand = id("hand"); W.station = id("station"); W.base = id("base");
  W.fwd = id("forward"); W.back = id("backward"); W.left = id("left"); W.right = id("right");
  W.undo = id("undo"); W.place = id("place"); W.drop = id("drop");
  W.yes = id("yes"); W.no = id("no"); W.home = id("home"); W.stop = id("stop");
  /* entering never resumes an old task, but a held object stays held */
  script_clear();
  set_state(s_holding ? S4_HOLDING : S0_IDLE);
}

static void exit_(void)
{
  motion_hold();
  script_clear();
  cancel_pi();
  s_queue_n = 0;
}

static void on_event(const app_event_t *e)
{
  switch ((evt_type_t)e->type) {
    case EVT_VOICE_WORD:
      on_word((int)e->arg);
      break;

    case EVT_TOUCH_STOP:
      on_word(W.stop);
      break;

    case EVT_TOUCH_UNDO:
      if (s_state == S6_JOG_QUEUE) on_word(W.undo);
      break;

    case EVT_TOUCH_STEP_CM:
      s_jog_step_mm = (int)clampi(e->v[0], 5, 100);
      break;

    case EVT_PI_OBJECT:
      if (s_state == S1_WAIT_OBJECT && s_pi_wait && (object_id_t)e->arg == s_obj) {
        s_pi_wait = false;
        s_obj_pose.x_mm = (float)e->v[0];
        s_obj_pose.y_mm = (float)e->v[1];
        s_obj_pose.z_mm = (float)e->v[2];
        s_obj_pose.pitch_deg = 0.0f;                  /* TODO(ik): grasp orientation */
        set_state(S2_CONFIRM_OBJECT);
      }
      break;

    case EVT_PI_NOTFOUND:
      if (s_pi_wait) { s_pi_wait = false; lcd_show_message("BOX NOT FOUND", 3000u); }
      break;

    case EVT_PI_TIMEOUT:
      if (s_pi_wait) { s_pi_wait = false; lcd_show_message("PI NOT RESPONDING", 3000u); set_state(S0_IDLE); }
      break;

    case EVT_MOTION_DONE: {
      script_result_t r = script_on_motion_done();
      if (r == SCRIPT_FAILED) {                       /* e.g. the pose changed: stop safely, wait for the user */
        lcd_show_message("MOVE FAILED", 3000u);
        controlled_stop();
        break;
      }
      if (r != SCRIPT_FINISHED) break;
      switch (s_state) {
        case S3_GRASPING:     s_holding = true;  set_state(S4_HOLDING); break;
        case S8A_RUN_QUEUE:   s_queue_n = 0;     set_state(S4_HOLDING); break;
        case S8B_PLACING:     s_holding = false; go_home_then_idle(false); break;
        case S9_RELEASE_HOME: set_state(S0_IDLE); break;
        default: break;
      }
      break;
    }

    case EVT_FAULT:
      if (e->v[0] == 0) break;                        /* only fatal faults stop the task */
      /* fall through */
    case EVT_ESTOP_PRESSED:
    case EVT_RAIL_LOST:
      if (is_moving_state(s_state) || s_script_running) controlled_stop();
      cancel_pi();
      break;

    default:
      break;
  }
}

static void step(void)
{
  /* TODO(voice): warn on the lcd when holding for a long time without a command (never auto-release) */
}

static void describe(char (*lines)[LCD_COLS], int n)
{
  if (n > 0) snprintf(lines[0], LCD_COLS, "%s", STATE_NAME[s_state]);
  if (n > 1) {
    if (s_state == S1_WAIT_OBJECT && s_pi_wait) {
#if APP_USE_PI
      snprintf(lines[1], LCD_COLS, "Searching %s", pi_object_name(s_obj));
#endif
    } else if (s_state == S2_CONFIRM_OBJECT) {
      snprintf(lines[1], LCD_COLS, "%d %d %d mm", (int)s_obj_pose.x_mm, (int)s_obj_pose.y_mm, (int)s_obj_pose.z_mm);
    } else if (s_state == S6_JOG_QUEUE) {
      snprintf(lines[1], LCD_COLS, "Queue %d/%d step %d mm", s_queue_n, JOG_MAX_ITEMS, s_jog_step_mm);
    } else {
      snprintf(lines[1], LCD_COLS, "Gripper: %s", s_holding ? "HOLDING" : "EMPTY");
    }
  }
  if (n > 2 && s_state == S6_JOG_QUEUE && s_queue_n > 0) {
    snprintf(lines[2], LCD_COLS, "last: %s %d", JOG_NAME[s_queue[s_queue_n - 1].dir], s_queue[s_queue_n - 1].step_mm);
  }
  if (n > 3 && (s_state == S2_CONFIRM_OBJECT || s_state == S6_JOG_QUEUE || s_state == S7_CONFIRM_PLACE)) {
    snprintf(lines[3], LCD_COLS, "say YES or NO");
  }
}

const mode_ops_t MODE_VOICE_OPS = {
  .name = "VOICE",
  .requires_enable = false,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
