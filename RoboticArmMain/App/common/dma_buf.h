/* dma_buf.h - put every DMA buffer in the non-cacheable SRAM2 region
 *
 * owner: core.
 * the D-cache is on (the CNN needs it). a buffer that DMA writes and the CPU reads
 * through the cache can show stale data. the MPU makes SRAM2 (0x2007C000, 16 KB)
 * non-cacheable and the linker script puts the .dma_buffer section there, so:
 *
 *     static uint16_t adc_buf[60] DMA_BUF;      // correct, no cache maintenance needed
 *
 * rule: every buffer handed to a HAL_..._DMA() call uses DMA_BUF. budget: 16 KB total,
 * add your buffer to the table in ProjectDevGuide/docs/TEAM_GUIDE.md.
 */
#ifndef APP_DMA_BUF_H
#define APP_DMA_BUF_H

#include <stdbool.h>

#define DMA_BUF __attribute__((section(".dma_buffer"), aligned(32)))

#define DMA_REGION_BASE 0x2007C000u
#define DMA_REGION_SIZE 0x00004000u

/* checks at boot that the linker edit and the MPU region are really in place.
 * returns false (and app_init raises a fatal fault) if either is missing */
bool dma_buf_check(void);

#endif /* APP_DMA_BUF_H */
