/* kws_stream.c - see kws_stream.h */
#include "kws_stream.h"
#include <math.h>
#include <string.h>

#define AMASK (KWS_AUDIO_RING - 1)
#define FMASK (KWS_FEAT_RING - 1)
#define LN_TO_DB 4.3429448f                 /* 10 / ln(10): ln(power) -> dB */

static int16_t  audio[KWS_N_CH][KWS_AUDIO_RING];
static volatile uint32_t audio_written = 0;  /* samples per channel since start (only the ISR writes) */

static float    feat[KWS_N_CH][KWS_FEAT_RING][KWS_N_MELS];
static float    energy[KWS_N_CH][KWS_FEAT_RING];   /* mean log-mel of each frame */
static float    noise[KWS_N_CH];
static uint32_t next_frame = 0;              /* also = number of frames made */
static uint32_t overruns = 0;
static float    scale = 1.0f / 32768.0f;     /* int16 -> [-1, 1], times the gain */
static int      cur_ch = 0;

void kws_stream_init(float gain_db)
{
  scale = powf(10.0f, gain_db / 20.0f) / 32768.0f;
  audio_written = 0;
  next_frame = 0;
  overruns = 0;
  cur_ch = 0;
  memset(audio, 0, sizeof(audio));
}

void kws_stream_push(const int16_t *ch0, const int16_t *ch1, int n)
{
  uint32_t w = audio_written;
  for (int i = 0; i < n; i++) {
    audio[0][(w + i) & AMASK] = ch0[i];
    audio[1][(w + i) & AMASK] = ch1[i];
  }
  audio_written = w + n;                     /* publish only after the samples are in */
}

int kws_stream_poll(void)
{
  static float buf[KWS_N_FFT];
  int made = 0;

  for (;;) {
    uint32_t avail = audio_written;
    uint32_t start = next_frame * KWS_HOP;
    if (start + KWS_N_FFT > avail)
      break;                                 /* next frame not complete yet */

    if (avail - start > KWS_AUDIO_RING - 2 * KWS_HOP) {
      /* fell too far behind: these samples are (about to be) overwritten. jump to the newest frame */
      overruns++;
      next_frame = (avail - KWS_N_FFT) / KWS_HOP;
      continue;
    }

    uint32_t slot = next_frame & FMASK;
    for (int ch = 0; ch < KWS_N_CH; ch++) {
      for (int i = 0; i < KWS_N_FFT; i++)
        buf[i] = (float)audio[ch][(start + i) & AMASK] * scale;

      float *mel = feat[ch][slot];
      kws_logmel_frame(buf, mel);

      float e = 0.0f;
      for (int b = 0; b < KWS_N_MELS; b++) e += mel[b];
      e *= (1.0f / KWS_N_MELS);
      energy[ch][slot] = e;

      /* noise floor: follows quiet frames quickly, loud frames very slowly (~5 s) */
      if (next_frame == 0)       noise[ch] = e;
      else if (e < noise[ch])    noise[ch] += 0.2f   * (e - noise[ch]);
      else                       noise[ch] += 0.002f * (e - noise[ch]);
    }
    next_frame++;
    made++;
  }
  return made;
}

uint32_t kws_stream_frames(void) { return next_frame; }
uint32_t kws_stream_overruns(void) { return overruns; }

static float peak_ln(int ch)
{
  uint32_t n = (next_frame < KWS_N_FRAMES) ? next_frame : KWS_N_FRAMES;
  float p = -1e30f;
  for (uint32_t k = 1; k <= n; k++) {
    float e = energy[ch][(next_frame - k) & FMASK];
    if (e > p) p = e;
  }
  return p;
}

float kws_stream_peak_db(int ch)  { return LN_TO_DB * peak_ln(ch); }
float kws_stream_noise_db(int ch) { return LN_TO_DB * noise[ch]; }

int kws_stream_choose_channel(float *snr_db)
{
  float snr[KWS_N_CH];
  for (int ch = 0; ch < KWS_N_CH; ch++)
    snr[ch] = LN_TO_DB * (peak_ln(ch) - noise[ch]);

  int other = 1 - cur_ch;
  if (snr[other] > snr[cur_ch] + 2.0f)       /* switch only if clearly better */
    cur_ch = other;
  if (snr_db) *snr_db = snr[cur_ch];
  return cur_ch;
}

void kws_stream_window(int ch, float *dst)
{
  uint32_t first = next_frame - KWS_N_FRAMES;   /* caller makes sure next_frame >= 101 */
  for (int t = 0; t < KWS_N_FRAMES; t++) {
    const float *mel = feat[ch][(first + t) & FMASK];
    for (int b = 0; b < KWS_N_MELS; b++)
      dst[b * KWS_N_FRAMES + t] = mel[b];
  }
}
