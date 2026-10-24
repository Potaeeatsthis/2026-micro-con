/* pi_proto.h - pi text protocol v1: line assembly and line parsing, no HAL
 *
 * owner: pi member.
 * pure functions only (no uart, no clock, no events), so the host tests can run them on a pc.
 * pi_link.c owns the uart, the pending request and the timeout; it feeds bytes in here and
 * acts on the parsed result.
 *
 * protocol v1 (guide section 12), one ASCII line per message, '\n' terminated, '\r' ignored,
 * fields separated by spaces:
 *   Pi -> STM32:  OBJ <name> <x> <y> <z>     integer mm, robot-base frame, top-face centre
 *   Pi -> STM32:  NOTFOUND <name>
 * names: blue_box, green_box, red_box, black_box (object_id_t order).
 *
 * new message: one row in PI_KEYWORDS (pi_proto.c), one value in pi_msg_type_t, one handler
 * row in RX_HANDLERS (pi_link.c), and the guide's protocol table.
 */
#ifndef APP_PI_PROTO_H
#define APP_PI_PROTO_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"

#define PI_LINE_MAX 80u     /* longest accepted line, without '\n' */

typedef enum {
  PI_MSG_NONE = 0,
  PI_MSG_OBJ,               /* obj, xyz[0..2] valid */
  PI_MSG_NOTFOUND           /* obj valid */
} pi_msg_type_t;

typedef enum {
  PI_OK = 0,
  PI_ERR_EMPTY,             /* blank line: callers ignore it silently */
  PI_ERR_UNKNOWN_KEYWORD,
  PI_ERR_FIELD_COUNT,
  PI_ERR_UNKNOWN_OBJECT,
  PI_ERR_BAD_INT,           /* not a whole base-10 integer, e.g. "12a", "-", "1.5" */
  PI_ERR_INT_RANGE,         /* outside int32 */
  PI_ERR_TOO_LONG           /* reported by pi_line_feed, never by pi_parse_line */
} pi_err_t;

typedef struct {
  pi_msg_type_t type;
  object_id_t   obj;
  int32_t       xyz[3];
} pi_msg_t;

/* splits line in place (writes '\0' between fields). out is cleared first */
pi_err_t    pi_parse_line(char *line, pi_msg_t *out);
const char *pi_err_str(pi_err_t err);

const char *pi_object_name(object_id_t obj);       /* "red_box", "none" if out of range */
object_id_t pi_object_from_name(const char *name);  /* OBJ_NONE if unknown */

/* line assembler: feed one received byte at a time */
typedef struct {
  char     buf[PI_LINE_MAX + 1u];
  uint32_t len;             /* chars in buf */
  uint32_t total;           /* chars seen in the current line, also past PI_LINE_MAX */
  bool     ended;           /* last feed ended a line: the next feed starts a new one */
} pi_line_t;

typedef enum {
  PI_LINE_NONE = 0,         /* keep feeding */
  PI_LINE_READY,            /* buf holds a '\0'-terminated line (may be empty) */
  PI_LINE_TOO_LONG          /* a line longer than PI_LINE_MAX ended; total = its length */
} pi_line_result_t;

void             pi_line_reset(pi_line_t *l);
/* the result refers to the line that just ended; l->buf / l->total stay valid until the next feed */
pi_line_result_t pi_line_feed(pi_line_t *l, char c);

#endif /* APP_PI_PROTO_H */
