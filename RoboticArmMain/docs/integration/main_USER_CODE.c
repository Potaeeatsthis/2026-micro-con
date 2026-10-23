/* =====================================================================================
 * Core/Src/main.c of RoboticArmMain: paste each block into the matching USER CODE section.
 * these three lines are ALL the user code main.c ever gets. everything else lives in App/.
 *
 * what CubeMX generates around them (do not touch):
 *   MX_..._Init() for every peripheral, then MX_X_CUBE_AI_Init()   <- network is created here
 *   inside while(1): MX_X_CUBE_AI_Process()                         <- empty on purpose (USER CODE 6)
 * ===================================================================================== */


/* ---------------------------------------------------------------- USER CODE BEGIN Includes */
#include "app.h"
/* ---------------------------------------------------------------- USER CODE END Includes */


/* ---------------------------------------------------------------- USER CODE BEGIN 2 */
  app_init();
/* ---------------------------------------------------------------- USER CODE END 2 */


/* ---------------------------------------------------------------- USER CODE BEGIN 3
 * careful: CubeMX puts the closing brace of while(1) INSIDE this block. keep it:  */
    app_loop();
  }
/* ---------------------------------------------------------------- USER CODE END 3 */
