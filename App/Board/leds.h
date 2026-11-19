/*
 * leds.h - the four status LEDs (PB6 green, PB7 orange, PB8 red, PB9 blue),
 * dimmable: they are TIM4 channels 1-4, driven by PWM (about 1 kHz).
 */
#ifndef APP_LEDS_H
#define APP_LEDS_H

#include <stdint.h>

typedef enum { LED_G = 0, LED_O, LED_R, LED_B, LED_COUNT } Led;

void Leds_Init(void);                      /* TIM4 PWM on PB6..PB9 */
void Leds_Set(Led led, int on);            /* full on / off (also from interrupts) */
void Leds_Level(Led led, uint8_t level);   /* 0..255, perceived brightness */
void Leds_Toggle(Led led);
int  Leds_FromName(const char *name);      /* -1 if unknown */

#endif
