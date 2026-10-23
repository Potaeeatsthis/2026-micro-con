/* app_log.h - debug printf on USART3 (ST-Link virtual COM port, 921600 8N1)
 *
 * owner: core.
 * rules:
 *   - never log from an interrupt handler or a HAL callback (printf blocks ~11 us per char)
 *   - no %f: newlib-nano has no float printf unless the project enables it.
 *     print floats as scaled integers, e.g. LOG("v=%ld mV\r\n", (long)(v * 1000.0f))
 *   - prefix each line with your module name, e.g. "motion: ..."
 */
#ifndef APP_LOG_H
#define APP_LOG_H

#include <stdio.h>

void app_log_init(void);

#define LOG(...)     printf(__VA_ARGS__)
#define LOG_ERR(...) printf("ERROR " __VA_ARGS__)

#endif /* APP_LOG_H */
