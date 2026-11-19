/*
 * ws2812.h - WS2812B chain on PB0 (TIM3 CH3 PWM, DMA1 stream 2 on TIM3 update).
 *
 * One PWM period per bit (800 kHz). The DMA runs circular over a small buffer of
 * two halves (WS_HALF_LEDS LEDs each); the half-transfer / transfer-complete
 * interrupt encodes the next LEDs into the half that was just sent, so a whole
 * 22x22 frame needs 384 bytes of PWM buffer instead of 23 KB. A run of zero
 * periods after the data gives the >280 us latch.
 */
#ifndef APP_WS2812_H
#define APP_WS2812_H

#include <stdint.h>

#define WS_HALF_LEDS   4u

void Ws2812_Init(void);
/* Starts sending grb[0..3*leds-1] (G, R, B per LED, chain order). The buffer must
 * stay unchanged until Ws2812_Busy() returns 0. Returns -1 if a frame is still
 * being sent. */
int  Ws2812_Send(const uint8_t *grb, unsigned leds);
int  Ws2812_Busy(void);
/* Frames sent / ISR count, for the console */
void Ws2812_Stats(unsigned long *frames, unsigned long *irqs);
/* Optional activity hook: called with 1 at the start and 0 at the end of a frame
 * (from the DMA interrupt, priority 1: must not call the RTOS) */
void Ws2812_SetActivityHook(void (*hook)(int busy));

#endif
