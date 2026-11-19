/*
 * fx_int.h - interface between the fx task and the effects (fx_*.c).
 *
 * start(arg) prepares an effect (returns -1 if it cannot run). frame(now)
 * draws the next picture into the matrix framebuffer and returns the number of
 * ms until the next frame, FX_SKIP to not show anything this time (no change),
 * or FX_END when the effect is over. stop() releases what start() took.
 */
#ifndef APP_FX_INT_H
#define APP_FX_INT_H

#include "fx.h"
#include "matrix.h"
#include <stdint.h>

#define FX_END   0u
#define FX_SKIP  0x80000000u      /* flag: no new picture; low bits = delay */

typedef struct {
  int      (*start)(int32_t arg);
  uint32_t (*frame)(uint32_t now);
  void     (*stop)(void);
  int      full_level;            /* 1: shown at 255, not at the intensity setting */
} FxOps;

extern const FxOps fx_color_ops, fx_gif_ops, fx_theme_ops, fx_eq_ops, fx_test_ops;

uint32_t Fx_Random(uint32_t n);   /* 0..n-1 (hardware RNG) */
void     Fx_CountError(void);

#endif
