/*
 * settings.h - user settings kept in the EEPROM (one CRC-protected record).
 *
 * The UI edits a copy (Settings_Get) and asks the App task to store it
 * (UI_SETTINGS_SAVE). A blank or corrupt record loads the defaults.
 */
#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SETTINGS_EE_ADDR 0x0100u   /* keeps clear of the boot fuse (bytes 100..102) */

typedef struct {
  uint8_t red, green, blue;     /* colour trim 0..255 */
  uint8_t intensity;            /* matrix brightness 0..255 */
  uint8_t volume;               /* audio 0..100 */
  uint8_t ui_step;              /* +/- step in the value editors, 1..99 */
  uint8_t gif_fps;              /* 1..100 */
  uint8_t autoplay_max_min;     /* auto GIF/theme: change every 1..N minutes (random) */
  uint8_t led_effect;           /* 0..1 */
  uint8_t led_delay;            /* 10..100 ms */
  uint8_t led_dma;              /* 1: blue status LED shows matrix transfers */
  uint8_t repeat;               /* audio repeat on/off */
  uint8_t auto_wav_random;      /* autoplay order: 0 = sequential, 1 = random */
  uint8_t auto_gif_random;
  uint8_t auto_theme_random;
} Settings;

void Settings_Load(void);              /* App task, at boot */
int  Settings_Save(void);              /* App task; 0 = stored */
Settings *Settings_Get(void);          /* the live copy (single-byte fields: safe to read anywhere) */
void Settings_Defaults(Settings *s);

#ifdef __cplusplus
}
#endif

#endif
