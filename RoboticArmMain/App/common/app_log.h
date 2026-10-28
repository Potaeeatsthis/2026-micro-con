/* app_log.h - debug printf on USART3 (ST-Link COM port, 921600 8N1)
 *
 * Never log from an interrupt (printf blocks ~11 us per char). No %f: print scaled integers.
 * Start each line with the module name, e.g. "motion: ...".
 */
#ifndef APP_LOG_H
#define APP_LOG_H

#include <stdio.h>

void app_log_init(void);

#define LOG(...)     printf(__VA_ARGS__)
#define LOG_ERR(...) printf("ERROR " __VA_ARGS__)

#endif /* APP_LOG_H */
