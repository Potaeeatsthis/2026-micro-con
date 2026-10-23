/* app_util.h - small helpers with no dependencies */
#ifndef APP_UTIL_H
#define APP_UTIL_H

#include <stdint.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

static inline float clampf(float v, float lo, float hi)
{
  return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static inline int32_t clampi(int32_t v, int32_t lo, int32_t hi)
{
  return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static inline float absf(float v) { return (v < 0.0f) ? -v : v; }

/* elapsed ms since t0, correct across the 49-day HAL_GetTick wrap */
static inline uint32_t ms_since(uint32_t now, uint32_t t0) { return now - t0; }

#endif /* APP_UTIL_H */
