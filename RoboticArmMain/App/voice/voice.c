/* voice.c - continuous keyword spotting (port of model_realtime_code as on the board 22 Oct 2026)
 *
 * needs, in this folder:
 *   from the released model (voice_command_research/final_model/<model>/kws_c/):
 *     kws_classes.h                       class order, generated from config.py - never edit
 *   unchanged feature code (same as the live project):
 *     kws_features.c/.h, kws_stream.c/.h, kws_feature_tables.h, kiss_fft.c/.h, _kiss_fft_guts.h
 * and Models/kws.onnx from the same release folder (X-CUBE-AI, regenerate after replacing it).
 *
 * differences from the live project:
 *   - the SAI callbacks live in app_callbacks.c and call voice_on_sai()
 *   - the audio buffer is DMA_BUF (non-cacheable), so no SCB_InvalidateDCache call
 *   - detections become EVT_VOICE_WORD events (the printf line is kept)
 *   - __io_putchar and the DWT setup moved to common/; the clip self-test stays in model_realtime_code
 */
#include "voice/voice.h"
#include "voice/kws_model.h"
#include "voice/kws_classes.h"
#include "voice/kws_features.h"
#include "voice/kws_stream.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_timing.h"
#include "common/app_util.h"
#include "common/dma_buf.h"
#include "main.h"
#include "sai.h"
#include "network.h"
#include <string.h>

/* ---- stream settings, same values as the board on 22 Oct (context/voice/status.md) ---- */
#define VOICE_GAIN_DB        15.0f   /* KWS_GAIN_DB */
#define VOICE_STRIDE_FRAMES  10      /* run the CNN every 10 new frames = 100 ms */
#define VOICE_AVG            3       /* average the last 3 results */
#define VOICE_THRESHOLD      0.75f   /* averaged probability needed for a detection */
#define VOICE_MIN_SNR_DB     12.0f   /* KWS_MIN_SNR_DB: quieter than this is not a command */
#define VOICE_REFRACTORY_MS  1500u   /* no second detection within 1.5 s */
#define VOICE_DEBUG_STATUS   0       /* 1 = status line on USART3 once per second */

#define AUDIO_BUF_WORDS  640
#define HALF_WORDS       (AUDIO_BUF_WORDS / 2)  /* 320 words = 160 stereo frames = 10 ms */
#define FRAMES_PER_HALF  (HALF_WORDS / 2)

static int32_t s_audio[AUDIO_BUF_WORDS] DMA_BUF;

static bool     s_running;
static float    s_hist[VOICE_AVG][N_CLASSES];
static int      s_hist_n, s_hist_pos;
static uint32_t s_last_infer_frame;
static uint32_t s_last_detect_ms, s_last_status_ms, s_led_ms;
static uint32_t s_infer_us;
static uint32_t s_overruns_seen;
static int      s_last_word = -1;
static uint32_t s_last_word_ms;
static uint32_t s_accept = VOICE_ACCEPT_ALL;   /* words the active task state wants (mode_voice) */
static int      s_ignored_word = -1;
static uint32_t s_ignored_ms;

_Static_assert(N_CLASSES <= 32, "voice_set_accept_mask holds one bit per class");

/* ---------------------------------------------------------------- isr side */
typedef struct { float x1; float y1; } dc_state_t;
static dc_state_t s_dc0, s_dc1;

/* one-pole dc blocker, y[n] = x[n] - x[n-1] + 0.995 y[n-1], 24-bit in -> int16 out */
static int16_t dc_block(dc_state_t *s, int32_t x24)
{
  float x = (float)x24;
  float y = x - s->x1 + 0.995f * s->y1;
  s->x1 = x;
  s->y1 = y;
  float out = clampf(y * (1.0f / 256.0f), -32768.0f, 32767.0f);
  return (int16_t)(out >= 0.0f ? out + 0.5f : out - 0.5f);
}

void voice_on_sai(int half)
{
  static int16_t ch0[FRAMES_PER_HALF], ch1[FRAMES_PER_HALF];
  const int32_t *src = &s_audio[half ? HALF_WORDS : 0];
  for (int k = 0; k < FRAMES_PER_HALF; k++) {
    ch0[k] = dc_block(&s_dc0, src[2 * k] >> 8);       /* left mic  */
    ch1[k] = dc_block(&s_dc1, src[2 * k + 1] >> 8);   /* right mic */
  }
  kws_stream_push(ch0, ch1, FRAMES_PER_HALF);
}

/* ---------------------------------------------------------------- init */
void voice_init(void)
{
  s_running = false;

  if (!kws_model_ready()) {
    LOG_ERR("voice: network not created (MX_X_CUBE_AI_Init / USER CODE 5)\r\n");
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  if (AI_NETWORK_IN_1_SIZE != KWS_N_MELS * KWS_N_FRAMES) {
    LOG_ERR("voice: model input %d values, features make %d\r\n",
            (int)AI_NETWORK_IN_1_SIZE, (int)(KWS_N_MELS * KWS_N_FRAMES));
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  if (AI_NETWORK_OUT_1_SIZE != N_CLASSES) {
    LOG_ERR("voice: model has %d outputs, kws_classes.h has %d - copy both from the same release\r\n",
            (int)AI_NETWORK_OUT_1_SIZE, N_CLASSES);
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  if (kws_features_init() != 0) {
    LOG_ERR("voice: kws_features_init failed (fft memory)\r\n");
    app_fault_raise(FAULT_INIT, false);
    return;
  }
  kws_stream_init(VOICE_GAIN_DB);

  if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)s_audio, AUDIO_BUF_WORDS) != HAL_OK) {
    LOG_ERR("voice: sai dma start failed\r\n");
    app_fault_raise(FAULT_INIT, false);
    return;
  }

  s_running = true;
  LOG("voice: listening, %d classes, threshold %d%%, min snr %d dB, gain %d dB\r\n", N_CLASSES,
      (int)(VOICE_THRESHOLD * 100.0f), (int)VOICE_MIN_SNR_DB, (int)VOICE_GAIN_DB);
}

/* ---------------------------------------------------------------- main loop */
static void status_line(void)
{
#if VOICE_DEBUG_STATUS
  uint32_t now = HAL_GetTick();
  if (ms_since(now, s_last_status_ms) < 1000u) return;
  s_last_status_ms = now;
  float snr;
  int ch = kws_stream_choose_channel(&snr);
  LOG("voice: peak %d/%d dB  noise %d/%d dB  ch%d snr %d dB  cnn %lu us  overruns %lu\r\n",
      (int)kws_stream_peak_db(0), (int)kws_stream_peak_db(1),
      (int)kws_stream_noise_db(0), (int)kws_stream_noise_db(1),
      ch, (int)snr, (unsigned long)s_infer_us, (unsigned long)kws_stream_overruns());
#else
  (void)s_last_status_ms;
#endif
}

void voice_step(void)
{
  if (!s_running) return;

  uint32_t now = HAL_GetTick();
  if (s_led_ms != 0u && ms_since(now, s_led_ms) > 300u) {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    s_led_ms = 0u;
  }

  uint32_t ov = kws_stream_overruns();
  if (ov != s_overruns_seen) {
    s_overruns_seen = ov;
    app_fault_raise(FAULT_VOICE_OVERRUN, false);      /* some module blocked the loop > 256 ms */
  }

  (void)kws_stream_poll();
  status_line();

  uint32_t f = kws_stream_frames();
  if (f < KWS_N_FRAMES || f - s_last_infer_frame < VOICE_STRIDE_FRAMES) return;
  s_last_infer_frame = f;

  /* 1. better mic, last 101 frames -> network input */
  float snr_db;
  int ch = kws_stream_choose_channel(&snr_db);
  kws_stream_window(ch, kws_model_input());

  /* 2. network (~43 ms for exp04) */
  uint32_t t0 = timing_cycles();
  if (kws_model_run() != 0) return;
  s_infer_us = timing_us_since(t0);

  /* 3. average the last VOICE_AVG results */
  const float *p = kws_model_output();
  for (int i = 0; i < N_CLASSES; i++) s_hist[s_hist_pos][i] = p[i];
  s_hist_pos = (s_hist_pos + 1) % VOICE_AVG;
  if (s_hist_n < VOICE_AVG) s_hist_n++;

  int top = 0;
  float top_p = 0.0f;
  for (int i = 0; i < N_CLASSES; i++) {
    float s = 0.0f;
    for (int k = 0; k < s_hist_n; k++) s += s_hist[k][i];
    s /= (float)s_hist_n;
    if (s > top_p) { top_p = s; top = i; }
  }

  /* 4. detection: a command word, confident, loud enough, not right after the previous one */
  if (top == CLS_UNKNOWN || top == CLS_SILENCE) return;
  if (top_p < VOICE_THRESHOLD) return;
  if (snr_db < VOICE_MIN_SNR_DB) return;
  if (s_last_detect_ms != 0u && ms_since(now, s_last_detect_ms) < VOICE_REFRACTORY_MS) return;

  /* 5. word filter: a word the task state does not want is dropped and starts no refractory,
   *    so the word that follows is still heard. one log line per word, not one per CNN run */
  if ((s_accept & (1u << top)) == 0u) {
    if (top != s_ignored_word || ms_since(now, s_ignored_ms) >= VOICE_REFRACTORY_MS) {
      LOG("--- %s %d%% ignored (not wanted now)\r\n", CLASS_NAMES[top], (int)(top_p * 100.0f));
    }
    s_ignored_word = top;
    s_ignored_ms = now;
    return;
  }

  s_last_detect_ms = now;
  s_last_word = top;
  s_last_word_ms = now;
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  s_led_ms = now;

  LOG(">>> %s %d%% (ch%d, snr %d dB)\r\n", CLASS_NAMES[top], (int)(top_p * 100.0f), ch, (int)snr_db);
  (void)app_event_post(EVT_VOICE_WORD, (uint16_t)top, (int32_t)(top_p * 1000.0f), ch,
                       (int32_t)(snr_db * 10.0f));
}

/* ---------------------------------------------------------------- getters */
int voice_n_classes(void) { return N_CLASSES; }

const char *voice_class_name(int id)
{
  if (id < 0 || id >= N_CLASSES) return "?";
  return CLASS_NAMES[id];
}

int voice_class_id(const char *name)
{
  for (int i = 0; i < N_CLASSES; i++) {
    if (strcmp(CLASS_NAMES[i], name) == 0) return i;
  }
  return -1;
}

void voice_set_accept_mask(uint32_t mask) { s_accept = mask; }

int      voice_last_word(void)    { return s_last_word; }
uint32_t voice_last_word_ms(void) { return s_last_word_ms; }
uint32_t voice_infer_us(void)     { return s_infer_us; }
bool     voice_running(void)      { return s_running; }
