/* app_fault.h - one place to report errors
 *
 * Safe from any context. A fatal fault freezes motion and lights LD3 until reset; the others are
 * counted and logged. New code: append it, never renumber.
 */
#ifndef APP_FAULT_H
#define APP_FAULT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  FAULT_NONE = 0,
  FAULT_DMA_REGION,        /* core: linker edit or MPU region missing (fatal) */
  FAULT_INIT,              /* any module: init failed, arg in the log line */
  FAULT_LOOP_OVERRUN,      /* core: one main-loop pass took longer than APP_LOOP_WARN_US */
  FAULT_EVENT_DROPPED,     /* core: event queue was full */
  FAULT_ADC,               /* input: HAL_ADC_ErrorCallback */
  FAULT_SAI,               /* voice: HAL_SAI_ErrorCallback */
  FAULT_UART_PI,           /* pi: HAL_UART_ErrorCallback on USART6 */
  FAULT_VOICE_OVERRUN,     /* voice: audio skipped, main loop too slow */
  FAULT_USER_BASE = 100    /* new codes go here */
} app_fault_t;

void        app_fault_raise(app_fault_t code, bool fatal);
bool        app_fault_is_fatal(void);
app_fault_t app_fault_last(void);
uint32_t    app_fault_count(void);

/* main loop: logs faults raised since the last call (from isr too), drives LD3 */
void        app_fault_report(void);

#endif /* APP_FAULT_H */
