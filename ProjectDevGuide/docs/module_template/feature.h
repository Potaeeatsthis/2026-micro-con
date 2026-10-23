/* feature.h - TEMPLATE for a new module. copy this folder to App/<name>/ and rename.
 *
 * owner: <your name>
 * one sentence: what this module does and what it does NOT do.
 *
 * the usual shape (delete what you do not need):
 *   <name>_init()    once, from app_init()        start peripherals, check config
 *   <name>_step()    every main-loop pass         the real work; must return in a few ms
 *   <name>_tick()    TIM6 isr, 50 Hz, ONLY if it truly must be in an isr (< 50 us)
 *   <name>_on_xxx()  from app_callbacks.c         isr: copy / flag only, never printf
 *   getters          for other modules and the lcd
 *   events           app_event_post(EVT_...) for "something happened" (see app_events.h)
 */
#ifndef APP_FEATURE_H
#define APP_FEATURE_H

#include <stdint.h>
#include <stdbool.h>

void feature_init(void);
void feature_step(void);

#endif /* APP_FEATURE_H */
