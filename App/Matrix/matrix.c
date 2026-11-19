/*
 * matrix.c - framebuffer and Matrix_Show(); see matrix.h.
 */
#include "matrix.h"
#include "ws2812.h"
#include "cmsis_os2.h"
#include <string.h>

#define SHOW_WAIT_MS 30u

static Rgb fb[MX_H][MX_W];
static uint8_t wire[3u * MX_LEDS];    /* G, R, B in chain order, read by the DMA ISR */

void Matrix_Init(void)
{
  Ws2812_Init();
  Matrix_Clear();
  (void)Matrix_Show(0);
}

void Matrix_Clear(void)
{
  memset(fb, 0, sizeof fb);
}

void Matrix_Fill(Rgb c)
{
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++) fb[y][x] = c;
}

void Matrix_Set(int x, int y, Rgb c)
{
  if (x >= 0 && x < MX_W && y >= 0 && y < MX_H) fb[y][x] = c;
}

Rgb Matrix_Get(int x, int y)
{
  if (x >= 0 && x < MX_W && y >= 0 && y < MX_H) return fb[y][x];
  return RGB_OFF;
}

int Matrix_Show(uint8_t level)
{
  for (uint32_t waited = 0; Ws2812_Busy(); waited++)
  {
    if (waited >= SHOW_WAIT_MS) return -1;
    osDelay(1);
  }
  for (int y = 0; y < MX_H; y++)
  {
    for (int x = 0; x < MX_W; x++)
    {
      uint8_t *w = &wire[3 * Matrix_ChainIndex(x, y)];
      const Rgb c = fb[y][x];
      w[0] = Matrix_Gamma(Matrix_Scale(c.g, level));
      w[1] = Matrix_Gamma(Matrix_Scale(c.r, level));
      w[2] = Matrix_Gamma(Matrix_Scale(c.b, level));
    }
  }
  return Ws2812_Send(wire, MX_LEDS);
}
