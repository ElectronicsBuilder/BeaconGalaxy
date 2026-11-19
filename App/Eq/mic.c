/*
 * mic.c - see mic.h. TIM2 TRGO (update) starts each ADC1 conversion; the DMA
 * fills two halves of MIC_BLOCK samples; the interrupt notes which half is ready.
 */
#include "mic.h"
#include "app_hal.h"
#include <string.h>

#define TIM2_CLOCK_HZ  84000000u
#define MIC_IRQ_PRIO   6u                   /* below the RTOS limit; no RTOS calls anyway */

static uint16_t buf[2u * MIC_BLOCK];
static volatile int ready_half = -1;
static volatile unsigned long blocks;
static int running;

void Mic_Start(void)
{
  if (running) return;
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  GPIO_InitTypeDef g = { 0 };
  g.Pin = GPIO_PIN_0;
  g.Mode = GPIO_MODE_ANALOG;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);

  /* ADC1: 12-bit, IN0, 56-cycle sampling, external trigger TIM2 TRGO on rising edge, DMA circular */
  ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE) | ADC_CCR_ADCPRE_0;          /* PCLK2 / 4 = 21 MHz */
  ADC1->CR1 = 0;
  ADC1->SQR1 = 0;                                                      /* one conversion */
  ADC1->SQR3 = 0;                                                      /* channel 0 */
  ADC1->SMPR2 = (3u << ADC_SMPR2_SMP0_Pos);                            /* 56 cycles */
  ADC1->CR2 = (6u << ADC_CR2_EXTSEL_Pos) | ADC_CR2_EXTEN_0 |           /* TIM2 TRGO, rising */
              ADC_CR2_DMA | ADC_CR2_DDS | ADC_CR2_ADON;

  DMA2_Stream0->CR = 0;
  while (DMA2_Stream0->CR & DMA_SxCR_EN) { }
  DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;
  DMA2_Stream0->PAR = (uint32_t)&ADC1->DR;
  DMA2_Stream0->M0AR = (uint32_t)buf;
  DMA2_Stream0->NDTR = 2u * MIC_BLOCK;
  DMA2_Stream0->FCR = 0;
  DMA2_Stream0->CR = (0u << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 |
                     DMA_SxCR_MINC | DMA_SxCR_CIRC | DMA_SxCR_HTIE | DMA_SxCR_TCIE | DMA_SxCR_EN;
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, MIC_IRQ_PRIO, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

  TIM2->CR1 = 0;
  TIM2->PSC = 0;
  TIM2->ARR = TIM2_CLOCK_HZ / MIC_RATE_HZ - 1u;
  TIM2->CNT = 0;
  TIM2->CR2 = (2u << TIM_CR2_MMS_Pos);                                 /* TRGO = update */
  TIM2->EGR = TIM_EGR_UG;
  ready_half = -1;
  blocks = 0;
  TIM2->CR1 = TIM_CR1_CEN;
  running = 1;
}

void Mic_Stop(void)
{
  if (!running) return;
  TIM2->CR1 = 0;
  DMA2_Stream0->CR &= ~DMA_SxCR_EN;
  HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
  ADC1->CR2 = 0;
  running = 0;
}

int Mic_Running(void)
{
  return running;
}

int Mic_Read(uint16_t *out)
{
  int h = ready_half;
  if (h < 0) return -1;
  ready_half = -1;
  memcpy(out, &buf[(unsigned)h * MIC_BLOCK], MIC_BLOCK * sizeof buf[0]);
  return 0;
}

unsigned long Mic_Blocks(void)
{
  return blocks;
}

void DMA2_Stream0_IRQHandler(void)
{
  uint32_t isr = DMA2->LISR;
  DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;
  if (isr & DMA_LISR_HTIF0) { ready_half = 0; blocks++; }
  if (isr & DMA_LISR_TCIF0) { ready_half = 1; blocks++; }
}
