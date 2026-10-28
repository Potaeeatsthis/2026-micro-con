/* voice.h - two INMP441 mics on SAI1 -> log-mel -> CNN -> EVT_VOICE_WORD
 *
 * This only recognises words; what they mean is in modes/mode_voice.c.
 * mode_voice sets which words are accepted (bit i = class id i). Other words are logged and dropped.
 */
#ifndef APP_VOICE_H
#define APP_VOICE_H

#include <stdint.h>
#include <stdbool.h>

void        voice_init(void);          /* checks the model, starts SAI1 DMA */
void        voice_step(void);          /* main loop: features + CNN every 100 ms */
void        voice_on_sai(int half);    /* SAI dma isr: 0 = first half, 1 = second half */

int         voice_n_classes(void);
const char *voice_class_name(int id);  /* "stop", "red box", "_unknown_" ... */
int         voice_class_id(const char *name); /* -1 if the model has no such word */

#define VOICE_ACCEPT_ALL 0xFFFFFFFFu
void        voice_set_accept_mask(uint32_t mask); /* main loop; default VOICE_ACCEPT_ALL */

int         voice_last_word(void);     /* -1 until the first accepted detection */
uint32_t    voice_last_word_ms(void);
uint32_t    voice_infer_us(void);      /* last CNN time */
bool        voice_running(void);

#endif /* APP_VOICE_H */
