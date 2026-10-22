/* app_fault.c */
#include "common/app_fault.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "main.h"

static volatile app_fault_t s_last = FAULT_NONE;
static volatile uint32_t    s_count;
static volatile uint32_t    s_reported;
static volatile bool        s_fatal;

void app_fault_raise(app_fault_t code, bool fatal)
{
  s_last = code;
  s_count = s_count + 1u;
  if (fatal) s_fatal = true;
  /* event queue full would loop forever through FAULT_EVENT_DROPPED, so never post that one */
  if (code != FAULT_EVENT_DROPPED) {
    (void)app_event_post(EVT_FAULT, (uint16_t)code, fatal ? 1 : 0, 0, 0);
  }
}

bool        app_fault_is_fatal(void) { return s_fatal; }
app_fault_t app_fault_last(void)     { return s_last; }
uint32_t    app_fault_count(void)    { return s_count; }

void app_fault_report(void)
{
  uint32_t n = s_count;
  if (n != s_reported) {
    LOG("fault: code %d (%lu total)%s\r\n", (int)s_last, (unsigned long)n,
        s_fatal ? " FATAL - motion frozen until reset" : "");
    s_reported = n;
  }
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, s_fatal ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
