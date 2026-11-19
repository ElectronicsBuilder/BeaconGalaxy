/*
 * mic.h - microphone on PA0 (ADC1 IN0), sampled at MIC_RATE_HZ by TIM2 into a
 * circular DMA buffer (DMA2 stream 0). Runs only while an equalizer is shown.
 */
#ifndef APP_MIC_H
#define APP_MIC_H

#include <stdint.h>

#define MIC_RATE_HZ   16000u
#define MIC_BLOCK     256u          /* samples per block (one FFT) */

void Mic_Start(void);
void Mic_Stop(void);
int  Mic_Running(void);
/* Copies the newest complete block (12-bit samples). Returns 0, or -1 if no new
 * block since the last call. */
int  Mic_Read(uint16_t *out);
/* Blocks captured since Mic_Start (for the console) */
unsigned long Mic_Blocks(void);

#endif
