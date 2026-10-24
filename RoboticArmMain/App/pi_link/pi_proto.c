/* pi_proto.c - see pi_proto.h. no HAL, no globals except const tables */
#include "pi_link/pi_proto.h"
#include "common/app_util.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOK 6           /* one more than the longest message, so extra fields are seen */

static const char *const OBJ_NAMES[OBJ_COUNT] = { "blue_box", "green_box", "red_box", "black_box" };

typedef struct {
  const char   *keyword;
  pi_msg_type_t type;
  int           n_tok;      /* fields including the keyword */
} pi_keyword_t;

/* EXTENSION POINT: new pi -> stm32 messages, one row each */
static const pi_keyword_t PI_KEYWORDS[] = {
  { "OBJ",      PI_MSG_OBJ,      5 },
  { "NOTFOUND", PI_MSG_NOTFOUND, 2 },
};

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

/* whole token must be a base-10 integer. errno is the only overflow check that works when
 * long is 32 bit (arm): there strtol clamps to LONG_MAX and a > INT32_MAX test never fires */
static pi_err_t parse_int(const char *s, int32_t *out)
{
  char *end = NULL;
  errno = 0;
  long v = strtol(s, &end, 10);
  if (end == s || *end != '\0') return PI_ERR_BAD_INT;
  if (errno == ERANGE) return PI_ERR_INT_RANGE;
#if LONG_MAX > INT32_MAX
  if (v > INT32_MAX || v < INT32_MIN) return PI_ERR_INT_RANGE;
#endif
  *out = (int32_t)v;
  return PI_OK;
}

pi_err_t pi_parse_line(char *line, pi_msg_t *out)
{
  memset(out, 0, sizeof *out);
  out->obj = OBJ_NONE;

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
  if (n == 0) return PI_ERR_EMPTY;

  const pi_keyword_t *kw = NULL;
  for (uint32_t i = 0; i < ARRAY_LEN(PI_KEYWORDS); i++) {
    if (strcmp(tok[0], PI_KEYWORDS[i].keyword) == 0) kw = &PI_KEYWORDS[i];
  }
  if (kw == NULL) return PI_ERR_UNKNOWN_KEYWORD;
  if (n != kw->n_tok) return PI_ERR_FIELD_COUNT;   /* n == MAX_TOK also means "more than that" */

  object_id_t obj = pi_object_from_name(tok[1]);
  if (obj == OBJ_NONE) return PI_ERR_UNKNOWN_OBJECT;

  int32_t xyz[3] = { 0, 0, 0 };
  if (kw->type == PI_MSG_OBJ) {
    for (int i = 0; i < 3; i++) {
      pi_err_t e = parse_int(tok[2 + i], &xyz[i]);
      if (e != PI_OK) return e;
    }
  }
  out->type = kw->type;
  out->obj = obj;
  memcpy(out->xyz, xyz, sizeof xyz);
  return PI_OK;
}

const char *pi_err_str(pi_err_t err)
{
  switch (err) {
    case PI_OK:                  return "ok";
    case PI_ERR_EMPTY:           return "empty line";
    case PI_ERR_UNKNOWN_KEYWORD: return "unknown keyword";
    case PI_ERR_FIELD_COUNT:     return "wrong field count";
    case PI_ERR_UNKNOWN_OBJECT:  return "unknown object";
    case PI_ERR_BAD_INT:         return "not an integer";
    case PI_ERR_INT_RANGE:       return "integer out of range";
    case PI_ERR_TOO_LONG:        return "line too long";
  }
  return "?";
}

/* ---------------------------------------------------------------- line assembler */
void pi_line_reset(pi_line_t *l)
{
  l->len = 0u;
  l->total = 0u;
  l->ended = false;
  l->buf[0] = '\0';
}

pi_line_result_t pi_line_feed(pi_line_t *l, char c)
{
  if (l->ended) pi_line_reset(l);
  if (c == '\r') return PI_LINE_NONE;
  if (c == '\n') {
    l->ended = true;
    l->buf[l->len] = '\0';
    return (l->total > PI_LINE_MAX) ? PI_LINE_TOO_LONG : PI_LINE_READY;
  }
  l->total++;
  if (l->len < PI_LINE_MAX) l->buf[l->len++] = c;  /* the rest of an over-long line is dropped */
  return PI_LINE_NONE;
}
