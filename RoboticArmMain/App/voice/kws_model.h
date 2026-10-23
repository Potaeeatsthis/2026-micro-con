/* kws_model.h - thin access to the X-CUBE-AI network
 *
 * owner: voice (Supanut).
 * implemented in X-CUBE-AI/App/app_x-cube-ai.c inside its USER CODE blocks, because the
 * template keeps the network handle and buffers `static` there
 * (paste block: docs/integration/app_x-cube-ai_USER_CODE.c).
 * regenerating the .ioc keeps those blocks, so this interface survives model updates.
 */
#ifndef APP_KWS_MODEL_H
#define APP_KWS_MODEL_H

#include <stdbool.h>

bool         kws_model_ready(void);   /* network created by MX_X_CUBE_AI_Init() */
float       *kws_model_input(void);   /* AI_NETWORK_IN_1_SIZE floats */
const float *kws_model_output(void);  /* AI_NETWORK_OUT_1_SIZE floats (softmax probs) */
int          kws_model_run(void);     /* 0 = ok; ~43 ms (exp04), main loop only */

#endif /* APP_KWS_MODEL_H */
