/* kws_features.c - see kws_features.h
 *
 * no HAL includes here on purpose: kiss_fft_log.h defines a macro called ERROR,
 * which clashes with the HAL's ERROR, so keep the two in separate .c files.
 */
#include "kws_features.h"
#include "kws_feature_tables.h"
#include "kiss_fftr.h"
#include <math.h>
#include <string.h>

/* KISS FFT keeps its twiddle factors in this buffer instead of using malloc (~4.4 KB needed) */
#define KWS_FFT_MEM_BYTES 6144
static uint8_t fft_mem[KWS_FFT_MEM_BYTES] __attribute__((aligned(8)));
static kiss_fftr_cfg fft_cfg = NULL;

static float frame_buf[KWS_N_FFT];
static kiss_fft_cpx spec[KWS_N_BINS];
static float power[KWS_N_BINS];

int kws_features_init(void)
{
  size_t len = sizeof(fft_mem);
  fft_cfg = kiss_fftr_alloc(KWS_N_FFT, 0, fft_mem, &len);   /* len becomes the bytes needed */
  return (fft_cfg != NULL) ? 0 : -1;
}

void kws_logmel_frame(const float *samples, float *mel)
{
  /* 1. window */
  for (int i = 0; i < KWS_N_FFT; i++)
    frame_buf[i] = samples[i] * KWS_WINDOW[i];

  /* 2. real FFT: 400 samples -> 201 complex bins */
  kiss_fftr(fft_cfg, frame_buf, spec);

  /* 3. power */
  for (int k = 0; k < KWS_N_BINS; k++)
    power[k] = spec[k].r * spec[k].r + spec[k].i * spec[k].i;

  /* 4. mel filters (only the non-zero part of each triangle is stored) and 5. log */
  const float *w = KWS_MEL_W;
  for (int b = 0; b < KWS_N_MELS; b++) {
    const float *p = &power[KWS_MEL_START[b]];
    float s = 0.0f;
    for (int j = 0; j < KWS_MEL_LEN[b]; j++)
      s += w[j] * p[j];
    w += KWS_MEL_LEN[b];
    mel[b] = logf(s + 1e-6f);
  }
}

void kws_logmel_clip_int16(const int16_t *clip, float *out)
{
  static float samples[KWS_N_FFT];
  float mel[KWS_N_MELS];

  for (int t = 0; t < KWS_N_FRAMES; t++) {
    int start = t * KWS_HOP - KWS_N_FFT / 2;          /* frame t is centred on sample t*160 */
    for (int i = 0; i < KWS_N_FFT; i++) {
      int n = start + i;
      samples[i] = (n >= 0 && n < KWS_CLIP_LEN) ? (float)clip[n] * (1.0f / 32768.0f) : 0.0f;
    }
    kws_logmel_frame(samples, mel);
    for (int b = 0; b < KWS_N_MELS; b++)
      out[b * KWS_N_FRAMES + t] = mel[b];
  }
}
