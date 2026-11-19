/*
 * matrix.h - the 22x22 LED matrix as a picture: x = 0..21 left to right,
 * y = 0..21 bottom to top (the chain starts bottom-left and snakes: even rows
 * run left to right, odd rows right to left).
 *
 * Drawing goes to a framebuffer; Matrix_Show() applies brightness and gamma,
 * reorders into chain order and hands the frame to the WS2812 driver. Only one
 * task (the effects task) draws.
 */
#ifndef APP_MATRIX_H
#define APP_MATRIX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MX_W     22
#define MX_H     22
#define MX_LEDS  (MX_W * MX_H)

typedef struct { uint8_t r, g, b; } Rgb;

static inline Rgb Rgb_Make(uint8_t r, uint8_t g, uint8_t b) { Rgb c = { r, g, b }; return c; }
#define RGB_OFF     Rgb_Make(0, 0, 0)

/* Pure helpers (host tests) */
int     Matrix_ChainIndex(int x, int y);           /* -1 if outside */
uint8_t Matrix_Gamma(uint8_t v);
uint8_t Matrix_Scale(uint8_t v, uint8_t level);    /* v * level / 255, rounded */
Rgb     Hsv_ToRgb(uint8_t h, uint8_t s, uint8_t v); /* all 0..255 */

void Matrix_Init(void);
void Matrix_Clear(void);
void Matrix_Fill(Rgb c);
void Matrix_Set(int x, int y, Rgb c);              /* ignored outside */
Rgb  Matrix_Get(int x, int y);
/* Sends the picture at brightness level 0..255 (255 = as drawn). Waits up to
 * 30 ms for the previous frame to finish; returns -1 if it never did. */
int  Matrix_Show(uint8_t level);

#ifdef __cplusplus
}
#endif

#endif
