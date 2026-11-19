/*
 * fx.h - what the LED matrix shows. One "fx" task owns the matrix and runs one
 * effect at a time; other tasks ask for a change with Fx_Request().
 */
#ifndef APP_FX_H
#define APP_FX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  FX_OFF,
  FX_COLOR,      /* the colour from Settings (red/green/blue), full level */
  FX_GIF,        /* arg = index in GifList */
  FX_THEME,      /* arg = FX_THEME_* */
  FX_EQ,         /* arg = FX_EQ_BARS / FX_EQ_VU */
  FX_TEST,       /* arg = test pattern 0..2 */
  FX_KIND_COUNT
} FxKind;

enum { FX_THEME_FIRE, FX_THEME_METEOR, FX_THEME_BOUNCING, FX_THEME_PLASMA, FX_THEME_STARS,
       FX_THEME_RAIN, FX_THEME_SWIRL, FX_THEME_LIFE, FX_THEME_COUNT };
const char *Fx_ThemeName(int theme);       /* "" if out of range */
enum { FX_EQ_BARS, FX_EQ_VU };

void Fx_Init(void);                      /* creates the task */
int  Fx_Request(FxKind kind, int32_t arg);   /* any task; -1 if the queue is full */

typedef struct {
  FxKind kind;
  int32_t arg;
  unsigned long frames;                  /* shown since this effect started */
  unsigned long errors;                  /* failed frames (missing / bad files) */
  uint32_t last_frame_ms;                /* time to produce the last frame */
} FxStatus;
void Fx_GetStatus(FxStatus *out);
const char *Fx_KindName(FxKind kind);

/* Equalizer tuning (console): signal below gate_db shows nothing */
void Fx_EqSetGate(int gate_db);
int  Fx_EqGate(void);
/* Last measured mic level (dB, 0 if the equalizer is not running) */
int  Fx_EqLevel(void);
/* GIF: ms to open / read the last frame file */
void Fx_GifTiming(uint32_t *open_ms, uint32_t *read_ms);

#ifdef __cplusplus
}
#endif

#endif
