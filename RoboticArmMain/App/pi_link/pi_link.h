/* pi_link.h - text protocol with the raspberry pi over USART6 (115200 8N1)
 *
 * owner: pi member.
 * protocol v1 (guide section 12), one ASCII line per message, '\n' terminated:
 *   STM32 -> Pi:  FIND red_box
 *   Pi -> STM32:  OBJ red_box 182 -45 30        (integer mm, robot-base frame, top-face centre)
 *   Pi -> STM32:  NOTFOUND red_box
 * results arrive as EVT_PI_OBJECT / EVT_PI_NOTFOUND / EVT_PI_TIMEOUT.
 * the pi does detection, depth and the camera -> robot-base transform; the stm32 does ik
 * and the grasp offset. this module does not move anything, it only posts events.
 * parsing lives in pi_proto.c (pure, host-tested); bad lines are logged with a reason.
 * APP_INPUT_TEST (local config only) adds a periodic FIND for bench tests, see pi_link.c.
 *
 * new message: one row in PI_KEYWORDS (pi_proto.c), one in RX_HANDLERS (pi_link.c) plus a
 * handler, and document it in the guide.
 */
#ifndef APP_PI_LINK_H
#define APP_PI_LINK_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"
#include "pi_link/pi_proto.h"   /* pi_object_name(), pi_object_from_name() */

#define PI_UART huart6      /* the only place that names the uart; huart3 = ST-Link usb for tests */

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
