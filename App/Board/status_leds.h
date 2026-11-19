/*
 * status_leds.h - what the four status LEDs show (Settings > LEDS).
 *
 *  led_effect   one of LED_FX_* below (0 and 1 keep the original meaning)
 *  led_delay    10..100 ms: the pace of the effect (one step of a sweep, etc.)
 *  led_dma      1: the blue LED lights while a matrix frame is being sent, and
 *               the effects use the other three LEDs
 */
#ifndef APP_STATUS_LEDS_H
#define APP_STATUS_LEDS_H

#include <stdint.h>

enum {
  LED_FX_SWEEP,      /* one light running back and forth, fading tail ("knight rider") */
  LED_FX_OFF,
  LED_FX_CHASE,      /* one light running round in one direction, fading tail */
  LED_FX_BREATHE,    /* all fade in and out */
  LED_FX_HEARTBEAT,  /* two quick beats, then rest */
  LED_FX_BLINK,      /* all on / all off */
  LED_FX_ALTERNATE,  /* odd and even LEDs take turns */
  LED_FX_SPARKLE,    /* random LEDs flash and fade */
  LED_FX_FILL,       /* a bar fills up, then empties */
  LED_FX_WAVE,       /* a smooth brightness wave rolls along */
  LED_FX_COUNT
};

void StatusLeds_Init(void);
void StatusLeds_Poll(uint32_t now_ms);      /* App task */
void StatusLeds_Hold(uint32_t ms);          /* leave the LEDs alone (console "led") */
const char *StatusLeds_EffectName(unsigned effect);

#endif
