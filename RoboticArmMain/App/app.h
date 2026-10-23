/* app.h - the two calls main.c makes
 *
 * owner: core.
 *   main.c USER CODE 2:  app_init();
 *   main.c USER CODE 3:  app_loop();     (inside while(1): ONE pass, must return quickly)
 */
#ifndef APP_H
#define APP_H

#include <stdint.h>

void     app_init(void);
void     app_loop(void);

/* called by app_callbacks.c from the TIM6 isr (50 Hz) */
void     app_control_tick(void);

uint32_t app_loop_max_us(void);   /* worst main-loop pass since boot */

#endif /* APP_H */
