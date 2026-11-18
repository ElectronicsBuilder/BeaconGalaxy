/*
 * leds.h - the four status LEDs (PB6 green, PB7 orange, PB8 red, PB9 blue).
 */
#ifndef APP_LEDS_H
#define APP_LEDS_H

typedef enum { LED_G = 0, LED_O, LED_R, LED_B, LED_COUNT } Led;

void Leds_Set(Led led, int on);
void Leds_Toggle(Led led);
int  Leds_FromName(const char *name);   /* -1 if unknown */

#endif
