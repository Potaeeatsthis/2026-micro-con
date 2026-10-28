/* pi_proto.h - pi text protocol: line assembly and parsing (pure, host-tested)
 *
 * One ASCII line per message, '\n' terminated, '\r' ignored, fields split by spaces:
 *   OBJ <name> <x> <y> <z>   integer mm, robot frame
 *   NOTFOUND <name>
 * New message: PI_KEYWORDS (pi_proto.c), pi_msg_type_t, RX_HANDLERS (pi_link.c), the guide table.
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

/* splits the line in place. out is cleared first */
pi_err_t    pi_parse_line(char *line, pi_msg_t *out);
const char *pi_err_str(pi_err_t err);

const char *pi_object_name(object_id_t obj);       /* "red_box", "none" if out of range */
object_id_t pi_object_from_name(const char *name);  /* OBJ_NONE if unknown */

/* line assembler, one byte at a time */
typedef struct {
  char     buf[PI_LINE_MAX + 1u];
  uint32_t len;             /* chars in buf */
  uint32_t total;           /* also counts past PI_LINE_MAX */
  bool     ended;
} pi_line_t;

typedef enum {
  PI_LINE_NONE = 0,         /* keep feeding */
  PI_LINE_READY,            /* buf holds a '\0'-terminated line (may be empty) */
  PI_LINE_TOO_LONG          /* a line longer than PI_LINE_MAX ended; total = its length */
} pi_line_result_t;

void             pi_line_reset(pi_line_t *l);
/* l->buf / l->total stay valid until the next feed */
pi_line_result_t pi_line_feed(pi_line_t *l, char c);

#endif /* APP_PI_PROTO_H */
