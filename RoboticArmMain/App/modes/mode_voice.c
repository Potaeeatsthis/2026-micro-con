/* mode_voice.c - voice mode: the S0..S11 task state machine (guide 11.3)
 *
 * Word ids are looked up by name on enter, so the model's class order does not matter.
 * Each state tells voice/ which words it accepts (voice_set_accept_mask); "stop" works everywhere.
 * Nothing moves without "yes" (S2 grasp, S6 jog queue, S7 place, S11 home), and the arm never
 * lets go of a held box on its own.
 *
 * hand / station: "yes" sends FIND to the pi and the reply is the target. base is a fixed spot.
 *
 * Grasp, place, home and the jog queue are scripts: a list of steps, the next one starts on
 * EVT_MOTION_DONE. Every pose is solved before step 1, so a script never stops halfway.
 * A stop or a fatal fault aborts it and goes to S10.
 *
 * Bench flags (app_config_local.h): APP_VOICE_DRY_RUN logs each step instead of moving,
 * APP_VOICE_FAKE_PI answers a box word with a fake position.
 */
#include "modes/mode_ops.h"
#include "modes/gripper.h"
#include "motion/motion.h"
#include "motion/servo.h"
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
#define APPROACH_MM        50.0f   /* height above the grasp / release point */
/* shorter approach if APPROACH_MM does not fit */
#define APPROACH_MIN_MM    30.0f
/* extra lift after closing, so the box is carried clear of the table and station */
#define HOLD_EXTRA_MM      20.0f
/* the pi gives the top of the box; grasp this far below it */
#define GRASP_BELOW_TOP_MM  10.5f
_Static_assert((int)(GRASP_BELOW_TOP_MM * 10.0f) > 0, "GRASP_BELOW_TOP_MM > 0");
/* how high above the palm the gripper opens */
#define HAND_RELEASE_MM    30.0f
_Static_assert((int)HAND_RELEASE_MM > 0 && (int)HAND_RELEASE_MM < (int)APPROACH_MM, "0 < HAND_RELEASE_MM < APPROACH_MM");
/* how high above the station top the gripper opens */
#define STATION_RELEASE_MM 25.0f
_Static_assert((int)STATION_RELEASE_MM > 0 && (int)STATION_RELEASE_MM < (int)APPROACH_MM,
               "0 < STATION_RELEASE_MM < APPROACH_MM");
/* grasp / place pitch: 0 = pointing up, 180 = straight down. try 180 first, then tilt down to the min */
#define GRASP_PITCH_PREF_DEG 180.0f
#define GRASP_PITCH_MIN_DEG  110.0f
#define GRASP_PITCH_STEP_DEG 10.0f
#define JOG_MAX_ITEMS      10
#define JOG_MAX_TRAVEL_MM  300.0f
#define JOG_DEFAULT_MM     20
#define JOG_STEP_MAX_01MM  200
#define SCRIPT_MAX         12

/* dry run / fake pi only, not measured */
#if APP_VOICE_DRY_RUN
static const cart_pose_t DRY_START_POSE = { 0.0f, 0.0f, 200.0f, 0.0f };
#endif
#if APP_VOICE_FAKE_PI
static const int32_t FAKE_OBJ_MM[3] = { 182, -45, 30 };
#endif

/* hand and station come from the pi */
typedef enum { DEST_HAND = 0, DEST_STATION, DEST_BASE, DEST_HERE, DEST_COUNT } dest_t;
static const char *const DEST_NAME[DEST_COUNT] = { "HAND", "STATION", "BASE", "HERE" };

/* base drop spot on the table, z = held box centre */
#define DEST_CALIBRATED 1
#define BASE_X_MM      220.0f
#define BASE_Y_MM      120.0f
#define BASE_Z_MM      12.0f
static const cart_pose_t DEST_POSE[3] = {
  { 0.0f, 0.0f, 0.0f, 0.0f },
  { 0.0f, 0.0f, 0.0f, 0.0f },
  { BASE_X_MM, BASE_Y_MM, BASE_Z_MM, GRASP_PITCH_PREF_DEG },
};

/* jog words, seen from behind the arm: forward = +X, left = +Y */
typedef enum { JOG_FWD = 0, JOG_BACK, JOG_LEFT, JOG_RIGHT } jog_dir_t;
static const float JOG_VEC[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
static const char *const JOG_NAME[4] = { "FWD", "BACK", "LEFT", "RIGHT" };
static const char *const JOG_WORD[4] = { "forward", "backward", "left", "right" };

/* ---------------------------------------------------------------- states */
typedef enum {
  S0_IDLE = 0, S1_WAIT_OBJECT, S2_CONFIRM_OBJECT, S3_GRASPING, S4_HOLDING,
  S6_JOG_QUEUE, S7_CONFIRM_PLACE, S8A_RUN_QUEUE, S8B_PLACING, S9_RELEASE_HOME, S10_STOPPED,
  S11_CONFIRM_HOME
} vstate_t;

static const char *const STATE_NAME[] = {
  "S0 IDLE", "S1 WHICH BOX?", "S2 CONFIRM BOX", "S3 GRASPING", "S4 HOLDING",
  "S6 JOG QUEUE", "S7 CONFIRM PLACE", "S8 RUN QUEUE", "S8 PLACING", "S9 GOING HOME", "S10 STOPPED",
  "S11 CONFIRM HOME"
};

typedef struct {
  int move, pick, grab, box[OBJ_BOX_COUNT], hand, station, base;
  int fwd, back, left, right, undo, place, drop, yes, no, home, stop;
} words_t;

static words_t     W;
static vstate_t    s_state = S0_IDLE;
static vstate_t    s_return = S0_IDLE;
static bool        s_home_open;
static object_id_t s_obj = OBJ_NONE;
static bool        s_pi_wait;
static cart_pose_t s_obj_pose;
static dest_t      s_dest = DEST_HERE;
static bool        s_holding;            /* gripper closed on an object */
static int         s_jog_step_01 = JOG_DEFAULT_MM * 10;   /* 0.1 mm */
static struct { jog_dir_t dir; int step_01; } s_queue[JOG_MAX_ITEMS];
static int         s_queue_n;

/* ---------------------------------------------------------------- scripts */
/* STEP_HOME_RAISE: lift J2 / J5 to the coupling-safe pose first, then the next pose is reachable */
typedef enum { STEP_POSE, STEP_GRIPPER, STEP_HOME_RAISE, STEP_HOME } step_kind_t;
typedef struct { step_kind_t kind; cart_pose_t pose; bool close; } script_step_t;

static script_step_t s_script[SCRIPT_MAX];
static int  s_script_len, s_script_pos;
static bool s_script_running;
#if APP_VOICE_DRY_RUN
static cart_pose_t s_dry_pose = { 0.0f, 0.0f, 200.0f, 0.0f };
static uint32_t    s_dry_step_ms;
#endif

static void script_clear(void) { s_script_len = 0; s_script_pos = 0; s_script_running = false; }
static void set_state(vstate_t s);

#if APP_VOICE_DRY_RUN
/* dry run: log the step, step() marks it done later */
static bool dry_run_step(const script_step_t *s)
{
  switch (s->kind) {
    case STEP_POSE:
      s_dry_pose = s->pose;
      LOG("dry-run: step %d/%d move to %d %d %d mm\r\n", s_script_pos + 1, s_script_len,
          (int)s->pose.x_mm, (int)s->pose.y_mm, (int)s->pose.z_mm);
      break;
    case STEP_GRIPPER:
      LOG("dry-run: step %d/%d gripper %s\r\n", s_script_pos + 1, s_script_len, s->close ? "CLOSE" : "OPEN");
      break;
    case STEP_HOME_RAISE:
      LOG("dry-run: step %d/%d raise shoulder / wrist (coupling-safe pose)\r\n", s_script_pos + 1, s_script_len);
      break;
    case STEP_HOME:
      s_dry_pose = DRY_START_POSE;
      LOG("dry-run: step %d/%d go home\r\n", s_script_pos + 1, s_script_len);
      break;
    default:
      return false;
  }
  s_dry_step_ms = HAL_GetTick();
  return true;
}
#endif

static void script_add(step_kind_t k, const cart_pose_t *p, bool close)
{
  if (s_script_len >= SCRIPT_MAX) return;
  script_step_t *s = &s_script[s_script_len++];
  s->kind = k;
  if (p) s->pose = *p;
  s->close = close;
}

/* home pose, gripper unchanged */
static void home_target(joint_vec_t *out)
{
  joint_vec_t now;
  motion_get_current(&now);
  for (int j = 0; j < ARM_N_JOINTS; j++) out->deg[j] = servo_cal(j)->home_deg;
  out->deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
}

static bool at_home(void)
{
  joint_vec_t now, home;
  motion_get_current(&now);
  home_target(&home);
  for (int j = 0; j < ARM_N_JOINTS; j++) if (absf(now.deg[j] - home.deg[j]) > 0.01f) return false;
  return true;
}

/* a clamped or refused target fails the step, it never moves somewhere else */
static bool target_exact(const joint_vec_t *t)
{
  motion_result_t r = motion_set_target_exact(t);
  if (r == MOTION_OK) return true;
  motion_hold();
  lcd_show_message(r == MOTION_CLAMPED ? "JOINT LIMIT" : "MOTION REFUSED", 3000u);
  return false;
}

static bool pose_to_target(const cart_pose_t *p)
{
  joint_vec_t now, sol;
  motion_get_current(&now);
  kin_status_t st = kin_ik_voice(p, &now, &sol);
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 3000u);
    return false;
  }
  sol.deg[J6_GRIPPER] = now.deg[J6_GRIPPER];
  return target_exact(&sol);
}

static bool run_step(const script_step_t *s)
{
  switch (s->kind) {
    case STEP_POSE:
      return pose_to_target(&s->pose);
    case STEP_GRIPPER:
      if (gripper_command(s->close)) return true;
      lcd_show_message("GRIPPER LIMIT", 3000u);
      return false;
    case STEP_HOME_RAISE: {
      joint_vec_t t;
      motion_get_current(&t);
      if (t.deg[J2_SHOULDER] < MOTION_J2_COUPLED_DEG) t.deg[J2_SHOULDER] = MOTION_J2_COUPLED_DEG;
      if (t.deg[J5_WRIST_PITCH] < MOTION_J5_WRIST_UP_DEG) t.deg[J5_WRIST_PITCH] = MOTION_J5_WRIST_UP_DEG;
      return target_exact(&t);
    }
    case STEP_HOME: {
      joint_vec_t home;
      home_target(&home);
      if (motion_set_target_exact(&home) == MOTION_OK) return true;
      lcd_show_message("HOME BLOCKED", 3000u);
      motion_hold();
      return false;
    }
    default:
      return false;
  }
}

/* s_holding follows the gripper step, so a stop mid-place knows if the box is already released */
static bool script_run_current(void)
{
  const script_step_t *s = &s_script[s_script_pos];
#if APP_VOICE_DRY_RUN
  bool ok = dry_run_step(s);
  (void)run_step;
#else
  bool ok = run_step(s);
#endif
  if (ok && s->kind == STEP_GRIPPER) s_holding = s->close;
  return ok;
}

/* solve every pose before running. each pose is solved from the previous one; if the J2/J5
 * coupling blocks it, a raise step goes in front */
static script_step_t s_base[SCRIPT_MAX];
static int           s_base_len;

static void coupling_safe(joint_vec_t *j)
{
  if (j->deg[J2_SHOULDER] < MOTION_J2_COUPLED_DEG) j->deg[J2_SHOULDER] = MOTION_J2_COUPLED_DEG;
  if (j->deg[J5_WRIST_PITCH] < MOTION_J5_WRIST_UP_DEG) j->deg[J5_WRIST_PITCH] = MOTION_J5_WRIST_UP_DEG;
}

static kin_status_t script_plan(bool set_pitch, float pitch_deg)
{
  script_step_t out[SCRIPT_MAX];
  int n = 0;
  joint_vec_t seed, sol;
  motion_get_current(&seed);
  for (int i = 0; i < s_base_len; i++) {
    script_step_t st = s_base[i];
    if (st.kind == STEP_POSE) {
      if (set_pitch) st.pose.pitch_deg = pitch_deg;
      kin_status_t r = kin_ik_voice(&st.pose, &seed, &sol);
      if (r == KIN_JOINT_LIMIT) {
        joint_vec_t v = seed;
        coupling_safe(&v);
        bool raised = (v.deg[J2_SHOULDER] != seed.deg[J2_SHOULDER]) || (v.deg[J5_WRIST_PITCH] != seed.deg[J5_WRIST_PITCH]);
        if (raised && n < SCRIPT_MAX && kin_ik_voice(&st.pose, &v, &sol) == KIN_OK) {
          out[n].kind = STEP_HOME_RAISE;
          out[n].close = false;
          out[n].pose = st.pose;
          n++;
          r = KIN_OK;
        }
      }
      if (r != KIN_OK) return r;
      seed = sol;
    }
    if (n >= SCRIPT_MAX) return KIN_OUT_OF_REACH;
    out[n++] = st;
  }
  for (int i = 0; i < n; i++) s_script[i] = out[i];
  s_script_len = n;
  return KIN_OK;
}

/* try pitch 180 first, then tilt 10 deg at a time. one pitch for the whole script */
static kin_status_t script_check(bool search_pitch)
{
  for (int i = 0; i < s_script_len; i++) s_base[i] = s_script[i];
  s_base_len = s_script_len;
#if APP_VOICE_DRY_RUN
  if (search_pitch) {
    for (int i = 0; i < s_script_len; i++) {
      if (s_script[i].kind == STEP_POSE) s_script[i].pose.pitch_deg = GRASP_PITCH_PREF_DEG;
    }
  }
  return KIN_OK;
#endif
  if (!search_pitch) return script_plan(false, 0.0f);
  kin_status_t st = KIN_OUT_OF_REACH;
  const int n = (int)((GRASP_PITCH_PREF_DEG - GRASP_PITCH_MIN_DEG) / GRASP_PITCH_STEP_DEG) + 1;
  for (int k = 0; k < n; k++) {
    float pitch = GRASP_PITCH_PREF_DEG - (float)k * GRASP_PITCH_STEP_DEG;
    st = script_plan(true, pitch);
    if (st == KIN_OK) {
      LOG("voice-mode: pitch %d deg, %d steps\r\n", (int)pitch, s_script_len);
      return KIN_OK;
    }
    if (st == KIN_NOT_READY) break;
  }
  for (int i = 0; i < s_base_len; i++) s_script[i] = s_base[i];
  s_script_len = s_base_len;
  return st;
}

/* plan the script and start step 1, or go back on failure */
static bool script_start(vstate_t moving, bool search_pitch)
{
  kin_status_t st = (s_script_len == 0) ? KIN_OUT_OF_REACH : script_check(search_pitch);
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 3000u);
    script_clear();
    return false;
  }
  vstate_t before = s_state;
  set_state(moving);
  s_script_pos = 0;
  s_script_running = script_run_current();
  if (!s_script_running) set_state(before);
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

/* ---------------------------------------------------------------- lcd voice page
 * S3 / S8 -> PROCESSING, S9 after a place -> COMPLETE, S10 -> STOPPED, waiting states -> nothing */
static bool s_lcd_complete;

static int lcd_box_of(object_id_t o)
{
  switch (o) {
    case OBJ_RED_BOX:   return LCD_BOX_RED;
    case OBJ_GREEN_BOX: return LCD_BOX_GREEN;
    case OBJ_BLACK_BOX: return LCD_BOX_BLACK;
    case OBJ_BLUE_BOX:  return LCD_BOX_BLUE;
    default:            return -1;
  }
}

/* jog queue pop-up, e.g. "[forward 1.25, right 1.10]" while S6 has entries */
static void lcd_queue_sync(void)
{
#if APP_USE_LCD
  char t[160];
  size_t n = 0;
  t[0] = '\0';
  if (s_state == S6_JOG_QUEUE && s_queue_n > 0) {
    n += (size_t)snprintf(&t[n], sizeof t - n, "[");
    for (int i = 0; i < s_queue_n && n < sizeof t; i++) {
      n += (size_t)snprintf(&t[n], sizeof t - n, "%s%s %d.%02d", i ? ", " : "", JOG_WORD[s_queue[i].dir],
                            s_queue[i].step_01 / 100, s_queue[i].step_01 % 100);
    }
    if (n < sizeof t) (void)snprintf(&t[n], sizeof t - n, "]");
  }
  lcd_voice_set_queue(t);
#endif
}

static void lcd_sync(void)
{
#if APP_USE_LCD
  lcd_queue_sync();
  switch (s_state) {
    case S3_GRASPING: case S8A_RUN_QUEUE: case S8B_PLACING:
      lcd_show_status(LCD_STATUS_PROCESSING);
      break;
    case S9_RELEASE_HOME:
      lcd_show_status(s_lcd_complete ? LCD_STATUS_COMPLETE : LCD_STATUS_PROCESSING);
      break;
    case S10_STOPPED:
      lcd_show_status(s_holding ? LCD_STATUS_STOPPED_HOLDING : LCD_STATUS_STOPPED_EMPTY);
      break;
    default:
      lcd_hide_status();
      break;
  }
#endif
}

/* ---------------------------------------------------------------- helpers */
static void set_state(vstate_t s)
{
  if (s == s_state) return;
  LOG("voice-mode: %s -> %s\r\n", STATE_NAME[s_state], STATE_NAME[s]);
  if (s_state == S9_RELEASE_HOME) s_lcd_complete = false;
  s_state = s;
  lcd_sync();
}

static bool is_moving_state(vstate_t s)
{
  return s == S3_GRASPING || s == S8A_RUN_QUEUE || s == S8B_PLACING || s == S9_RELEASE_HOME;
}

static bool current_pose(cart_pose_t *out)
{
#if APP_VOICE_DRY_RUN
  *out = s_dry_pose;
  return true;
#endif
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
#if !APP_VOICE_DRY_RUN
  joint_vec_t now;
  motion_get_current(&now);
  if (now.deg[J2_SHOULDER] < MOTION_J2_COUPLED_DEG) script_add(STEP_HOME_RAISE, NULL, false);
#endif
  script_add(STEP_HOME, NULL, false);
  (void)script_start(S9_RELEASE_HOME, false);
}

/* S7 / S11 go back where they came from on "no" / "stop" */
static void ask_place(dest_t d, vstate_t from)
{
  s_dest = d;
  s_return = from;
  set_state(S7_CONFIRM_PLACE);
}

static void ask_home(bool open_first, vstate_t from)
{
  s_home_open = open_first;
  s_return = from;
  set_state(S11_CONFIRM_HOME);
}

/* ---------------------------------------------------------------- word filter */
#if APP_USE_VOICE
static uint32_t bit(int w) { return (w >= 0 && w < 32) ? (1u << w) : 0u; }

static uint32_t accepted_words(void)
{
  uint32_t m = bit(W.stop);
  uint32_t jog = bit(W.fwd) | bit(W.back) | bit(W.left) | bit(W.right);
  switch (s_state) {
    case S0_IDLE:
      m |= bit(W.move) | bit(W.pick) | bit(W.grab) | bit(W.home);
      break;
    case S1_WAIT_OBJECT:
      if (!s_pi_wait) for (int i = 0; i < (int)OBJ_BOX_COUNT; i++) m |= bit(W.box[i]);
      break;
    case S7_CONFIRM_PLACE:
      if (!s_pi_wait) m |= bit(W.yes) | bit(W.no);
      break;
    case S2_CONFIRM_OBJECT: case S11_CONFIRM_HOME:
      m |= bit(W.yes) | bit(W.no);
      break;
    case S4_HOLDING:
      m |= bit(W.hand) | bit(W.station) | bit(W.base) | bit(W.place) | bit(W.drop) | jog | bit(W.home);
      break;
    case S6_JOG_QUEUE:
      m |= jog | bit(W.undo) | bit(W.yes) | bit(W.no);
      break;
    case S10_STOPPED:
      m |= bit(W.place) | bit(W.drop) | bit(W.home);
      break;
    default:
      break;
  }
  return m;
}
#endif

static void update_filter(void)
{
#if APP_USE_VOICE
  voice_set_accept_mask(accepted_words());
#endif
}

/* ---------------------------------------------------------------- per-state word handling */
static int word_to_object(int w)
{
  for (int i = 0; i < (int)OBJ_BOX_COUNT; i++) if (w == W.box[i]) return i;
  return -1;
}

/* does the script just built fit? (dry run: always) */
static bool script_fits(void)
{
  kin_status_t st = script_check(true);
  for (int i = 0; i < s_base_len; i++) s_script[i] = s_base[i];
  s_script_len = s_base_len;
  return st == KIN_OK;
}

/* open, above, down, close, lift */
static void build_grasp(float approach_mm, float lift_mm)
{
  cart_pose_t grasp = s_obj_pose, above, lift;
  grasp.z_mm -= GRASP_BELOW_TOP_MM;
  above = grasp;
  above.z_mm += approach_mm;
  lift = grasp;
  lift.z_mm += lift_mm;
  script_clear();
  script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_POSE, &above, false);
  script_add(STEP_POSE, &grasp, false);
  script_add(STEP_GRIPPER, NULL, true);
  script_add(STEP_POSE, &lift, false);
}

static void start_grasp(void)
{
  /* try: long approach + high hold, long, short + high hold, short */
  static const float TRY[4][2] = {
    { APPROACH_MM, APPROACH_MM + HOLD_EXTRA_MM }, { APPROACH_MM, APPROACH_MM },
    { APPROACH_MIN_MM, APPROACH_MIN_MM + HOLD_EXTRA_MM }, { APPROACH_MIN_MM, APPROACH_MIN_MM },
  };
  for (int i = 0; i < 4; i++) {
    build_grasp(TRY[i][0], TRY[i][1]);
    if (i == 3 || script_fits()) break;
  }
  (void)script_start(S3_GRASPING, true);
}

/* above -> release height -> open -> above */
static void build_place(const cart_pose_t *target, float release_mm, float approach_mm)
{
  cart_pose_t above = *target, release = *target;
  above.z_mm += approach_mm;
  release.z_mm += release_mm;
  script_clear();
  script_add(STEP_POSE, &above, false);
  script_add(STEP_POSE, &release, false);
  script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_POSE, &above, false);
}

static void place_at(const cart_pose_t *target, float release_mm)
{
  build_place(target, release_mm, APPROACH_MM);
  if (APPROACH_MIN_MM > release_mm + 5.0f && !script_fits()) build_place(target, release_mm, APPROACH_MIN_MM);
  (void)script_start(S8B_PLACING, true);
}

static bool dest_from_pi(dest_t d) { return d == DEST_HAND || d == DEST_STATION; }
static object_id_t dest_obj(dest_t d) { return d == DEST_HAND ? OBJ_HAND : OBJ_STATION; }

/* ask the pi after "yes" (the hand can move). the reply calls place_at() */
static void find_dest(void)
{
#if APP_USE_PI
  if (pi_link_find(dest_obj(s_dest))) s_pi_wait = true;
  else lcd_show_message("PI BUSY", 2000u);
#else
  lcd_show_message("PI DISABLED", 2000u);
#endif
}

static void start_place(void)
{
  cart_pose_t target;
  if (dest_from_pi(s_dest)) {
    find_dest();
    return;
  }
  if (s_dest == DEST_HERE) {
    if (!current_pose(&target)) { lcd_show_message("IK NOT READY", 3000u); return; }
    target.z_mm -= APPROACH_MM;                     /* TODO: lower to the table, not a fixed amount */
  } else {
    if (!DEST_CALIBRATED && !APP_VOICE_DRY_RUN) { lcd_show_message("DEST NOT SET", 3000u); return; }
    target = DEST_POSE[s_dest];
  }
  place_at(&target, 0.0f);
}

static void run_queue(void)
{
  cart_pose_t p;
  if (!current_pose(&p)) { lcd_show_message("IK NOT READY", 3000u); return; }
  float travel = 0.0f;
  script_clear();
  for (int i = 0; i < s_queue_n; i++) {
    float step_mm = 0.1f * (float)s_queue[i].step_01;
    p.x_mm += JOG_VEC[s_queue[i].dir][0] * step_mm;
    p.y_mm += JOG_VEC[s_queue[i].dir][1] * step_mm;
    travel += step_mm;
    script_add(STEP_POSE, &p, false);
  }
  if (travel > JOG_MAX_TRAVEL_MM) { lcd_show_message("QUEUE TOO LONG", 3000u); script_clear(); return; }
  (void)script_start(S8A_RUN_QUEUE, false);
}

static void on_word(int w)
{
  if (w == W.stop) {
    if (is_moving_state(s_state)) { controlled_stop(); return; }
    switch (s_state) {
      case S1_WAIT_OBJECT: case S2_CONFIRM_OBJECT: cancel_pi(); set_state(S0_IDLE); break;
      case S6_JOG_QUEUE:   s_queue_n = 0;          set_state(S4_HOLDING); break;
      case S7_CONFIRM_PLACE: case S11_CONFIRM_HOME: cancel_pi(); set_state(s_return); break;
      default: break;
    }
    return;
  }

  int obj;
  switch (s_state) {
    case S0_IDLE:
      if (w == W.move || w == W.pick || w == W.grab) set_state(S1_WAIT_OBJECT);
      else if (w == W.home) ask_home(false, S0_IDLE);
      break;

    case S1_WAIT_OBJECT:
      obj = word_to_object(w);
      if (obj >= 0 && !s_pi_wait) {
#if APP_VOICE_FAKE_PI
        s_obj = (object_id_t)obj; s_pi_wait = true;
        lcd_voice_set_pick(lcd_box_of(s_obj));
        LOG("fake-pi: OBJ %d %d %d mm\r\n", (int)FAKE_OBJ_MM[0], (int)FAKE_OBJ_MM[1], (int)FAKE_OBJ_MM[2]);
        (void)app_event_post(EVT_PI_OBJECT, (uint16_t)obj, FAKE_OBJ_MM[0], FAKE_OBJ_MM[1], FAKE_OBJ_MM[2]);
#elif APP_USE_PI
        if (pi_link_find((object_id_t)obj)) {
          s_obj = (object_id_t)obj; s_pi_wait = true;
          lcd_voice_set_pick(lcd_box_of(s_obj));
        }
        else lcd_show_message("PI BUSY", 2000u);
#else
        lcd_show_message("PI DISABLED", 2000u);
#endif
      }
      break;

    case S2_CONFIRM_OBJECT:
      if (w == W.yes) start_grasp();
      else if (w == W.no) set_state(S1_WAIT_OBJECT);
      break;

    case S4_HOLDING:
      if (w == W.hand)         ask_place(DEST_HAND, S4_HOLDING);
      else if (w == W.station) ask_place(DEST_STATION, S4_HOLDING);
      else if (w == W.base)    ask_place(DEST_BASE, S4_HOLDING);
      else if (w == W.place || w == W.drop) ask_place(DEST_HERE, S4_HOLDING);
      else if (w == W.fwd || w == W.back || w == W.left || w == W.right) {
        s_queue_n = 0;
        set_state(S6_JOG_QUEUE);
        on_word(w);
      }
      else if (w == W.home && s_holding) lcd_show_message("RELEASE FIRST", 3000u);
      break;

    case S6_JOG_QUEUE: {
      int d = (w == W.fwd) ? JOG_FWD : (w == W.back) ? JOG_BACK : (w == W.left) ? JOG_LEFT
            : (w == W.right) ? JOG_RIGHT : -1;
      if (d >= 0) {
        if (s_queue_n < JOG_MAX_ITEMS) {
          s_queue[s_queue_n].dir = (jog_dir_t)d;
          s_queue[s_queue_n].step_01 = s_jog_step_01;
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
      lcd_queue_sync();
      break;
    }

    case S7_CONFIRM_PLACE:
      if (s_pi_wait) break;
      if (w == W.yes) start_place();
      else if (w == W.no) set_state(s_return);
      break;

    case S10_STOPPED:
      if (w == W.place || w == W.drop) ask_place(DEST_HERE, S10_STOPPED);
      else if (w == W.home) ask_home(true, S10_STOPPED);
      break;

    case S11_CONFIRM_HOME:
      if (w == W.yes) {
        go_home_then_idle(s_home_open);
        if (s_state != S9_RELEASE_HOME) set_state(s_return);
      }
      else if (w == W.no) set_state(s_return);
      break;

    default:
      break;
  }
}

/* ---------------------------------------------------------------- mode ops */
static int id(const char *name)
{
#if APP_USE_VOICE
  return voice_class_id(name);
#else
  (void)name;
  return -2;
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
  if (APP_VOICE_DRY_RUN) LOG("voice-mode: DRY RUN (moves are logged, nothing moves)%s\r\n",
                             APP_VOICE_FAKE_PI ? ", FAKE PI" : "");
  /* a new task every time, but a held box stays held */
  script_clear();
  set_state(s_holding ? S4_HOLDING : S0_IDLE);
  if (!s_holding) lcd_voice_set_pick(-1);
  for (int i = 0; i < LCD_BOX_COUNT; i++) lcd_voice_set_detected((lcd_box_t)i, false);
  lcd_sync();
  update_filter();
}

static void exit_(void)
{
  motion_hold();
  script_clear();
  cancel_pi();
  s_queue_n = 0;
#if APP_USE_LCD
  lcd_hide_status();
  lcd_voice_set_queue("");
#endif
#if APP_USE_VOICE
  voice_set_accept_mask(VOICE_ACCEPT_ALL);
#endif
}

static void on_motion_done(void)
{
  script_result_t r = script_on_motion_done();
  if (r == SCRIPT_FAILED) {
    lcd_show_message("MOVE FAILED", 3000u);
    controlled_stop();
    return;
  }
  if (r != SCRIPT_FINISHED) return;
  switch (s_state) {
    case S3_GRASPING:     set_state(S4_HOLDING); break;
    case S8A_RUN_QUEUE:   s_queue_n = 0; set_state(S4_HOLDING); break;
    case S8B_PLACING:
      s_lcd_complete = true;
      go_home_then_idle(false);
      if (s_state != S9_RELEASE_HOME) s_lcd_complete = false;
      break;
    case S9_RELEASE_HOME:
      if (!APP_VOICE_DRY_RUN && !at_home()) {
        lcd_show_message("HOME NOT REACHED", 3000u);
        controlled_stop();
        break;
      }
      set_state(S0_IDLE);
      break;
    default: break;
  }
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
      s_jog_step_01 = (int)clampi(e->v[0], 0, JOG_STEP_MAX_01MM);
      break;

    case EVT_PI_OBJECT:
      if (s_state == S7_CONFIRM_PLACE && s_pi_wait && dest_from_pi(s_dest) && (object_id_t)e->arg == dest_obj(s_dest)) {
        s_pi_wait = false;
        cart_pose_t target = { (float)e->v[0], (float)e->v[1], (float)e->v[2], GRASP_PITCH_PREF_DEG };
        place_at(&target, s_dest == DEST_HAND ? HAND_RELEASE_MM : STATION_RELEASE_MM);
      }
      else if (s_state == S1_WAIT_OBJECT && s_pi_wait && (object_id_t)e->arg == s_obj) {
        s_pi_wait = false;
        if (lcd_box_of(s_obj) >= 0) lcd_voice_set_detected((lcd_box_t)lcd_box_of(s_obj), true);
        s_obj_pose.x_mm = (float)e->v[0];
        s_obj_pose.y_mm = (float)e->v[1];
        s_obj_pose.z_mm = (float)e->v[2];
        s_obj_pose.pitch_deg = GRASP_PITCH_PREF_DEG;
        set_state(S2_CONFIRM_OBJECT);
      }
      break;

    case EVT_PI_NOTFOUND:
      if (s_pi_wait) {
        s_pi_wait = false;
        if (lcd_box_of((object_id_t)e->arg) >= 0) lcd_voice_set_detected((lcd_box_t)lcd_box_of((object_id_t)e->arg), false);
        lcd_show_message(s_state != S7_CONFIRM_PLACE ? "BOX NOT FOUND"
                         : s_dest == DEST_HAND ? "HAND NOT FOUND" : "STATION NOT FOUND", 3000u);
      }
      break;

    case EVT_PI_TIMEOUT:
      if (s_pi_wait) {
        s_pi_wait = false;
        lcd_show_message("PI NOT RESPONDING", 3000u);
        set_state(s_state == S7_CONFIRM_PLACE ? s_return : S0_IDLE);
      }
      break;

    case EVT_MOTION_DONE:
      if (!APP_VOICE_DRY_RUN) on_motion_done();
      break;

    case EVT_FAULT:
      if (e->v[0] == 0) break;
      if (is_moving_state(s_state) || s_script_running) controlled_stop();
      cancel_pi();
      break;

    default:
      break;
  }
}

static void step(void)
{
#if APP_VOICE_DRY_RUN
  if (s_script_running && ms_since(HAL_GetTick(), s_dry_step_ms) >= APP_VOICE_DRY_RUN_STEP_MS) on_motion_done();
#endif
  update_filter();
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
      char q[48];
      snprintf(q, sizeof q, "Queue %d/%d step %d.%02d cm", s_queue_n, JOG_MAX_ITEMS,
               s_jog_step_01 / 100, s_jog_step_01 % 100);
      snprintf(lines[1], LCD_COLS, "%.26s", q);
    } else if (s_state == S7_CONFIRM_PLACE && s_pi_wait) {
      snprintf(lines[1], LCD_COLS, "Finding %s", s_dest == DEST_HAND ? "hand" : "station");
    } else if (s_state == S7_CONFIRM_PLACE) {
      snprintf(lines[1], LCD_COLS, "Place at %s?", DEST_NAME[s_dest]);
    } else if (s_state == S11_CONFIRM_HOME) {
      snprintf(lines[1], LCD_COLS, "Go home%s?", s_home_open ? " + open" : "");
    } else {
      snprintf(lines[1], LCD_COLS, "Gripper: %s", s_holding ? "HOLDING" : "EMPTY");
    }
  }
  if (n > 2 && s_state == S6_JOG_QUEUE && s_queue_n > 0) {
    char q[48];
    snprintf(q, sizeof q, "last: %s %d.%02d cm", JOG_NAME[s_queue[s_queue_n - 1].dir],
             s_queue[s_queue_n - 1].step_01 / 100, s_queue[s_queue_n - 1].step_01 % 100);
    snprintf(lines[2], LCD_COLS, "%.26s", q);
  }
  if (n > 3 && (s_state == S2_CONFIRM_OBJECT || s_state == S6_JOG_QUEUE
                || (s_state == S7_CONFIRM_PLACE && !s_pi_wait) || s_state == S11_CONFIRM_HOME)) {
    snprintf(lines[3], LCD_COLS, "say YES or NO");
  }
}

const mode_ops_t MODE_VOICE_OPS = {
  .name = "VOICE",
  .manual = false,
  .enter = enter,
  .exit = exit_,
  .on_event = on_event,
  .step = step,
  .describe = describe,
};
