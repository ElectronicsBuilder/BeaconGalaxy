/*
 * leds.c - the four status LEDs. See leds.h.
 */
#include "leds.h"
#include "app_hal.h"
#include <string.h>

static GPIO_TypeDef *const ports[LED_COUNT] = {
  LED_GREEN_GPIO_Port, LED_ORANGE_GPIO_Port, LED_RED_GPIO_Port, LED_BLUE_GPIO_Port };
static const uint16_t pins[LED_COUNT] = { LED_GREEN_Pin, LED_ORANGE_Pin, LED_RED_Pin, LED_BLUE_Pin };
static const char *const names[LED_COUNT] = { "green", "orange", "red", "blue" };

void Leds_Set(Led led, int on)
{
  if (led < LED_COUNT) HAL_GPIO_WritePin(ports[led], pins[led], on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Leds_Toggle(Led led)
{
  if (led < LED_COUNT) HAL_GPIO_TogglePin(ports[led], pins[led]);
}

int Leds_FromName(const char *name)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    if (strcmp(name, names[i]) == 0) return i;
  }
  return -1;
}
