/*
 * fft.h - small in-place radix-2 FFT (float, pure, host tested).
 */
#ifndef APP_FFT_H
#define APP_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

#define FFT_MAX_N 256

/* re/im of n points (n a power of two, <= FFT_MAX_N). Returns -1 if n is invalid. */
int  Fft_Run(float *re, float *im, unsigned n);
/* |X[k]| for k = 0..n/2-1 into mag (may alias re) */
void Fft_Magnitude(const float *re, const float *im, float *mag, unsigned n);

#ifdef __cplusplus
}
#endif

#endif
