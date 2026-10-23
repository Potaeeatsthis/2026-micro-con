/* feature.c - TEMPLATE, see feature.h */
#include "feature/feature.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "main.h"

/* module state: static, so nobody else can change it behind your back.
 * anything an isr writes and the main loop reads: volatile */
static bool s_ready;

void feature_init(void)
{
  /* start your peripheral here; on failure: LOG_ERR + app_fault_raise(FAULT_INIT, false) and return */
  s_ready = true;
  LOG("feature: init\r\n");
}

void feature_step(void)
{
  if (!s_ready) return;
  /* do a SMALL slice of work and return. long jobs are split over several passes */
}
