/* mode_voice.c - Auto/Voice mode: the S0..S11 task state machine (guide section 11.3)
 *
 * owner: voice (Supanut) + ik member for the motion scripts.
 *
 * words arrive as EVT_VOICE_WORD (class id). ids are looked up BY NAME on enter, so the
 * order of the model's classes does not matter here.
 *
 * word filter: every pass this mode tells voice/ which words the current state accepts
 * (voice_set_accept_mask). any other word is ignored by voice/ and does not start its
 * refractory time, so a ghost word cannot block the real one that follows. "stop" is
 * accepted in every state. on_word() still checks the word itself, so the filter is a
 * second line, not the only one.
 *
 * nothing moves without "yes": S2 (grasp), S6 (jog queue), S7 (place), S11 (go home).
 * waiting states wait for a valid word or "stop" for as long as it takes (no timeouts,
 * decided 25 Oct 2026). the arm never releases a held object on its own.
 *
 * changes from the guide (25 Oct 2026): S5 is merged into S7 (S7 shows the destination);
 * S11 CONFIRM HOME is new; "place" from S10 goes through S7.
 * "hand" (26 Oct 2026): no fixed pose. "yes" in S7 sends FIND hand and S7 waits (only "stop"
 * accepted); the reply's palm centre is the target, read once, and the gripper opens
 * HAND_RELEASE_MM above it, never at the palm. not found / no reply: back to the S7 / S4 choice.
 * "station" (27 Oct 2026): found by the pi too (depth), so it can be moved. the same flow as the hand:
 * "yes" sends FIND station; the reply is the free drop spot ON the station top, and the gripper opens
 * STATION_RELEASE_MM above it (the held block hangs below the gripper). no free spot = NOTFOUND.
 *
 * multi-step moves (grasp, place, go home, jog queue) are "scripts": a short list of
 * steps run one after another, the next step starts on EVT_MOTION_DONE. a stop or a
 * fatal fault aborts the script and goes to S10.
 * ik (28 Oct 2026, while ik is still switched off in firmware): grasp and place search their pitch
 * (GRASP_PITCH_*); every pose is solved before step 1; a target motion would clamp or refuse is a
 * failed step (JOINT LIMIT / MOTION REFUSED -> MOVE FAILED -> S10), never a move to another pose.
 *
 * bench flags (app_config_local.h only), to walk every state before the arm exists:
 *   APP_VOICE_DRY_RUN  each script step is logged ("dry-run: ...") and counts as done after
 *                      APP_VOICE_DRY_RUN_STEP_MS; no ik, no motion call, nothing moves.
 *                      the "current pose" is the last logged pose; uncalibrated destinations
 *                      use their placeholder pose.
 *   APP_VOICE_FAKE_PI  a box word in S1 gets an instant fake OBJ reply (FAKE_OBJ_MM), no pi.
 *
 * still TODO (marked TODO(voice) / TODO(ik)):
 *   - destination pose (base) from calibration
 *   - STATION_RELEASE_MM: block height under the gripper + clearance (measure on the real gripper)
 *   - HAND_RELEASE_MM: how high above the palm the gripper opens (team decision)
 *   - jog direction vectors in the robot-base frame, queue travel limit
 *   - grasp / place heights, approach distance
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
#define APPROACH_MM        50.0f   /* TODO(ik): height above the object before descending */
/* TODO(voice): gripper height above the palm where it opens. the held box hangs below the gripper,
 * so this must be >= (box below the gripper + clearance); if that reaches APPROACH_MM, raise APPROACH_MM too */
#define HAND_RELEASE_MM    30.0f
/* below "above", so the release pose is a real move (a no-move step never reports done) */
_Static_assert((int)HAND_RELEASE_MM > 0 && (int)HAND_RELEASE_MM < (int)APPROACH_MM, "0 < HAND_RELEASE_MM < APPROACH_MM");
/* TODO(voice): gripper height above the station top where it opens: the held block's height below the
 * gripper + a few mm, so the block sits on the top instead of being pushed into it */
#define STATION_RELEASE_MM 20.0f
_Static_assert((int)STATION_RELEASE_MM > 0 && (int)STATION_RELEASE_MM < (int)APPROACH_MM,
               "0 < STATION_RELEASE_MM < APPROACH_MM");
/* grasp / place orientation. pitch = the tool approach in the arm's vertical plane (kinematics.h):
 * 0 = gripper pointing up, 180 = straight down, smaller = tilted toward the arm's reach (home is ~148).
 * straight down only reaches x ~200..240 mm near the table, so grasp / place try 180 first and tilt
 * GRASP_PITCH_STEP_DEG at a time down to GRASP_PITCH_MIN_DEG (script_check). one pitch per script. */
#define GRASP_PITCH_PREF_DEG 180.0f
#define GRASP_PITCH_MIN_DEG  120.0f
#define GRASP_PITCH_STEP_DEG 10.0f
#define JOG_MAX_ITEMS      10      /* guide: bounded queue */
#define JOG_MAX_TRAVEL_MM  300.0f  /* TODO(voice): validate on the real arm */
#define JOG_DEFAULT_MM     20
#define SCRIPT_MAX         8

/* bench only (APP_VOICE_DRY_RUN / APP_VOICE_FAKE_PI): made-up positions, not measured */
#if APP_VOICE_DRY_RUN
static const cart_pose_t DRY_START_POSE = { 0.0f, 0.0f, 200.0f, 0.0f };
#endif
#if APP_VOICE_FAKE_PI
static const int32_t FAKE_OBJ_MM[3] = { 182, -45, 30 };   /* same example as the pi protocol docs */
#endif

/* hand and station are not fixed poses: "yes" asks the pi (FIND hand / FIND station) at that moment */
typedef enum { DEST_HAND = 0, DEST_STATION, DEST_BASE, DEST_HERE, DEST_COUNT } dest_t;
static const char *const DEST_NAME[DEST_COUNT] = { "HAND", "STATION", "BASE", "HERE" };

/* TODO(voice): set during calibration, then set DEST_CALIBRATED to 1 */
#define DEST_CALIBRATED 0
static const cart_pose_t DEST_POSE[3] = {
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* hand: unused, the position comes from the pi */
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* station: unused, the drop spot comes from the pi */
  { 0.0f, 0.0f, 0.0f, 0.0f },   /* base */
};

/* TODO(ik): confirm which way is +x / +y for the person speaking */
typedef enum { JOG_FWD = 0, JOG_BACK, JOG_LEFT, JOG_RIGHT } jog_dir_t;
static const float JOG_VEC[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
static const char *const JOG_NAME[4] = { "FWD", "BACK", "LEFT", "RIGHT" };

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

/* word ids, resolved by name */
typedef struct {
  int move, pick, grab, box[OBJ_BOX_COUNT], hand, station, base;
  int fwd, back, left, right, undo, place, drop, yes, no, home, stop;
} words_t;

static words_t     W;
static vstate_t    s_state = S0_IDLE;
static vstate_t    s_return = S0_IDLE;   /* where S7 / S11 go back to on "no" or "stop" */
static bool        s_home_open;          /* S11: open the gripper before going home (from S10) */
static object_id_t s_obj = OBJ_NONE;
static bool        s_pi_wait;
static cart_pose_t s_obj_pose;           /* fixed when the user says yes in S2 */
static dest_t      s_dest = DEST_HERE;
static bool        s_holding;            /* gripper closed on an object */
static int         s_jog_step_mm = JOG_DEFAULT_MM;
static struct { jog_dir_t dir; int step_mm; } s_queue[JOG_MAX_ITEMS];
static int         s_queue_n;

/* ---------------------------------------------------------------- scripts */
/* STEP_HOME_RAISE: before HOME when J2 is below MOTION_J2_COUPLED_DEG, lift J2 to it (J5 at least
 * MOTION_J5_WRIST_UP_DEG), so the home move never asks for a coupled-blocked wrist angle */
typedef enum { STEP_POSE, STEP_GRIPPER, STEP_HOME_RAISE, STEP_HOME } step_kind_t;
typedef struct { step_kind_t kind; cart_pose_t pose; bool close; } script_step_t;

static script_step_t s_script[SCRIPT_MAX];
static int  s_script_len, s_script_pos;
static bool s_script_running;
#if APP_VOICE_DRY_RUN
static cart_pose_t s_dry_pose = { 0.0f, 0.0f, 200.0f, 0.0f };   /* = DRY_START_POSE */
static uint32_t    s_dry_step_ms;                               /* when the current faked step began */
#endif

static void script_clear(void) { s_script_len = 0; s_script_pos = 0; s_script_running = false; }
static void set_state(vstate_t s);

#if APP_VOICE_DRY_RUN
/* log the step instead of moving; step() reports it done after APP_VOICE_DRY_RUN_STEP_MS */
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
      LOG("dry-run: step %d/%d raise shoulder before home\r\n", s_script_pos + 1, s_script_len);
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

/* the installed home pose, gripper unchanged (home never opens the gripper itself) */
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

/* all or nothing: a clamped or refused target is a failed step (the script stops via
 * SCRIPT_FAILED / controlled_stop), never a move to a different pose than planned */
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
  kin_status_t st = kin_ik(p, &now, &sol);
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
      lcd_show_message("GRIPPER LIMIT", 3000u);          /* clamped: not holding, script stops */
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
      lcd_show_message("HOME BLOCKED", 3000u);           /* would end early at a clamped pose */
      motion_hold();
      return false;
    }
    default:
      return false;
  }
}

/* s_holding follows the gripper step when it is commanded, not when the script ends, so a
 * stop after "open" in a place script knows the object is already released */
static bool script_run_current(void)
{
  const script_step_t *s = &s_script[s_script_pos];
#if APP_VOICE_DRY_RUN
  bool ok = dry_run_step(s);
  (void)run_step;                                     /* real motion is not used in a dry run */
#else
  bool ok = run_step(s);
#endif
  if (ok && s->kind == STEP_GRIPPER) s_holding = s->close;
  return ok;
}

/* every pose of the script must be solvable BEFORE the first step runs, so a script
 * never starts (e.g. opens the gripper) and then gets stuck halfway. each pose is solved from the
 * previous pose's solution (the arm's real order), so the J2/J5 check matches what motion sees */
static kin_status_t script_check_poses(void)
{
  joint_vec_t seed, sol;
  motion_get_current(&seed);
  for (int i = 0; i < s_script_len; i++) {
    if (s_script[i].kind != STEP_POSE) continue;
    kin_status_t st = kin_ik(&s_script[i].pose, &seed, &sol);
    if (st != KIN_OK) return st;
    seed = sol;
  }
  return KIN_OK;
}

static void script_set_pitch(float pitch_deg)
{
  for (int i = 0; i < s_script_len; i++) {
    if (s_script[i].kind == STEP_POSE) s_script[i].pose.pitch_deg = pitch_deg;
  }
}

/* search_pitch (grasp / place): straight down first, then tilt 10 deg at a time; the first pitch that
 * solves EVERY pose is used for the whole script, so the gripper does not turn between steps */
static bool script_check(bool search_pitch)
{
#if APP_VOICE_DRY_RUN
  if (search_pitch) script_set_pitch(GRASP_PITCH_PREF_DEG);
  return true;                                        /* no ik yet: every pose counts as reachable */
#endif
  kin_status_t st;
  if (!search_pitch) {
    st = script_check_poses();
  } else {
    st = KIN_OUT_OF_REACH;
    const int n = (int)((GRASP_PITCH_PREF_DEG - GRASP_PITCH_MIN_DEG) / GRASP_PITCH_STEP_DEG) + 1;
    for (int k = 0; k < n; k++) {
      float pitch = GRASP_PITCH_PREF_DEG - (float)k * GRASP_PITCH_STEP_DEG;
      script_set_pitch(pitch);
      st = script_check_poses();
      if (st == KIN_OK) {
        LOG("voice-mode: pitch %d deg\r\n", (int)pitch);
        break;
      }
      if (st == KIN_NOT_READY) break;               /* ik off: no pitch will help */
    }
  }
  if (st != KIN_OK) {
    lcd_show_message(kin_status_name(st), 3000u);
    return false;
  }
  return true;
}

/* checks the whole script, enters the moving state, runs step 1; on failure goes back */
static bool script_start(vstate_t moving, bool search_pitch)
{
  if (s_script_len == 0 || !script_check(search_pitch)) {
    script_clear();
    return false;
  }
  vstate_t before = s_state;
  set_state(moving);                                  /* before step 1, so the log reads in order */
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

/* S7 / S11 remember where they came from, so "no" and "stop" go back there */
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

/* the words the current state reacts to; "stop" always */
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
      if (!s_pi_wait) m |= bit(W.yes) | bit(W.no);   /* finding the hand / station: only stop */
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
      break;                                          /* moving states: only stop */
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
  (void)script_start(S3_GRASPING, true);             /* on failure it stays in S2 */
}

/* above -> release height -> open -> above. release_mm = how far above target the gripper opens */
static void place_at(const cart_pose_t *target, float release_mm)
{
  cart_pose_t above = *target, release = *target;
  above.z_mm += APPROACH_MM;
  release.z_mm += release_mm;
  script_clear();
  script_add(STEP_POSE, &above, false);
  script_add(STEP_POSE, &release, false);
  script_add(STEP_GRIPPER, NULL, false);
  script_add(STEP_POSE, &above, false);
  (void)script_start(S8B_PLACING, true);
}

/* destinations the pi finds (they can move); the rest are fixed poses */
static bool dest_from_pi(dest_t d) { return d == DEST_HAND || d == DEST_STATION; }
static object_id_t dest_obj(dest_t d) { return d == DEST_HAND ? OBJ_HAND : OBJ_STATION; }

/* the hand (and the station) can move, so the position is asked for after "yes", not when the word
 * was said. stays in S7 until the reply; EVT_PI_OBJECT then calls place_at() */
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
    target.z_mm -= APPROACH_MM;                     /* TODO(ik): lower to the table, not a fixed amount */
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
  joint_vec_t now, sol;
  motion_get_current(&now);
  float travel = 0.0f;
  script_clear();
  for (int i = 0; i < s_queue_n; i++) {
    p.x_mm += JOG_VEC[s_queue[i].dir][0] * (float)s_queue[i].step_mm;
    p.y_mm += JOG_VEC[s_queue[i].dir][1] * (float)s_queue[i].step_mm;
    travel += (float)s_queue[i].step_mm;
    if (!APP_VOICE_DRY_RUN && kin_ik(&p, &now, &sol) != KIN_OK) {   /* whole path checked before moving */
      lcd_show_message("OUT OF REACH", 3000u);
      script_clear();
      return;
    }
    script_add(STEP_POSE, &p, false);
  }
  if (travel > JOG_MAX_TRAVEL_MM) { lcd_show_message("QUEUE TOO LONG", 3000u); script_clear(); return; }
  (void)script_start(S8A_RUN_QUEUE, false);         /* keeps the pitch it is holding at */
}

static void on_word(int w)
{
  if (w == W.stop) {
    if (is_moving_state(s_state)) { controlled_stop(); return; }
    /* waiting state: cancel the pending choice */
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
        LOG("fake-pi: OBJ %d %d %d mm\r\n", (int)FAKE_OBJ_MM[0], (int)FAKE_OBJ_MM[1], (int)FAKE_OBJ_MM[2]);
        (void)app_event_post(EVT_PI_OBJECT, (uint16_t)obj, FAKE_OBJ_MM[0], FAKE_OBJ_MM[1], FAKE_OBJ_MM[2]);
#elif APP_USE_PI
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
      if (w == W.hand)         ask_place(DEST_HAND, S4_HOLDING);
      else if (w == W.station) ask_place(DEST_STATION, S4_HOLDING);
      else if (w == W.base)    ask_place(DEST_BASE, S4_HOLDING);
      else if (w == W.place || w == W.drop) ask_place(DEST_HERE, S4_HOLDING);
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
      if (s_pi_wait) break;                          /* finding the hand / station: only stop (handled above) */
      if (w == W.yes) start_place();                 /* on failure it stays here: say no */
      else if (w == W.no) set_state(s_return);
      break;

    case S10_STOPPED:
      if (w == W.place || w == W.drop) ask_place(DEST_HERE, S10_STOPPED);
      else if (w == W.home) ask_home(true, S10_STOPPED);  /* guide: home from S10 opens the gripper */
      break;

    case S11_CONFIRM_HOME:
      if (w == W.yes) {
        go_home_then_idle(s_home_open);
        if (s_state != S9_RELEASE_HOME) set_state(s_return);  /* could not start */
      }
      else if (w == W.no) set_state(s_return);
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
  if (APP_VOICE_DRY_RUN) LOG("voice-mode: DRY RUN (moves are logged, nothing moves)%s\r\n",
                             APP_VOICE_FAKE_PI ? ", FAKE PI" : "");
  /* entering never resumes an old task, but a held object stays held */
  script_clear();
  set_state(s_holding ? S4_HOLDING : S0_IDLE);
  update_filter();
}

static void exit_(void)
{
  motion_hold();
  script_clear();
  cancel_pi();
  s_queue_n = 0;
#if APP_USE_VOICE
  voice_set_accept_mask(VOICE_ACCEPT_ALL);            /* other modes do not use words */
#endif
}

static void on_motion_done(void)
{
  script_result_t r = script_on_motion_done();
  if (r == SCRIPT_FAILED) {                           /* e.g. the pose changed: stop safely, wait for the user */
    lcd_show_message("MOVE FAILED", 3000u);
    controlled_stop();
    return;
  }
  if (r != SCRIPT_FINISHED) return;
  switch (s_state) {
    case S3_GRASPING:     set_state(S4_HOLDING); break;   /* s_holding set by the close step */
    case S8A_RUN_QUEUE:   s_queue_n = 0; set_state(S4_HOLDING); break;
    case S8B_PLACING:     go_home_then_idle(false); break;  /* s_holding cleared by the open step */
    case S9_RELEASE_HOME:
      if (!APP_VOICE_DRY_RUN && !at_home()) {             /* a target clamped on the way: not home */
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
      s_jog_step_mm = (int)clampi(e->v[0], 5, 100);
      break;

    case EVT_PI_OBJECT:
      if (s_state == S7_CONFIRM_PLACE && s_pi_wait && dest_from_pi(s_dest) && (object_id_t)e->arg == dest_obj(s_dest)) {
        s_pi_wait = false;
        cart_pose_t target = { (float)e->v[0], (float)e->v[1], (float)e->v[2], GRASP_PITCH_PREF_DEG };  /* place_at picks the pitch */
        place_at(&target, s_dest == DEST_HAND ? HAND_RELEASE_MM : STATION_RELEASE_MM);  /* on failure: stays in S7 */
      }
      else if (s_state == S1_WAIT_OBJECT && s_pi_wait && (object_id_t)e->arg == s_obj) {
        s_pi_wait = false;
        s_obj_pose.x_mm = (float)e->v[0];
        s_obj_pose.y_mm = (float)e->v[1];
        s_obj_pose.z_mm = (float)e->v[2];
        s_obj_pose.pitch_deg = GRASP_PITCH_PREF_DEG;  /* start_grasp picks the final pitch */
        set_state(S2_CONFIRM_OBJECT);
      }
      break;

    case EVT_PI_NOTFOUND:
      if (s_pi_wait) {
        s_pi_wait = false;
        lcd_show_message(s_state != S7_CONFIRM_PLACE ? "BOX NOT FOUND"
                         : s_dest == DEST_HAND ? "HAND NOT FOUND" : "STATION NOT FOUND", 3000u);
      }
      break;

    case EVT_PI_TIMEOUT:
      if (s_pi_wait) {
        s_pi_wait = false;
        lcd_show_message("PI NOT RESPONDING", 3000u);
        set_state(s_state == S7_CONFIRM_PLACE ? s_return : S0_IDLE);   /* finding hand / station: still holding */
      }
      break;

    case EVT_MOTION_DONE:
      if (!APP_VOICE_DRY_RUN) on_motion_done();       /* dry run: step() fakes it */
      break;

    case EVT_FAULT:
      if (e->v[0] == 0) break;                        /* only fatal faults stop the task */
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
  update_filter();                                    /* after this pass's events: state and pi wait are final */
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
    snprintf(lines[2], LCD_COLS, "last: %s %d", JOG_NAME[s_queue[s_queue_n - 1].dir], s_queue[s_queue_n - 1].step_mm);
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
