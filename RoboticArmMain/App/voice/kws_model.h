/* kws_model.h - access to the X-CUBE-AI network
 *
 * Implemented in X-CUBE-AI/App/app_x-cube-ai.c inside its USER CODE blocks (the network handle is
 * static there), so it survives regenerating the .ioc.
 * Paste block: ProjectDevGuide/docs/integration/app_x-cube-ai_USER_CODE.c
 */
#ifndef APP_KWS_MODEL_H
#define APP_KWS_MODEL_H

#include <stdbool.h>

bool         kws_model_ready(void);   /* network created by MX_X_CUBE_AI_Init() */
float       *kws_model_input(void);   /* AI_NETWORK_IN_1_SIZE floats */
const float *kws_model_output(void);  /* AI_NETWORK_OUT_1_SIZE floats (softmax probs) */
int          kws_model_run(void);     /* 0 = ok; ~43 ms, main loop only */

#endif /* APP_KWS_MODEL_H */
