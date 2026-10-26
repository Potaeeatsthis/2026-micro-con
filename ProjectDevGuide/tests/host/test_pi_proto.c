/* host test of the pi text protocol parser + line assembler (App/pi_link/pi_proto.c).
 * pure code, no HAL fakes needed. run: sh ProjectDevGuide/tests/host/run.sh */
#include <stdio.h>
#include <string.h>
#include "pi_link/pi_proto.h"

static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", msg, __LINE__); } } while (0)

static pi_msg_t g_msg;

/* parse a copy (the parser writes into its input) */
static pi_err_t parse(const char *s)
{
  char buf[256];
  strncpy(buf, s, sizeof buf - 1u);
  buf[sizeof buf - 1u] = '\0';
  return pi_parse_line(buf, &g_msg);
}

/* feed a string byte by byte; returns the last non-NONE result (or NONE) */
static pi_line_result_t feed(pi_line_t *l, const char *s, int *n_results)
{
  pi_line_result_t last = PI_LINE_NONE;
  *n_results = 0;
  for (const char *c = s; *c; c++) {
    pi_line_result_t r = pi_line_feed(l, *c);
    if (r != PI_LINE_NONE) { last = r; (*n_results)++; }
  }
  return last;
}

int main(void)
{
  printf("== pi_proto: valid lines\n");
  CHECK(parse("OBJ red_box 182 -45 30") == PI_OK, "OBJ parses");
  CHECK(g_msg.type == PI_MSG_OBJ && g_msg.obj == OBJ_RED_BOX, "OBJ type + object");
  CHECK(g_msg.xyz[0] == 182 && g_msg.xyz[1] == -45 && g_msg.xyz[2] == 30, "OBJ x y z incl. negative");
  CHECK(parse("OBJ blue_box -1 -2 -3") == PI_OK && g_msg.xyz[2] == -3 && g_msg.obj == OBJ_BLUE_BOX, "all negative");
  CHECK(parse("OBJ black_box 0 +12 -0") == PI_OK && g_msg.xyz[1] == 12 && g_msg.xyz[2] == 0, "+12 and -0 accepted");
  CHECK(parse("NOTFOUND green_box") == PI_OK && g_msg.type == PI_MSG_NOTFOUND && g_msg.obj == OBJ_GREEN_BOX, "NOTFOUND parses");
  CHECK(parse("  OBJ   red_box  1   2  3  ") == PI_OK && g_msg.xyz[2] == 3, "extra spaces tolerated");
  CHECK(parse("OBJ red_box 2147483647 -2147483648 0") == PI_OK && g_msg.xyz[0] == 2147483647 && g_msg.xyz[1] == (-2147483647 - 1), "int32 limits accepted");

  printf("== pi_proto: malformed lines\n");
  CHECK(parse("") == PI_ERR_EMPTY, "empty line");
  CHECK(parse("    ") == PI_ERR_EMPTY, "spaces only");
  CHECK(parse("hello") == PI_ERR_UNKNOWN_KEYWORD, "unknown keyword");
  CHECK(parse("obj red_box 1 2 3") == PI_ERR_UNKNOWN_KEYWORD, "keywords are case-sensitive");
  CHECK(parse("FIND red_box") == PI_ERR_UNKNOWN_KEYWORD, "FIND is stm32 -> pi only");
  CHECK(parse("OBJ red_box 1 2") == PI_ERR_FIELD_COUNT, "OBJ missing z");
  CHECK(parse("OBJ red_box") == PI_ERR_FIELD_COUNT, "OBJ missing x y z");
  CHECK(parse("OBJ") == PI_ERR_FIELD_COUNT, "OBJ alone");
  CHECK(parse("OBJ red_box 1 2 3 4") == PI_ERR_FIELD_COUNT, "OBJ extra field");
  CHECK(parse("OBJ red_box 1 2 3 4 5 6 7") == PI_ERR_FIELD_COUNT, "OBJ many extra fields");
  CHECK(parse("NOTFOUND") == PI_ERR_FIELD_COUNT, "NOTFOUND missing name");
  CHECK(parse("NOTFOUND red_box 1") == PI_ERR_FIELD_COUNT, "NOTFOUND extra field");
  CHECK(parse("OBJ red_block 1 2 3") == PI_ERR_UNKNOWN_OBJECT, "red_block is not a v1 name");
  CHECK(parse("NOTFOUND red") == PI_ERR_UNKNOWN_OBJECT, "pipeline colour name is not a v1 name");
  CHECK(parse("OBJ red_box x y z") == PI_ERR_BAD_INT, "letters");
  CHECK(parse("OBJ red_box 12a 2 3") == PI_ERR_BAD_INT, "trailing letter");
  CHECK(parse("OBJ red_box 1.5 2 3") == PI_ERR_BAD_INT, "decimal point");
  CHECK(parse("OBJ red_box - 2 3") == PI_ERR_BAD_INT, "lone minus");
  CHECK(parse("OBJ red_box 1 2 0x10") == PI_ERR_BAD_INT, "hex not accepted");
  CHECK(parse("OBJ red_box 2147483648 2 3") == PI_ERR_INT_RANGE, "int32 max + 1");
  CHECK(parse("OBJ red_box 1 -2147483649 3") == PI_ERR_INT_RANGE, "int32 min - 1");
  CHECK(parse("OBJ red_box 1 2 99999999999999999999") == PI_ERR_INT_RANGE, "way out of range");
  CHECK(parse("OBJ red_box 1 2 3") == PI_OK && g_msg.type == PI_MSG_OBJ, "valid again after errors");
  CHECK(parse("OBJ red_box 1 2 x") != PI_OK && g_msg.type == PI_MSG_NONE && g_msg.obj == OBJ_NONE, "failed parse leaves out cleared");
  CHECK(strcmp(pi_err_str(PI_ERR_FIELD_COUNT), "wrong field count") == 0, "reason text");

  printf("== pi_proto: object names\n");
  CHECK(strcmp(pi_object_name(OBJ_RED_BOX), "red_box") == 0, "name of red_box");
  CHECK(strcmp(pi_object_name(OBJ_NONE), "none") == 0, "name of OBJ_NONE");
  CHECK(pi_object_from_name("black_box") == OBJ_BLACK_BOX, "black_box from name");
  CHECK(pi_object_from_name("purple_box") == OBJ_NONE, "unknown name");
  CHECK(strcmp(pi_object_name(OBJ_HAND), "hand") == 0, "name of hand");
  CHECK(pi_object_from_name("hand") == OBJ_HAND, "hand from name");
  CHECK(pi_object_from_name("Hand") == OBJ_NONE, "names are case-sensitive");
  CHECK(OBJ_BOX_COUNT == 4 && OBJ_HAND == OBJ_BOX_COUNT && OBJ_COUNT == 5, "4 boxes, then hand");
  CHECK(parse("OBJ hand 101 -4 279") == PI_OK && g_msg.obj == OBJ_HAND && g_msg.xyz[2] == 279, "OBJ hand parses");
  CHECK(parse("NOTFOUND hand") == PI_OK && g_msg.type == PI_MSG_NOTFOUND && g_msg.obj == OBJ_HAND, "NOTFOUND hand parses");

  printf("== pi_proto: line assembler\n");
  pi_line_t l;
  int n;
  pi_line_reset(&l);
  CHECK(feed(&l, "OBJ red_box 1 2 3\r\n", &n) == PI_LINE_READY && n == 1, "CRLF line ready once");
  CHECK(strcmp(l.buf, "OBJ red_box 1 2 3") == 0, "\\r stripped");
  CHECK(feed(&l, "NOTFOUND re", &n) == PI_LINE_NONE && n == 0, "partial line waits");
  CHECK(feed(&l, "d_box\n", &n) == PI_LINE_READY && strcmp(l.buf, "NOTFOUND red_box") == 0, "line split over two chunks");
  CHECK(feed(&l, "\n", &n) == PI_LINE_READY && l.buf[0] == '\0', "blank line is an empty READY");

  char exact[PI_LINE_MAX + 2u];
  memset(exact, 'A', PI_LINE_MAX);
  exact[PI_LINE_MAX] = '\n';
  exact[PI_LINE_MAX + 1u] = '\0';
  CHECK(feed(&l, exact, &n) == PI_LINE_READY && strlen(l.buf) == PI_LINE_MAX, "exactly PI_LINE_MAX chars accepted");

  char longl[202];
  memset(longl, 'A', 200);
  longl[200] = '\n';
  longl[201] = '\0';
  CHECK(feed(&l, longl, &n) == PI_LINE_TOO_LONG && n == 1, "200-char line -> TOO_LONG once");
  CHECK(l.total == 200u, "TOO_LONG reports the real length");
  CHECK(feed(&l, "OBJ red_box 4 5 6\n", &n) == PI_LINE_READY && strcmp(l.buf, "OBJ red_box 4 5 6") == 0,
        "next line after an over-long one is clean");
  exact[PI_LINE_MAX] = 'B';                /* 81 chars, then newline */
  char l81[PI_LINE_MAX + 3u];
  memcpy(l81, exact, PI_LINE_MAX + 1u);
  l81[PI_LINE_MAX + 1u] = '\n';
  l81[PI_LINE_MAX + 2u] = '\0';
  CHECK(feed(&l, l81, &n) == PI_LINE_TOO_LONG, "PI_LINE_MAX + 1 chars -> TOO_LONG");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
