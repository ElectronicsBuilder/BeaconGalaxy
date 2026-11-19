/*
 * leds.c - the four status LEDs. See leds.h.
 * TIM4 runs from the 84 MHz APB1 timer clock: prescaler 328, 256 steps =
 * about 1 kHz, too fast to see flicker. A square curve makes fades look even.
 */
#include "leds.h"
#include "app_hal.h"
#include <string.h>

#define PWM_STEPS  256u
#define PWM_PSC    327u

static volatile uint32_t *const ccr[LED_COUNT] = { &TIM4->CCR1, &TIM4->CCR2, &TIM4->CCR3, &TIM4->CCR4 };
static const char *const names[LED_COUNT] = { "green", "orange", "red", "blue" };

void Leds_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_TIM4_CLK_ENABLE();
  GPIO_InitTypeDef g = { 0 };
  g.Pin = LED_GREEN_Pin | LED_ORANGE_Pin | LED_RED_Pin | LED_BLUE_Pin;   /* PB6..PB9 */
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  g.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &g);

  TIM4->CR1 = 0;
  TIM4->PSC = PWM_PSC;
  TIM4->ARR = PWM_STEPS - 1u;
  TIM4->CCMR1 = (6u << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE | (6u << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
  TIM4->CCMR2 = (6u << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE | (6u << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;
  for (int i = 0; i < LED_COUNT; i++) *ccr[i] = 0;
  TIM4->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  TIM4->EGR = TIM_EGR_UG;
  TIM4->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}

void Leds_Level(Led led, uint8_t level)
{
  if (led < LED_COUNT) *ccr[led] = ((uint32_t)level * level + 254u) / 255u;   /* 0..255, square curve */
}

void Leds_Set(Led led, int on)
{
  Leds_Level(led, on ? 255u : 0u);
}

void Leds_Toggle(Led led)
{
  if (led < LED_COUNT) Leds_Set(led, *ccr[led] == 0u);
}

int Leds_FromName(const char *name)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    if (strcmp(name, names[i]) == 0) return i;
  }
  return -1;
}
