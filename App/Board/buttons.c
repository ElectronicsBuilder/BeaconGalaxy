/*
 * buttons.c - front buttons. See buttons.h.
 * Active HIGH (10k pull-downs on the board; measured: idle pins read 0 even
 * with the internal pull-ups on). A press is reported once the pin
 * has been stable for 20 ms; holding a key repeats it every 150 ms after 600 ms.
 */
#include "buttons.h"
#include "app_hal.h"
#include "ring.h"
#include <string.h>

#define DEBOUNCE_MS     20u
#define REPEAT_DELAY_MS 600u
#define REPEAT_MS       150u

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  Key key;
  uint8_t stable;       /* debounced: 1 = down */
  uint8_t last_raw;
  uint32_t same_ms;     /* how long the raw level is unchanged */
  uint32_t held_ms;
} Button;

static Button buttons[] = {
  { BTN_UP_GPIO_Port,    BTN_UP_Pin,    KEY_UP,    0, 0, 0, 0 },
  { BTN_DOWN_GPIO_Port,  BTN_DOWN_Pin,  KEY_DOWN,  0, 0, 0, 0 },
  { BTN_BACK_GPIO_Port,  BTN_BACK_Pin,  KEY_BACK,  0, 0, 0, 0 },
  { BTN_ENTER_GPIO_Port, BTN_ENTER_Pin, KEY_ENTER, 0, 0, 0, 0 },
};
#define BUTTON_COUNT (sizeof buttons / sizeof buttons[0])

static uint8_t queue_mem[16];
static Ring queue;   /* one producer at a time: the App task (poll + console) */

void Buttons_Init(void)
{
  Ring_Init(&queue, queue_mem, sizeof queue_mem);
}

void Buttons_Inject(Key k)
{
  if (k != KEY_NONE) Ring_Put(&queue, (uint8_t)k);
}

Key Buttons_Get(void)
{
  uint8_t k;
  return Ring_Get(&queue, &k) ? (Key)k : KEY_NONE;
}

void Buttons_Poll(uint32_t elapsed_ms)
{
  for (unsigned i = 0; i < BUTTON_COUNT; i++)
  {
    Button *b = &buttons[i];
    uint8_t raw = HAL_GPIO_ReadPin(b->port, b->pin) == GPIO_PIN_SET;
    if (raw != b->last_raw)
    {
      b->last_raw = raw;
      b->same_ms = 0;
      continue;
    }
    b->same_ms += elapsed_ms;
    if (b->same_ms >= DEBOUNCE_MS && raw != b->stable)
    {
      b->stable = raw;
      b->held_ms = 0;
      if (raw) Buttons_Inject(b->key);
    }
    if (b->stable)
    {
      uint32_t before = b->held_ms;
      b->held_ms += elapsed_ms;
      if (b->key != KEY_ENTER && b->key != KEY_BACK && b->held_ms >= REPEAT_DELAY_MS &&
          (before < REPEAT_DELAY_MS || (b->held_ms - REPEAT_DELAY_MS) / REPEAT_MS !=
                                       (before - REPEAT_DELAY_MS) / REPEAT_MS))
      {
        Buttons_Inject(b->key);   /* auto-repeat for UP / DOWN */
      }
    }
  }
}

int Buttons_IsDown(Key k)
{
  for (unsigned i = 0; i < BUTTON_COUNT; i++)
  {
    if (buttons[i].key == k) return buttons[i].stable;
  }
  return 0;
}

static const char *const names[] = { "none", "up", "down", "back", "enter" };

const char *Buttons_Name(Key k)
{
  return ((unsigned)k < 5u) ? names[k] : "?";
}

Key Buttons_FromName(const char *name)
{
  for (int i = 1; i < 5; i++)
  {
    if (strcmp(name, names[i]) == 0) return (Key)i;
  }
  return KEY_NONE;
}
