/*
 * matrix_px.c - pure pixel helpers of matrix.h (no hardware; host tested).
 */
#include "matrix.h"

/* Same perceptual curve as the original firmware (gamma 2.8) */
static const uint8_t gamma8[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
  2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
  5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

int Matrix_ChainIndex(int x, int y)
{
  if (x < 0 || x >= MX_W || y < 0 || y >= MX_H) return -1;
  return (y & 1) ? y * MX_W + (MX_W - 1 - x) : y * MX_W + x;
}

uint8_t Matrix_Gamma(uint8_t v)
{
  return gamma8[v];
}

uint8_t Matrix_Scale(uint8_t v, uint8_t level)
{
  return (uint8_t)(((unsigned)v * level + 127u) / 255u);
}

Rgb Hsv_ToRgb(uint8_t h, uint8_t s, uint8_t v)
{
  if (s == 0) return Rgb_Make(v, v, v);
  unsigned region = h / 43u;                       /* six sectors of 0..255 */
  unsigned rem = (h - region * 43u) * 6u;          /* 0..255 within the sector */
  uint8_t p = (uint8_t)((v * (255u - s)) / 255u);
  uint8_t q = (uint8_t)((v * (255u - (s * rem) / 255u)) / 255u);
  uint8_t t = (uint8_t)((v * (255u - (s * (255u - rem)) / 255u)) / 255u);
  switch (region)
  {
    case 0:  return Rgb_Make(v, t, p);
    case 1:  return Rgb_Make(q, v, p);
    case 2:  return Rgb_Make(p, v, t);
    case 3:  return Rgb_Make(p, q, v);
    case 4:  return Rgb_Make(t, p, v);
    default: return Rgb_Make(v, p, q);
  }
}
