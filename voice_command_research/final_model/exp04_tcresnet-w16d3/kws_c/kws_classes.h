/* kws_classes.h - made by export_model.py from voice_command_research\config.py, do not edit
 * model: exp04_tcresnet-w16d3_aug-all. Replaces N_CLASSES / CLS_UNKNOWN / CLS_SILENCE / CLASS_NAMES in app_x-cube-ai.c
 */
#pragma once

#define N_CLASSES   23
#define CLS_UNKNOWN 21
#define CLS_SILENCE 22

static const char *const CLASS_NAMES[N_CLASSES] = {
  "move", "pick", "grab", "blue box", "green box", "red box", "black box", "hand", "station", "base", "forward", "backward", "left", "right", "place", "drop", "yes", "no", "home", "stop", "undo", "_unknown_", "_silence_"
};
