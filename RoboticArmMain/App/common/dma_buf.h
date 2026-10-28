/* dma_buf.h - put DMA buffers in non-cacheable SRAM2
 *
 * The D-cache is on (the CNN needs it), so a DMA buffer read through the cache can be stale. The MPU
 * makes SRAM2 (0x2007C000, 16 KB) non-cacheable and the linker puts .dma_buffer there:
 *
 *     static uint16_t adc_buf[60] DMA_BUF;
 *
 * Every buffer passed to a HAL_..._DMA() call uses DMA_BUF. 16 KB total, see TEAM_GUIDE.md.
 */
#ifndef APP_DMA_BUF_H
#define APP_DMA_BUF_H

#include <stdbool.h>

#define DMA_BUF __attribute__((section(".dma_buffer"), aligned(32)))

#define DMA_REGION_BASE 0x2007C000u
#define DMA_REGION_SIZE 0x00004000u

/* boot check that the linker section and MPU region are in place */
bool dma_buf_check(void);

#endif /* APP_DMA_BUF_H */
