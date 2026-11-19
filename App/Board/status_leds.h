/*
 * status_leds.h - what the four status LEDs show (Settings: LEDS menu).
 *
 *  led_effect 0  "knight rider" sweep, led_delay ms per step
 *  led_effect 1  off
 *  led_dma    1  the blue LED is lit while a matrix frame is being sent (and
 *                the sweep keeps to red and green, as the original firmware)
 */
#ifndef APP_STATUS_LEDS_H
#define APP_STATUS_LEDS_H

#include <stdint.h>

void StatusLeds_Init(void);
void StatusLeds_Poll(uint32_t now_ms);      /* App task */
void StatusLeds_Hold(uint32_t ms);          /* leave the LEDs alone (console "led") */

#endif
