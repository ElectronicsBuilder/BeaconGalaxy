/*
 * buttons.h - the four front buttons (UP PC0, DOWN PC1, BACK PC2, ENTER PC7).
 * Polled and debounced in the App task; key presses go to a small queue that
 * the UI reads. The console can inject presses ("key up").
 */
#ifndef APP_BUTTONS_H
#define APP_BUTTONS_H

#include <stdint.h>

typedef enum {
  KEY_NONE = 0,
  KEY_UP,
  KEY_DOWN,
  KEY_BACK,
  KEY_ENTER,
} Key;

void Buttons_Init(void);
void Buttons_Poll(uint32_t elapsed_ms);    /* App task */
void Buttons_Inject(Key k);                /* console / VT100 arrows */
Key  Buttons_Get(void);                    /* next press, KEY_NONE if none */
int  Buttons_IsDown(Key k);                /* raw debounced state */
const char *Buttons_Name(Key k);
Key  Buttons_FromName(const char *name);

#endif
