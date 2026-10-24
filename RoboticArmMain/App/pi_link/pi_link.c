/* pi_link.c
 *
 * rx: USART6 -> DMA circular ring (rx_ring). HAL_UARTEx_ReceiveToIdle_DMA reports the
 * write position on idle line, half and full; the main loop reads from s_tail up to it.
 * the ring holds ~89 ms of continuous traffic at 115200 baud; pi messages are short and
 * rare, so a main-loop pause (CNN 33 ms) cannot overflow it in practice.
 * line assembly and parsing are in pi_proto.c (pure, host-tested); this file owns the
 * uart, the pending request, the timeout, events and logging.
 *
 * a reply is accepted only for the request that is pending; a late reply after a cancel
 * or timeout is logged and ignored, so an old position can never start a motion.
 * every bad line is logged once with its reason (over-long lines too); line logs are
 * rate-limited so a noisy rx pin cannot flood the debug uart.
 *
 * APP_INPUT_TEST (app_config_local.h only): sends FIND <APP_INPUT_TEST_OBJ> every
 * APP_INPUT_TEST_FIND_MS and prints link counters once a second when they change.
 * needs APP_USE_MODES 0 (the voice mode would also send FIND) and APP_USE_MOTION 0.
 */
#include "pi_link/pi_link.h"
#include "pi_link/pi_proto.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "common/dma_buf.h"
#include "main.h"
#include "usart.h"
#include <string.h>

#ifndef APP_INPUT_TEST
#define APP_INPUT_TEST 0
#endif
#ifndef APP_INPUT_TEST_FIND_MS
#define APP_INPUT_TEST_FIND_MS 5000u
#endif
#ifndef APP_INPUT_TEST_OBJ
#define APP_INPUT_TEST_OBJ OBJ_RED_BOX
#endif
#if APP_INPUT_TEST && (APP_USE_MODES || APP_USE_MOTION)
#error "APP_INPUT_TEST needs APP_USE_MODES 0 and APP_USE_MOTION 0 (no motion, test sends its own FIND)"
#endif

#define RX_LEN   1024u
#define TX_LEN   96u
#define LOG_SHOW_CHARS     40u     /* a bad line is logged up to this many chars */
#define LOG_LINES_PER_SEC  10u     /* line logs (bad / ignored) per second, the rest are counted */

static uint8_t s_rx[RX_LEN] DMA_BUF;
static uint8_t s_tx[TX_LEN] DMA_BUF;

static volatile uint32_t s_head;          /* dma write position, from the isr */
static uint32_t          s_tail;          /* main-loop read position */
static volatile bool     s_tx_busy;
static volatile bool     s_need_restart;

static pi_line_t s_line;

static object_id_t s_pending = OBJ_NONE;
static uint32_t    s_pending_ms;
static uint32_t    s_last_rx_ms;

/* counters (main loop only) */
static uint32_t s_rx_bytes;
static uint32_t s_ok_lines;
static uint32_t s_bad_lines;
static uint32_t s_timeouts;
static uint32_t s_finds;

/* line-log rate limit */
static uint32_t s_log_win_ms;
static uint32_t s_log_count;
static uint32_t s_log_dropped;

/* ---------------------------------------------------------------- helpers */
static bool start_rx(void)
{
  s_head = 0u;
  s_tail = 0u;
  pi_line_reset(&s_line);
  return HAL_UARTEx_ReceiveToIdle_DMA(&PI_UART, s_rx, RX_LEN) == HAL_OK;
}

static bool line_log_allowed(void)
{
  uint32_t now = HAL_GetTick();
  if (ms_since(now, s_log_win_ms) >= 1000u) {
    if (s_log_dropped != 0u) {
      LOG("pi: %lu line logs suppressed\r\n", (unsigned long)s_log_dropped);
    }
    s_log_win_ms = now;
    s_log_count = 0u;
    s_log_dropped = 0u;
  }
  if (s_log_count < LOG_LINES_PER_SEC) {
    s_log_count++;
    return true;
  }
  s_log_dropped++;
  return false;
}

/* printable copy of the start of a line for the log: control bytes become '.' */
static void log_safe_copy(char *dst, const char *src)
{
  uint32_t i = 0u;
  for (; i < LOG_SHOW_CHARS && src[i] != '\0'; i++) {
    char c = src[i];
    dst[i] = (c >= 0x20 && c < 0x7F) ? c : '.';
  }
  dst[i] = '\0';
}

static void bad_line(pi_err_t err, const char *shown, uint32_t len)
{
  s_bad_lines++;
  if (!line_log_allowed()) return;
  LOG("pi: bad line (%s, %lu chars): '%s%s'\r\n", pi_err_str(err), (unsigned long)len, shown,
      (len > LOG_SHOW_CHARS) ? "..." : "");
}

/* ---------------------------------------------------------------- message handlers
 * one function per message type. the line is already parsed and valid */
static void rx_obj(const pi_msg_t *m)
{
  const char *name = pi_object_name(m->obj);
  if (m->obj != s_pending) {
    if (line_log_allowed()) LOG("pi: ignored OBJ %s (not the pending request)\r\n", name);
    return;
  }
  uint32_t rtt = ms_since(HAL_GetTick(), s_pending_ms);
  s_pending = OBJ_NONE;
  (void)app_event_post(EVT_PI_OBJECT, (uint16_t)m->obj, m->xyz[0], m->xyz[1], m->xyz[2]);
  LOG("pi: OBJ %s %ld %ld %ld mm, rtt %lu ms\r\n", name, (long)m->xyz[0], (long)m->xyz[1],
      (long)m->xyz[2], (unsigned long)rtt);
}

static void rx_notfound(const pi_msg_t *m)
{
  const char *name = pi_object_name(m->obj);
  if (m->obj != s_pending) {
    if (line_log_allowed()) LOG("pi: ignored NOTFOUND %s (not the pending request)\r\n", name);
    return;
  }
  uint32_t rtt = ms_since(HAL_GetTick(), s_pending_ms);
  s_pending = OBJ_NONE;
  (void)app_event_post(EVT_PI_NOTFOUND, (uint16_t)m->obj, 0, 0, 0);
  LOG("pi: NOTFOUND %s, rtt %lu ms\r\n", name, (unsigned long)rtt);
}

typedef struct {
  pi_msg_type_t type;
  void (*handler)(const pi_msg_t *m);
} rx_handler_t;

/* EXTENSION POINT: new pi -> stm32 messages, one row each (plus PI_KEYWORDS in pi_proto.c) */
static const rx_handler_t RX_HANDLERS[] = {
  { PI_MSG_OBJ,      rx_obj },
  { PI_MSG_NOTFOUND, rx_notfound },
};

static void handle_line(char *line, uint32_t len)
{
  char shown[LOG_SHOW_CHARS + 1u];
  log_safe_copy(shown, line);                    /* before the parser splits the line */

  pi_msg_t m;
  pi_err_t err = pi_parse_line(line, &m);
  if (err == PI_ERR_EMPTY) return;               /* blank lines are not errors */
  if (err != PI_OK) {
    bad_line(err, shown, len);
    return;
  }
  s_ok_lines++;
  for (uint32_t i = 0; i < ARRAY_LEN(RX_HANDLERS); i++) {
    if (RX_HANDLERS[i].type == m.type) {
      RX_HANDLERS[i].handler(&m);
      return;
    }
  }
}

#if APP_INPUT_TEST
static uint32_t s_test_find_ms;
static uint32_t s_test_stat_ms;
static uint32_t s_test_stat_sum;

/* periodic FIND + 1 Hz counters (only printed when something changed) */
static void test_step(void)
{
  uint32_t now = HAL_GetTick();
  if (s_pending == OBJ_NONE && ms_since(now, s_test_find_ms) >= APP_INPUT_TEST_FIND_MS) {
    s_test_find_ms = now;
    if (!pi_link_find(APP_INPUT_TEST_OBJ)) LOG("pi: test FIND not sent (tx busy)\r\n");
  }
  if (ms_since(now, s_test_stat_ms) >= 1000u) {
    s_test_stat_ms = now;
    uint32_t sum = s_rx_bytes + s_ok_lines + s_bad_lines + s_timeouts + s_finds;
    if (sum != s_test_stat_sum) {
      s_test_stat_sum = sum;
      LOG("pi: stat rx %lu B, ok %lu, bad %lu, find %lu, timeout %lu\r\n", (unsigned long)s_rx_bytes,
          (unsigned long)s_ok_lines, (unsigned long)s_bad_lines, (unsigned long)s_finds,
          (unsigned long)s_timeouts);
    }
  }
}
#endif

/* ---------------------------------------------------------------- api */
void pi_link_init(void)
{
  if (!start_rx()) {
    LOG_ERR("pi: usart6 rx dma start failed\r\n");
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  LOG("pi: link on USART6, timeout %lu ms\r\n", (unsigned long)APP_PI_TIMEOUT_MS);
#if APP_INPUT_TEST
  LOG("pi: TEST mode, FIND %s every %lu ms\r\n", pi_object_name(APP_INPUT_TEST_OBJ),
      (unsigned long)APP_INPUT_TEST_FIND_MS);
  s_test_find_ms = HAL_GetTick() - APP_INPUT_TEST_FIND_MS + 1000u;   /* first FIND 1 s after boot */
#endif
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
    s_rx_bytes++;
    pi_line_result_t r = pi_line_feed(&s_line, c);
    if (r == PI_LINE_READY) {
      handle_line(s_line.buf, s_line.total);
    } else if (r == PI_LINE_TOO_LONG) {
      char shown[LOG_SHOW_CHARS + 1u];
      log_safe_copy(shown, s_line.buf);
      bad_line(PI_ERR_TOO_LONG, shown, s_line.total);
    }
  }

  if (s_pending != OBJ_NONE && ms_since(HAL_GetTick(), s_pending_ms) > APP_PI_TIMEOUT_MS) {
    LOG("pi: no answer for %s within %lu ms\r\n", pi_object_name(s_pending),
        (unsigned long)APP_PI_TIMEOUT_MS);
    s_pending = OBJ_NONE;
    s_timeouts++;
    (void)app_event_post0(EVT_PI_TIMEOUT);
  }

#if APP_INPUT_TEST
  test_step();
#endif
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
  strcat(msg, pi_object_name(obj));
  if (!pi_link_send_line(msg)) return false;
  s_pending = obj;
  s_pending_ms = HAL_GetTick();
  s_finds++;
  LOG("pi: sent %s\r\n", msg);
  return true;
}

void     pi_link_cancel(void)           { s_pending = OBJ_NONE; }
bool     pi_link_request_pending(void)  { return s_pending != OBJ_NONE; }
uint32_t pi_link_last_rx_ms(void)       { return s_last_rx_ms; }
uint32_t pi_link_bad_lines(void)        { return s_bad_lines; }
