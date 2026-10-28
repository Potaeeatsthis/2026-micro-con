/* kws_stream.h - continuous listening: audio in, log-mel frames out, for 2 microphones
 *
 *   SAI interrupt (every 10 ms):  kws_stream_push(ch0, ch1, 160)     160 new samples per channel
 *   main loop:                    kws_stream_poll()                   makes every frame that is ready
 *                                 kws_stream_choose_channel(&snr)     better mic for the last 1 s
 *                                 kws_stream_window(ch, input)        last 101 frames -> network input
 *
 * frame f uses samples [f*160, f*160+400) of the stream, so frames match the training
 * features (25 ms window, 10 ms hop). The window is the last 101 frames = 1.01 s.
 * Plain C, no HAL, so it can be tested on a PC.
 */
#pragma once
#include <stdint.h>
#include "kws_features.h"

#define KWS_N_CH         2
#define KWS_AUDIO_RING   4096   /* samples per channel kept (256 ms), power of 2 */
#define KWS_FEAT_RING    128    /* frames per channel kept (1.28 s), power of 2  */

/* gain_db: digital gain before the features (the INMP441s are quieter than the training data) */
void     kws_stream_init(float gain_db);

/* from the SAI DMA callback: n new samples per channel (int16, same scale as the WAV files) */
void     kws_stream_push(const int16_t *ch0, const int16_t *ch1, int n);

/* from the main loop: computes all frames that are ready, returns how many were made */
int      kws_stream_poll(void);

/* frames made so far (per channel) */
uint32_t kws_stream_frames(void);

/* picks the mic with the higher SNR over the last 101 frames (2 dB hysteresis) */
int      kws_stream_choose_channel(float *snr_db);

/* last 101 frames of channel ch, band-major: dst[band * 101 + frame] (network input order) */
void     kws_stream_window(int ch, float *dst);

/* loudest frame of the last 101 and the noise floor, as dB of the mean log-mel power */
float    kws_stream_peak_db(int ch);
float    kws_stream_noise_db(int ch);

/* times the main loop fell so far behind that audio was skipped */
uint32_t kws_stream_overruns(void);
