/* app_timing.h - DWT cycle counter for timing code
 *
 *     uint32_t t0 = timing_cycles();
 *     do_work();
 *     uint32_t us = timing_us_since(t0);
 * wraps after ~19.9 s at 216 MHz.
 */
#ifndef APP_TIMING_H
#define APP_TIMING_H

#include <stdint.h>

void     timing_init(void);
uint32_t timing_cycles(void);
uint32_t timing_us_since(uint32_t start_cycles);

#endif /* APP_TIMING_H */
