/* app_events.h - one event queue from the producers (isr or main loop) to modes_step()
 *
 * New event: append it to its group (never renumber) and note what arg / v[] mean.
 */
#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  EVT_NONE = 0,

  /* input */
  EVT_BTN_MODE = 10,        /* unused, kept so the numbers stay */
  EVT_ENC_STEP,             /* encoder turned: v[0] = detents (+cw / -ccw) */
  EVT_ENC_PRESS,            /* encoder knob pressed */
  EVT_JOY_PRESS,            /* joystick pressed while centred (= fine/fast toggle) */
  EVT_ENABLE_PRESSED,       /* unused */
  EVT_ENABLE_RELEASED,      /* unused */

  /* safety */
  EVT_ESTOP_PRESSED = 30,   /* unused */
  EVT_ESTOP_RELEASED,       /* unused */
  EVT_RAIL_LOST,            /* unused */
  EVT_RAIL_BACK,            /* unused */
  EVT_LIMIT_HIT,            /* unused */
  EVT_FAULT,                /* arg = app_fault_t */

  /* voice */
  EVT_VOICE_WORD = 50,      /* arg = class id, v[0] = prob x1000, v[1] = mic ch, v[2] = snr dB x10 */

  /* pi link */
  EVT_PI_OBJECT = 70,       /* arg = object_id_t, v[0..2] = x, y, z mm (robot frame) */
  EVT_PI_NOTFOUND,          /* arg = object_id_t */
  EVT_PI_TIMEOUT,           /* no answer within APP_PI_TIMEOUT_MS */

  /* touch screen */
  EVT_TOUCH_MODE_TAB = 90,  /* arg = arm_mode_t */
  EVT_TOUCH_STOP,           /* software stop (not a safety stop) */
  EVT_TOUCH_UNDO,           /* jog queue undo */
  EVT_TOUCH_STEP_CM,        /* jog step slider: v[0] = 0.1 mm, 0..200 */
  EVT_TOUCH_CONFIRM,        /* "safe restart" confirm button */
  EVT_TOUCH_SELECT,         /* < / > on the single / group page: v[0] = +1 / -1 */

  /* motion */
  EVT_MOTION_DONE = 110,    /* reached the target and idle */

  /* new groups start here, 20 per group */
  EVT_USER_BASE = 200
} evt_type_t;

typedef struct {
  uint16_t type;     /* evt_type_t */
  uint16_t arg;
  int32_t  v[3];
  uint32_t tick_ms;  /* when it happened (button presses: the debounced edge time) */
} app_event_t;

void     app_events_init(void);

/* any context. false = queue full, event dropped */
bool     app_event_post(evt_type_t type, uint16_t arg, int32_t v0, int32_t v1, int32_t v2);
static inline bool app_event_post0(evt_type_t type) { return app_event_post(type, 0, 0, 0, 0); }
/* same, with the time it happened */
bool     app_event_post_at(evt_type_t type, uint16_t arg, int32_t v0, int32_t v1, int32_t v2, uint32_t tick_ms);

/* main loop only, called by modes_step() */
bool     app_event_pop(app_event_t *out);

/* drop everything (when APP_USE_MODES is 0) */
void     app_events_drain(void);

uint32_t app_events_dropped(void);
uint32_t app_events_high_water(void);

#endif /* APP_EVENTS_H */
