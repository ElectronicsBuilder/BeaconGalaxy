/*
 * status_leds.c - see status_leds.h.
 */
#include "status_leds.h"
#include "leds.h"
#include "settings.h"
#include "ws2812.h"

/* sweep order across the board: red, blue, orange, green and back */
static const Led sweep4[] = { LED_R, LED_B, LED_O, LED_G, LED_O, LED_B };
static const Led sweep2[] = { LED_R, LED_G };

static uint32_t next_step, hold_until;
static unsigned pos;
static int last_effect = -1, last_dma = -1;

static void P_DmaHook(int busy)
{
  if (Settings_Get()->led_dma) Leds_Set(LED_B, busy);
}

void StatusLeds_Init(void)
{
  Ws2812_SetActivityHook(P_DmaHook);
}

void StatusLeds_Hold(uint32_t ms)
{
  hold_until = ms;      /* relative; turned into a deadline at the next poll */
  if (hold_until == 0) hold_until = 1;
  last_effect = -2;
}

void StatusLeds_Poll(uint32_t now)
{
  static uint32_t hold_deadline;
  if (last_effect == -2 && hold_until)
  {
    hold_deadline = now + hold_until;
    hold_until = 0;
  }
  if (last_effect == -2)
  {
    if ((int32_t)(now - hold_deadline) < 0) return;
    last_effect = -1;
  }

  const Settings *s = Settings_Get();
  int effect = s->led_effect, dma = s->led_dma != 0;
  if (effect != last_effect || dma != last_dma)
  {
    for (int i = 0; i < LED_COUNT; i++) Leds_Set((Led)i, 0);
    last_effect = effect;
    last_dma = dma;
    pos = 0;
    next_step = now;
  }
  if (effect != 0 || (int32_t)(now - next_step) < 0) return;

  const Led *seq = dma ? sweep2 : sweep4;
  unsigned n = dma ? sizeof sweep2 / sizeof sweep2[0] : sizeof sweep4 / sizeof sweep4[0];
  Leds_Set(seq[pos % n], 0);
  pos = (pos + 1u) % n;
  Leds_Set(seq[pos], 1);
  unsigned d = s->led_delay < 10u ? 10u : s->led_delay;
  next_step = now + 2u * d;
}
