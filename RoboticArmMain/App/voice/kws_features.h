/* kws_features.h - log-mel features on the STM32, identical to the training code
 *
 * Python (shared/features.py) does, for a 1 s clip at 16 kHz:
 *     librosa.feature.melspectrogram(y=x, sr=16000, n_fft=400, hop_length=160, win_length=400,
 *                                    n_mels=40, fmin=20, fmax=8000)       -> power mel, center=True
 *     np.log(S + 1e-6)                                                    -> (40, 101)
 *
 * This file does the same, one frame at a time:
 *     400 samples (25 ms) x periodic Hann window -> 400-point real FFT (KISS FFT)
 *     -> power |X|^2 of 201 bins -> 40 mel filters (librosa's, exported as a table) -> ln(. + 1e-6)
 *
 * Samples are floats in [-1, 1], like soundfile gives: int16 / 32768.
 * The tables come from make_kws_feature_data.py (kws_feature_tables.h).
 */
#pragma once
#include <stdint.h>

#define KWS_SR          16000
#define KWS_N_FFT       400     /* 25 ms window */
#define KWS_HOP         160     /* 10 ms hop */
#define KWS_N_BINS      201     /* N_FFT / 2 + 1 */
#define KWS_N_MELS      40
#define KWS_CLIP_LEN    16000   /* 1 s */
#define KWS_N_FRAMES    101     /* 1 + 16000 / 160, with librosa's center=True padding */

/* call once at start-up. returns 0 if OK, -1 if the FFT memory is too small */
int kws_features_init(void);

/* one frame: 400 samples in [-1, 1] -> 40 log-mel values */
void kws_logmel_frame(const float *samples, float *mel);

/* a whole 1 s clip exactly like librosa (center=True: 200 zeros added at each end)
 * -> out[band * 101 + frame], band-major, the order the network input wants */
void kws_logmel_clip_int16(const int16_t *clip, float *out);
