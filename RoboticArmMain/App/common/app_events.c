/* app_events.c - fixed-size ring, protected by masking interrupts for a few cycles */
#include "common/app_events.h"
#include "main.h"

#define EVT_QUEUE_LEN 32u   /* power of 2 */

static app_event_t       s_q[EVT_QUEUE_LEN];
static volatile uint32_t s_head;      /* next write */
static volatile uint32_t s_tail;      /* next read */
static volatile uint32_t s_dropped;
static volatile uint32_t s_high_water;

void app_events_init(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  s_head = 0;
  s_tail = 0;
  s_dropped = 0;
  s_high_water = 0;
  __set_PRIMASK(primask);
}

bool app_event_post(evt_type_t type, uint16_t arg, int32_t v0, int32_t v1, int32_t v2)
{
  return app_event_post_at(type, arg, v0, v1, v2, HAL_GetTick());
}

bool app_event_post_at(evt_type_t type, uint16_t arg, int32_t v0, int32_t v1, int32_t v2, uint32_t tick_ms)
{
  bool ok = false;
  uint32_t primask = __get_PRIMASK();   /* restores the caller's state, so nesting is safe */
  __disable_irq();

  uint32_t used = s_head - s_tail;
  if (used < EVT_QUEUE_LEN) {
    app_event_t *e = &s_q[s_head & (EVT_QUEUE_LEN - 1u)];
    e->type = (uint16_t)type;
    e->arg = arg;
    e->v[0] = v0;
    e->v[1] = v1;
    e->v[2] = v2;
    e->tick_ms = tick_ms;
    s_head = s_head + 1u;
    if (used + 1u > s_high_water) s_high_water = used + 1u;
    ok = true;
  } else {
    s_dropped = s_dropped + 1u;
  }

  __set_PRIMASK(primask);
  return ok;
}

bool app_event_pop(app_event_t *out)
{
  bool ok = false;
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  if (s_tail != s_head) {
    *out = s_q[s_tail & (EVT_QUEUE_LEN - 1u)];
    s_tail = s_tail + 1u;
    ok = true;
  }
  __set_PRIMASK(primask);
  return ok;
}

void app_events_drain(void)
{
  app_event_t e;
  while (app_event_pop(&e)) {
  }
}

uint32_t app_events_dropped(void)    { return s_dropped; }
uint32_t app_events_high_water(void) { return s_high_water; }
