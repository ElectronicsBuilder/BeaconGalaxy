/*
 * status_leds.c - see status_leds.h.
 *
 * Every FRAME_MS the current effect computes a brightness for each LED in use
 * (positions 0..n-1 in board order: red, blue, orange, green, as the original
 * sweep). Effects with a "tail" only set the lit LED to full; the others fade
 * out by themselves.
 */
#include "status_leds.h"
#include "leds.h"
#include "settings.h"
#include "ws2812.h"
#include <stddef.h>

#define FRAME_MS   10u
#define TAIL_FADE  80u           /* % kept per frame */
#define MAX_LEDS   4

static const Led order4[] = { LED_R, LED_B, LED_O, LED_G };
static const Led order3[] = { LED_R, LED_O, LED_G };    /* blue shows the matrix transfers */

static const char *const names[LED_FX_COUNT] = {
  "SWEEP", "OFF", "CHASE", "BREATHE", "HEARTBT", "BLINK", "ALTERN", "SPARKLE", "FILL", "WAVE"
};

static uint8_t level[MAX_LEDS];
static uint32_t next_frame, hold_until, rng = 0x2545F491u;
static int holding;
static int last_effect = -1, last_dma = -1;
static uint32_t start;          /* ms when the effect (re)started */
static int last_step = -1;

const char *StatusLeds_EffectName(unsigned e)
{
  return e < LED_FX_COUNT ? names[e] : "?";
}

static void P_DmaHook(int busy)
{
  if (Settings_Get()->led_dma) Leds_Set(LED_B, busy);
}

void StatusLeds_Init(void)
{
  Leds_Init();
  Ws2812_SetActivityHook(P_DmaHook);
}

void StatusLeds_Hold(uint32_t ms)
{
  hold_until = ms ? ms : 1u;    /* relative; made a deadline at the next poll */
  holding = 1;
}

static uint32_t P_Rand(uint32_t n)
{
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return n ? rng % n : 0u;
}

/* triangle wave 0..255 with the given period */
static uint8_t P_Triangle(uint32_t t, uint32_t period)
{
  uint32_t p = t % period;
  uint32_t half = period / 2u;
  return (uint8_t)(p < half ? p * 255u / half : (period - p) * 255u / (period - half));
}

static void P_Fade(int n)
{
  for (int i = 0; i < n; i++) level[i] = (uint8_t)(level[i] * TAIL_FADE / 100u);
}

static void P_All(int n, uint8_t v)
{
  for (int i = 0; i < n; i++) level[i] = v;
}

/* one frame of effect e; t = ms since start, step = pace in ms */
static void P_Frame(int e, int n, uint32_t t, uint32_t step)
{
  int s = (int)(t / step);                       /* steps since start */
  int new_step = s != last_step;
  last_step = s;
  switch (e)
  {
    case LED_FX_SWEEP:                           /* 0 1 2 3 2 1 0 ... */
    {
      P_Fade(n);
      int period = 2 * (n - 1);
      int p = (s / 2) % period;                  /* two steps per LED, like the original */
      level[p < n ? p : period - p] = 255u;
      break;
    }
    case LED_FX_CHASE:
      P_Fade(n);
      level[(s / 2) % n] = 255u;
      break;
    case LED_FX_BREATHE:
      P_All(n, P_Triangle(t, step * 60u));
      break;
    case LED_FX_HEARTBEAT:                       /* beat, short gap, beat, long rest */
    {
      uint32_t beat = step * 4u, cycle = step * 40u, p = t % cycle;
      uint8_t v = 0;
      if (p < beat) v = 255u;
      else if (p >= beat * 2u && p < beat * 3u) v = 200u;
      if (v) P_All(n, v);
      else P_Fade(n);
      break;
    }
    case LED_FX_BLINK:
      P_All(n, ((s / 10) & 1) ? 0u : 255u);
      break;
    case LED_FX_ALTERNATE:
      P_Fade(n);
      for (int i = (s / 8) & 1; i < n; i += 2) level[i] = 255u;
      break;
    case LED_FX_SPARKLE:
      P_Fade(n);
      if (new_step && P_Rand(3u) == 0u) level[P_Rand((uint32_t)n)] = 255u;
      break;
    case LED_FX_FILL:                            /* 0..n lit, then n..0 */
    {
      int p = (s / 3) % (2 * n);
      int lit = p <= n ? p : 2 * n - p;
      for (int i = 0; i < n; i++) level[i] = i < lit ? 255u : (uint8_t)(level[i] * TAIL_FADE / 100u);
      break;
    }
    case LED_FX_WAVE:
      for (int i = 0; i < n; i++) level[i] = P_Triangle(t + (uint32_t)i * step * 6u, step * 24u);
      break;
    default:                                     /* LED_FX_OFF */
      P_All(n, 0u);
      break;
  }
}

void StatusLeds_Poll(uint32_t now)
{
  static uint32_t hold_deadline;
  if (holding)
  {
    if (hold_until)
    {
      hold_deadline = now + hold_until;
      hold_until = 0;
    }
    if ((int32_t)(now - hold_deadline) < 0) return;
    holding = 0;
    last_effect = -1;                            /* repaint after the console used them */
  }
  if ((int32_t)(now - next_frame) < 0) return;
  next_frame = now + FRAME_MS;

  const Settings *s = Settings_Get();
  int effect = s->led_effect < LED_FX_COUNT ? s->led_effect : LED_FX_SWEEP;
  int dma = s->led_dma != 0;
  const Led *order = dma ? order3 : order4;
  int n = dma ? 3 : 4;
  if (effect != last_effect || dma != last_dma)
  {
    for (int i = 0; i < LED_COUNT; i++) Leds_Set((Led)i, 0);
    for (int i = 0; i < MAX_LEDS; i++) level[i] = 0;
    last_effect = effect;
    last_dma = dma;
    last_step = -1;
    start = now;
  }
  uint32_t step = s->led_delay < 10u ? 10u : s->led_delay;
  P_Frame(effect, n, now - start, step);
  for (int i = 0; i < n; i++) Leds_Level(order[i], level[i]);
}
