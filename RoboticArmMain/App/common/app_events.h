/* app_events.h - one event queue between producers (isr + main loop) and the mode logic
 *
 * owner: core.
 *   producers: input, safety, voice, pi_link, lcd/touch, motion. any context, isr included
 *   consumer:  modes_step() only (main loop). nobody else pops
 *
 * why: a module that detects something just posts an event; it does not need to know
 * which mode is active or who reacts. this keeps the feature folders independent.
 *
 * adding an event type: append it in the right group (never renumber old ones),
 * write what arg / v[] mean next to it, and handle it in the mode(s) that care.
 */
#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  EVT_NONE = 0,

  /* input (owner: input) */
  EVT_BTN_MODE = 10,        /* unused since 27 Oct 2026 (no MODE button); kept so numbers stay */
  EVT_ENC_STEP,             /* encoder turned: v[0] = detents (+cw / -ccw) */
  EVT_ENC_PRESS,            /* encoder knob pressed */
  EVT_JOY_PRESS,            /* joystick pressed while centred (= fine/fast toggle) */
  EVT_ENABLE_PRESSED,       /* unused since 27 Oct 2026 (no hold-to-enable) */
  EVT_ENABLE_RELEASED,      /* unused since 27 Oct 2026 (no hold-to-enable) */

  /* safety (owner: core/safety) */
  EVT_ESTOP_PRESSED = 30,   /* unused since 27 Oct 2026 (no e-stop) */
  EVT_ESTOP_RELEASED,       /* unused since 27 Oct 2026 (no e-stop) */
  EVT_RAIL_LOST,            /* servo power gone: targets frozen */
  EVT_RAIL_BACK,            /* servo power back: still needs safety_confirm_restart() */
  EVT_LIMIT_HIT,            /* arg = switch index 0..3 */
  EVT_FAULT,                /* arg = app_fault_t */

  /* voice (owner: voice) */
  EVT_VOICE_WORD = 50,      /* arg = class id, v[0] = prob x1000, v[1] = mic ch, v[2] = snr dB x10 */

  /* pi link (owner: pi) */
  EVT_PI_OBJECT = 70,       /* arg = object_id_t, v[0..2] = x, y, z in mm (robot-base frame) */
  EVT_PI_NOTFOUND,          /* arg = object_id_t */
  EVT_PI_TIMEOUT,           /* no answer within APP_PI_TIMEOUT_MS */

  /* touch screen (owner: lcd) */
  EVT_TOUCH_MODE_TAB = 90,  /* arg = arm_mode_t */
  EVT_TOUCH_STOP,           /* software stop button (convenience, not safety) */
  EVT_TOUCH_UNDO,           /* jog queue undo */
  EVT_TOUCH_STEP_CM,        /* jog step slider: v[0] = step in mm */
  EVT_TOUCH_CONFIRM,        /* "safe restart" confirm button */

  /* motion (owner: motion) */
  EVT_MOTION_DONE = 110,    /* trajectory reached its target and is idle */

  /* new groups start here; reserve a block of 20 per group */
  EVT_USER_BASE = 200
} evt_type_t;

typedef struct {
  uint16_t type;     /* evt_type_t */
  uint16_t arg;
  int32_t  v[3];
  uint32_t tick_ms;  /* HAL_GetTick() when posted */
} app_event_t;

void     app_events_init(void);

/* safe from any context (isr or main). returns false if the queue was full (event dropped) */
bool     app_event_post(evt_type_t type, uint16_t arg, int32_t v0, int32_t v1, int32_t v2);
static inline bool app_event_post0(evt_type_t type) { return app_event_post(type, 0, 0, 0, 0); }

/* main loop only, called by modes_step() */
bool     app_event_pop(app_event_t *out);

/* drops everything (used when APP_USE_MODES is 0 so the queue never fills) */
void     app_events_drain(void);

uint32_t app_events_dropped(void);
uint32_t app_events_high_water(void);

#endif /* APP_EVENTS_H */
