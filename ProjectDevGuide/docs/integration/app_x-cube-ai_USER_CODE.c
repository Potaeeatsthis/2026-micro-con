/* =====================================================================================
 * X-CUBE-AI/App/app_x-cube-ai.c of RoboticArmMain (Application Template, network "network")
 *
 * goal: this generated file only CREATES the network and offers 4 accessors.
 * all voice logic lives in App/voice/. these blocks survive every regeneration.
 *
 *   USER CODE includes : the header of the accessors
 *   USER CODE 2        : DELETE the two template functions (acquire_and_process_data,
 *                        post_process) and paste the 4 accessors instead
 *   USER CODE 5        : only ai_boostrap() (inside MX_X_CUBE_AI_Init)
 *   USER CODE 6        : EMPTY (inside MX_X_CUBE_AI_Process). the template's default body is
 *                        a do { } while loop that never returns and would freeze app_loop()
 *
 * check after pasting: no __io_putchar, no HAL_*Callback and no kws_classes.h / CLASS_NAMES in
 * this file (they live in App/common/app_log.c, App/app_callbacks.c and App/voice/voice.c).
 * the live project (model_realtime_code) had all of them here - do not copy its user code over.
 * ===================================================================================== */


/* ---------------------------------------------------------------- USER CODE BEGIN includes */
#include "voice/kws_model.h"
/* ---------------------------------------------------------------- USER CODE END includes */


/* ---------------------------------------------------------------- USER CODE BEGIN 2 */
bool kws_model_ready(void)
{
  return network != AI_HANDLE_NULL;
}

float *kws_model_input(void)
{
  return (float *)data_ins[0];
}

const float *kws_model_output(void)
{
  return (const float *)data_outs[0];
}

int kws_model_run(void)
{
  return ai_run();
}
/* ---------------------------------------------------------------- USER CODE END 2 */


/* ---------------------------------------------------------------- USER CODE BEGIN 5 */
  ai_boostrap(data_activations0);
/* ---------------------------------------------------------------- USER CODE END 5 */


/* ---------------------------------------------------------------- USER CODE BEGIN 6 */
/* ---------------------------------------------------------------- USER CODE END 6 */
