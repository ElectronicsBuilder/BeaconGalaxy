/*
 * ws2812.c - see ws2812.h.
 *
 * TIM3 runs from the 84 MHz APB1 timer clock: 105 ticks = 1.25 us per bit.
 * CCR3 is preloaded, so the value the DMA writes at an update event is the
 * duty cycle of the next period: 0 keeps the line low (idle / latch).
 */
#include "ws2812.h"
#include "app_hal.h"
#include <stddef.h>

#define TIM_CLOCK_HZ    84000000u
#define BIT_TICKS       (TIM_CLOCK_HZ / 800000u)            /* 105 = 1.25 us */
#define T0H_TICKS       ((TIM_CLOCK_HZ / 1000000u) * 40u / 100u)   /* 0.40 us */
#define T1H_TICKS       ((TIM_CLOCK_HZ / 1000000u) * 80u / 100u)   /* 0.80 us */
#define HALF_SLOTS      (WS_HALF_LEDS * 24u)
#define RESET_HALVES    3u                                   /* 3 x 120 us low */
#define WS_IRQ_PRIO     1u                                   /* above the RTOS: never masked */

static uint16_t pwm[2u * HALF_SLOTS];
static const uint8_t *volatile data;
static volatile unsigned data_halves;    /* halves that carry LED data */
static volatile unsigned fill_next;      /* next half number to encode */
static volatile unsigned halves_done;
static volatile int busy;
static unsigned leds_total;
static unsigned long frames, irqs;
static void (*activity)(int busy);

static void P_Encode(uint16_t *dst, unsigned half)
{
  unsigned first = half * WS_HALF_LEDS;
  for (unsigned l = 0; l < WS_HALF_LEDS; l++)
  {
    unsigned led = first + l;
    if (half >= data_halves || led >= leds_total)
    {
      for (unsigned b = 0; b < 24u; b++) *dst++ = 0;
      continue;
    }
    const uint8_t *p = data + 3u * led;
    for (unsigned c = 0; c < 3u; c++)
    {
      uint8_t v = p[c];
      for (unsigned b = 0; b < 8u; b++, v <<= 1) *dst++ = (v & 0x80u) ? T1H_TICKS : T0H_TICKS;
    }
  }
}

void Ws2812_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* PB0 = TIM3_CH3 (AF2), push-pull, fast edges */
  GPIO_InitTypeDef g = { 0 };
  g.Pin = GPIO_PIN_0;
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_PULLDOWN;
  g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  g.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOB, &g);

  TIM3->CR1 = TIM_CR1_ARPE;
  TIM3->PSC = 0;
  TIM3->ARR = BIT_TICKS - 1u;
  TIM3->CCR3 = 0;
  TIM3->CCMR2 = (6u << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;     /* PWM mode 1, preload */
  TIM3->CCER = TIM_CCER_CC3E;
  TIM3->DIER = TIM_DIER_UDE;                                       /* DMA request on update */
  TIM3->EGR = TIM_EGR_UG;

  /* DMA1 stream 2, channel 5 = TIM3_UP: memory -> TIM3->CCR3, halfwords, circular */
  DMA1_Stream2->CR = 0;
  while (DMA1_Stream2->CR & DMA_SxCR_EN) { }
  DMA1_Stream2->PAR = (uint32_t)&TIM3->CCR3;
  DMA1_Stream2->M0AR = (uint32_t)pwm;
  DMA1_Stream2->NDTR = 2u * HALF_SLOTS;
  DMA1_Stream2->FCR = 0;                                           /* direct mode */
  DMA1_Stream2->CR = (5u << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_PL_1 | DMA_SxCR_PL_0 |
                     DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_CIRC |
                     DMA_SxCR_DIR_0 | DMA_SxCR_HTIE | DMA_SxCR_TCIE;
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, WS_IRQ_PRIO, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

int Ws2812_Busy(void)
{
  return busy;
}

int Ws2812_Send(const uint8_t *grb, unsigned leds)
{
  if (busy || grb == NULL || leds == 0) return -1;
  data = grb;
  leds_total = leds;
  data_halves = (leds + WS_HALF_LEDS - 1u) / WS_HALF_LEDS;
  halves_done = 0;
  P_Encode(&pwm[0], 0);
  P_Encode(&pwm[HALF_SLOTS], 1);
  fill_next = 2;
  busy = 1;
  if (activity) activity(1);

  TIM3->CR1 &= ~TIM_CR1_CEN;
  TIM3->CCR3 = 0;
  TIM3->CNT = 0;
  TIM3->EGR = TIM_EGR_UG;                     /* load CCR3 = 0 now */
  TIM3->SR = 0;
  DMA1->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2;
  DMA1_Stream2->M0AR = (uint32_t)pwm;
  DMA1_Stream2->NDTR = 2u * HALF_SLOTS;
  DMA1_Stream2->CR |= DMA_SxCR_EN;
  TIM3->CR1 |= TIM_CR1_CEN;
  return 0;
}

void Ws2812_Stats(unsigned long *f, unsigned long *i)
{
  if (f) *f = frames;
  if (i) *i = irqs;
}

void Ws2812_SetActivityHook(void (*hook)(int busy))
{
  activity = hook;
}

void DMA1_Stream2_IRQHandler(void)
{
  uint32_t isr = DMA1->LISR;
  DMA1->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2;
  irqs++;
  /* Switching the stream off at the end of a frame sets its transfer-complete
   * flag once more: that interrupt belongs to no frame. */
  if (!busy) return;
  if (!(isr & (DMA_LISR_HTIF2 | DMA_LISR_TCIF2))) return;

  uint16_t *half = (isr & DMA_LISR_TCIF2) ? &pwm[HALF_SLOTS] : &pwm[0];
  if (++halves_done >= data_halves + RESET_HALVES)
  {
    TIM3->CR1 &= ~TIM_CR1_CEN;                /* the line has been low for the latch time */
    DMA1_Stream2->CR &= ~DMA_SxCR_EN;
    TIM3->CCR3 = 0;
    frames++;
    busy = 0;
    if (activity) activity(0);
    return;
  }
  P_Encode(half, fill_next++);
}
