/* app_timing.c */
#include "common/app_timing.h"
#include "main.h"

void timing_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->LAR = 0xC5ACCE55u;                 /* unlock, needed on cortex-m7 */
  DWT->CYCCNT = 0u;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t timing_cycles(void)
{
  return DWT->CYCCNT;
}

uint32_t timing_us_since(uint32_t start_cycles)
{
  return (DWT->CYCCNT - start_cycles) / (SystemCoreClock / 1000000u);
}
