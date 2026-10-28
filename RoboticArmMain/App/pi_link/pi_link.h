/* pi_link.h - text protocol with the raspberry pi on USART6 (115200 8N1)
 *
 *   STM32 -> Pi:  FIND red_box
 *   Pi -> STM32:  OBJ red_box 182 -45 30   (mm, robot frame)
 *   Pi -> STM32:  NOTFOUND red_box
 * Replies arrive as EVT_PI_OBJECT / EVT_PI_NOTFOUND / EVT_PI_TIMEOUT. The pi does the vision and
 * the camera -> robot transform, the stm32 does the IK.
 */
#ifndef APP_PI_LINK_H
#define APP_PI_LINK_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"
#include "pi_link/pi_proto.h"   /* pi_object_name(), pi_object_from_name() */

#define PI_UART huart6      /* the only place that names the uart */

void        pi_link_init(void);
void        pi_link_step(void);                 /* main loop: parse lines, timeouts, error restart */

void        pi_link_on_rx_event(uint16_t pos);  /* HAL_UARTEx_RxEventCallback (idle / half / full) */
void        pi_link_on_tx_done(void);
void        pi_link_on_error(void);

bool        pi_link_find(object_id_t obj);      /* sends FIND, starts the timeout. false if busy */
void        pi_link_cancel(void);               /* forget the pending request (mode change, stop) */
bool        pi_link_request_pending(void);
bool        pi_link_send_line(const char *line);/* raw line without '\n'; false if tx busy */

uint32_t    pi_link_last_rx_ms(void);           /* 0 = nothing received yet */
uint32_t    pi_link_bad_lines(void);            /* malformed / unknown / over-long lines since boot */

#endif /* APP_PI_LINK_H */
