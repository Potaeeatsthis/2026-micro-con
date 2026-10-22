/* dma_buf.c - boot-time check of the two manual steps that are easy to forget:
 * the linker-script edit (.dma_buffer -> RAM_DMA) and the MPU region 0 in CubeMX */
#include "common/dma_buf.h"
#include "common/app_log.h"
#include "main.h"

static volatile uint8_t s_probe[32] DMA_BUF;

static bool probe_in_region(void)
{
  uint32_t a = (uint32_t)&s_probe[0];
  return (a >= DMA_REGION_BASE) && (a + sizeof(s_probe) <= DMA_REGION_BASE + DMA_REGION_SIZE);
}

/* true if an enabled MPU region starts at DMA_REGION_BASE and is not cacheable */
static bool mpu_region_ok(void)
{
  if ((MPU->CTRL & MPU_CTRL_ENABLE_Msk) == 0u) return false;

  uint32_t n = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
  for (uint32_t r = 0; r < n; r++) {
    MPU->RNR = r;
    uint32_t rasr = MPU->RASR;
    uint32_t base = MPU->RBAR & MPU_RBAR_ADDR_Msk;
    if ((rasr & MPU_RASR_ENABLE_Msk) == 0u) continue;
    if (base != DMA_REGION_BASE) continue;
    bool cacheable = (rasr & MPU_RASR_C_Msk) != 0u;
    return !cacheable;
  }
  return false;
}

bool dma_buf_check(void)
{
  bool ok = true;
  if (!probe_in_region()) {
    LOG_ERR("dma_buf: .dma_buffer is at 0x%08lX, not in SRAM2 -> linker script edit missing\r\n",
            (unsigned long)(uint32_t)&s_probe[0]);
    ok = false;
  }
  if (!mpu_region_ok()) {
    LOG_ERR("dma_buf: no non-cacheable MPU region at 0x%08lX -> check CORTEX_M7 settings in the .ioc\r\n",
            (unsigned long)DMA_REGION_BASE);
    ok = false;
  }
  return ok;
}
