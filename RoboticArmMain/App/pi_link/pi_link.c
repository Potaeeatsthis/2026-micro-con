/* pi_link.c
 *
 * rx: USART6 -> DMA circular ring (rx_ring). HAL_UARTEx_ReceiveToIdle_DMA reports the
 * write position on idle line, half and full; the main loop reads from s_tail up to it.
 * the ring holds ~89 ms of continuous traffic at 115200 baud; pi messages are short and
 * rare, so a main-loop pause (CNN 33 ms) cannot overflow it in practice.
 *
 * a reply is accepted only for the request that is pending; a late reply after a cancel
 * or timeout is logged and ignored, so an old position can never start a motion.
 */
#include "pi_link/pi_link.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "common/dma_buf.h"
#include "main.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>

#define RX_LEN   1024u
#define TX_LEN   96u
#define LINE_MAX 80u
#define MAX_TOK  6

static uint8_t s_rx[RX_LEN] DMA_BUF;
static uint8_t s_tx[TX_LEN] DMA_BUF;

static volatile uint32_t s_head;          /* dma write position, from the isr */
static uint32_t          s_tail;          /* main-loop read position */
static volatile bool     s_tx_busy;
static volatile bool     s_need_restart;

static char     s_line[LINE_MAX + 1u];
static uint32_t s_line_len;
static bool     s_line_overflow;

static object_id_t s_pending = OBJ_NONE;
static uint32_t    s_pending_ms;
static uint32_t    s_last_rx_ms;
static uint32_t    s_bad_lines;

static const char *const OBJ_NAMES[OBJ_COUNT] = { "blue_box", "green_box", "red_box", "black_box" };

/* ---------------------------------------------------------------- helpers */
const char *pi_object_name(object_id_t obj)
{
  return (obj < OBJ_COUNT) ? OBJ_NAMES[obj] : "none";
}

object_id_t pi_object_from_name(const char *name)
{
  for (int i = 0; i < (int)OBJ_COUNT; i++) {
    if (strcmp(OBJ_NAMES[i], name) == 0) return (object_id_t)i;
  }
  return OBJ_NONE;
}

static bool parse_int(const char *s, int32_t *out)
{
  char *end = NULL;
  long v = strtol(s, &end, 10);
  if (end == s || *end != '\0') return false;
  *out = (int32_t)v;
  return true;
}

static bool start_rx(void)
{
  s_head = 0u;
  s_tail = 0u;
  s_line_len = 0u;
  return HAL_UARTEx_ReceiveToIdle_DMA(&PI_UART, s_rx, RX_LEN) == HAL_OK;
}

/* ---------------------------------------------------------------- message handlers
 * one function per message keyword. tok[0] is the keyword. return false = malformed */
static bool rx_obj(char **tok, int n)
{
  if (n != 5) return false;
  object_id_t obj = pi_object_from_name(tok[1]);
  int32_t x, y, z;
  if (obj == OBJ_NONE || !parse_int(tok[2], &x) || !parse_int(tok[3], &y) || !parse_int(tok[4], &z)) {
    return false;
  }
  if (obj != s_pending) {
    LOG("pi: ignored OBJ %s (not the pending request)\r\n", tok[1]);
    return true;
  }
  s_pending = OBJ_NONE;
  (void)app_event_post(EVT_PI_OBJECT, (uint16_t)obj, x, y, z);
  LOG("pi: OBJ %s %ld %ld %ld mm\r\n", tok[1], (long)x, (long)y, (long)z);
  return true;
}

static bool rx_notfound(char **tok, int n)
{
  if (n != 2) return false;
  object_id_t obj = pi_object_from_name(tok[1]);
  if (obj == OBJ_NONE) return false;
  if (obj != s_pending) {
    LOG("pi: ignored NOTFOUND %s (not the pending request)\r\n", tok[1]);
    return true;
  }
  s_pending = OBJ_NONE;
  (void)app_event_post(EVT_PI_NOTFOUND, (uint16_t)obj, 0, 0, 0);
  LOG("pi: NOTFOUND %s\r\n", tok[1]);
  return true;
}

typedef struct {
  const char *keyword;
  bool (*handler)(char **tok, int n);
} rx_handler_t;

/* EXTENSION POINT: new pi -> stm32 messages, one row each */
static const rx_handler_t RX_HANDLERS[] = {
  { "OBJ",      rx_obj },
  { "NOTFOUND", rx_notfound },
};

static void handle_line(char *line)
{
  char *tok[MAX_TOK];
  int n = 0;
  char *p = line;
  while (*p != '\0' && n < MAX_TOK) {
    while (*p == ' ') p++;
    if (*p == '\0') break;
    tok[n++] = p;
    while (*p != '\0' && *p != ' ') p++;
    if (*p == ' ') *p++ = '\0';
  }
  if (n == 0) return;

  for (uint32_t i = 0; i < ARRAY_LEN(RX_HANDLERS); i++) {
    if (strcmp(tok[0], RX_HANDLERS[i].keyword) == 0) {
      if (!RX_HANDLERS[i].handler(tok, n)) {
        s_bad_lines++;
        LOG("pi: malformed %s line\r\n", tok[0]);
      }
      return;
    }
  }
  s_bad_lines++;
  LOG("pi: unknown message '%s'\r\n", tok[0]);
}

/* ---------------------------------------------------------------- api */
void pi_link_init(void)
{
  if (!start_rx()) {
    LOG_ERR("pi: usart6 rx dma start failed\r\n");
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  LOG("pi: link on USART6, timeout %lu ms\r\n", (unsigned long)APP_PI_TIMEOUT_MS);
}

void pi_link_on_rx_event(uint16_t pos)
{
  s_head = (pos >= RX_LEN) ? 0u : pos;
}

void pi_link_on_tx_done(void) { s_tx_busy = false; }

void pi_link_on_error(void)
{
  s_need_restart = true;
  app_fault_raise(FAULT_UART_PI, false);
}

void pi_link_step(void)
{
  if (s_need_restart) {
    s_need_restart = false;
    (void)HAL_UART_AbortReceive(&PI_UART);
    s_tx_busy = false;
    if (!start_rx()) LOG_ERR("pi: rx restart failed\r\n");
  }

  uint32_t head = s_head;
  while (s_tail != head) {
    char c = (char)s_rx[s_tail];
    s_tail = (s_tail + 1u) % RX_LEN;
    s_last_rx_ms = HAL_GetTick();
    if (c == '\r') continue;
    if (c == '\n') {
      if (!s_line_overflow) {
        s_line[s_line_len] = '\0';
        handle_line(s_line);
      }
      s_line_len = 0u;
      s_line_overflow = false;
      continue;
    }
    if (s_line_len < LINE_MAX) {
      s_line[s_line_len++] = c;
    } else {
      s_line_overflow = true;                    /* drop the rest of an over-long line */
    }
  }

  if (s_pending != OBJ_NONE && ms_since(HAL_GetTick(), s_pending_ms) > APP_PI_TIMEOUT_MS) {
    LOG("pi: no answer for %s within %lu ms\r\n", pi_object_name(s_pending),
        (unsigned long)APP_PI_TIMEOUT_MS);
    s_pending = OBJ_NONE;
    (void)app_event_post0(EVT_PI_TIMEOUT);
  }
}

bool pi_link_send_line(const char *line)
{
  if (s_tx_busy) return false;
  size_t n = strlen(line);
  if (n + 1u > TX_LEN) return false;
  memcpy(s_tx, line, n);
  s_tx[n] = '\n';
  s_tx_busy = true;
  if (HAL_UART_Transmit_DMA(&PI_UART, s_tx, (uint16_t)(n + 1u)) != HAL_OK) {
    s_tx_busy = false;
    return false;
  }
  return true;
}

bool pi_link_find(object_id_t obj)
{
  if (obj >= OBJ_COUNT || s_pending != OBJ_NONE) return false;
  char msg[32];
  strcpy(msg, "FIND ");
  strcat(msg, OBJ_NAMES[obj]);
  if (!pi_link_send_line(msg)) return false;
  s_pending = obj;
  s_pending_ms = HAL_GetTick();
  return true;
}

void     pi_link_cancel(void)           { s_pending = OBJ_NONE; }
bool     pi_link_request_pending(void)  { return s_pending != OBJ_NONE; }
uint32_t pi_link_last_rx_ms(void)       { return s_last_rx_ms; }
