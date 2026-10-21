# Voice command models for the STM32F767ZI (X-CUBE-AI)

Released keyword-spotting models only, one folder each. Made by `workspace/export_model.py`
from `workspace/experiments/<experiment>/runs/final.pth` (weights themselves are not in git).

| Folder | Model | Classes | Status |
|---|---|---|---|
| `exp04_tcresnet-w16d3/` | TC-ResNet width 16, depth 3, 56,559 params | 23 | candidate, not yet tested on the board |

## Files
| File | What |
|---|---|
| `onnx_out/kws.onnx` | model for X-CUBE-AI. Input `logmel` 1 x 40 x 101 **raw** log-mel (normalisation is inside), output `probs` 1 x 23 (softmax inside) |
| `onnx_out/kws_test_vector.h` | one test input in both memory orders + the expected 23 outputs (checks the input order X-CUBE-AI uses) |
| `kws_c/kws_clip_test.h` | power-up self-test: a 1 s clip, its log-mel, and this model's 23 probabilities |
| `kws_c/kws_classes.h` | `N_CLASSES`, `CLS_UNKNOWN`, `CLS_SILENCE`, `CLASS_NAMES` (from `config.py`) |

The feature code (`kws_features.c/.h`, `kws_stream.c/.h`, `kws_feature_tables.h`, kissfft) is
unchanged from the deployed 22-class model.

## Classes (index order)
move, pick, grab, blue box, green box, red box, black box, hand, station, base, forward, backward,
left, right, place, drop, yes, no, home, stop, undo, `_unknown_` (21), `_silence_` (22).
`_unknown_` and `_silence_` never trigger an action.

## Using it in the firmware
1. X-CUBE-AI: load `onnx_out/kws.onnx`, Analyze, generate.
2. Add `kws_c/kws_clip_test.h` and `kws_c/kws_classes.h` to the include folder.
3. In `app_x-cube-ai.c`, replace the hard-coded `N_CLASSES` / `CLS_UNKNOWN` / `CLS_SILENCE` /
   `CLASS_NAMES` with `#include "kws_classes.h"`.
4. Suggested stream settings: average of 3, threshold 0.75, refractory 1500 ms, stride 10 frames.

## exp04 results on board recordings (PC simulation of the firmware, gain +15 dB)
From `workspace/experiments/leaderboard.csv` and `.../exp04_tcresnet-w16d3_aug-all/results/threshold_grid.csv`.

| Board session | Command accuracy (1 s clips) | Stream: commands detected (thr 0.75) | Wrong commands | Unknown words that triggered |
|---|---|---|---|---|
| S1 (one speaker, 200 commands, 30 unknown) | 0.990 | 184 / 200 | 0 | 15 / 30 |
| S0 (old mic wiring, 94 commands, 15 unknown) | 0.968 | 30 / 94 | 0 | 6 / 15 |

Limits: one speaker (the person who trained it was also recorded in S1), no `undo` recordings yet,
and rhyming non-commands (nose, foam, bright …) still often trigger a command.
