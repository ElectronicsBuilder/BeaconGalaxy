/*
 * fx_eq.c - equalizer bars and VU meter from the microphone.
 *
 * Every 16 ms block (256 samples at 16 kHz): remove the DC offset, Hann
 * window, FFT, then 22 bands spaced logarithmically from 125 Hz to 8 kHz. Levels
 * are in dB against a slowly falling reference (automatic gain); anything below
 * the gate is treated as silence. Both modes show the 22 bands, one per column
 * (the original colours):
 *   BARS  blue bars, red peak dot on the top of each bar
 *   VU    blue bars turning red in the top third, peak dot floating above the
 *         bar (red in the top third, blue below)
 */
#include "fx_int.h"
#include "fft.h"
#include "mic.h"
#include <math.h>
#include <string.h>

#define N           MIC_BLOCK
#define BANDS       MX_W
#define FIRST_BIN   2                     /* 125 Hz */
#define LAST_BIN    (N / 2 - 1)           /* ~8 kHz */
#define RANGE_DB    36.0f                 /* shown dynamic range */
#define REF_FALL_DB 0.08f                 /* reference release per frame */
#define PEAK_HOLD   10                    /* frames before a peak dot falls */
#define PEAK_FALL   2                     /* frames per pixel of fall */
#define DEFAULT_GATE_DB 50

static int mode;
static uint16_t samples[N];
static float re[N], im[N];
static float window[N];
static uint8_t band_lo[BANDS + 1];
static float ref_db;
static uint8_t height[BANDS], peak[BANDS], peak_hold[BANDS], peak_tick[BANDS];
static volatile int gate_db = DEFAULT_GATE_DB;
static volatile int level_db;

void Fx_EqSetGate(int db) { gate_db = db; }
int  Fx_EqGate(void) { return gate_db; }
int  Fx_EqLevel(void) { return level_db; }

static int P_Start(int32_t arg)
{
  mode = (arg == FX_EQ_VU) ? FX_EQ_VU : FX_EQ_BARS;
  for (unsigned i = 0; i < N; i++) window[i] = 0.5f - 0.5f * cosf(6.28318530718f * (float)i / (float)(N - 1));
  for (int b = 0; b <= BANDS; b++)
  {
    float e = (float)FIRST_BIN * powf((float)LAST_BIN / (float)FIRST_BIN, (float)b / (float)BANDS);
    band_lo[b] = (uint8_t)lroundf(e);
  }
  ref_db = (float)gate_db + RANGE_DB / 2.0f;
  memset(height, 0, sizeof height);
  memset(peak, 0, sizeof peak);
  memset(peak_hold, 0, sizeof peak_hold);
  memset(peak_tick, 0, sizeof peak_tick);
  level_db = 0;
  Mic_Start();
  return 0;
}

static void P_Stop(void)
{
  Mic_Stop();
  level_db = 0;
}

static float P_Db(float v)
{
  return 20.0f * log10f(v + 1e-6f);
}

/* dB -> 0..MX_H pixels against the reference */
static uint8_t P_Pixels(float db)
{
  if (db < (float)gate_db) return 0;
  float h = (db - (ref_db - RANGE_DB)) / RANGE_DB * (float)MX_H;
  return (uint8_t)(h <= 0.0f ? 0 : (h >= (float)MX_H ? MX_H : lroundf(h)));
}

static void P_Peak(int b, uint8_t h)
{
  if (h >= peak[b]) { peak[b] = h; peak_hold[b] = PEAK_HOLD; peak_tick[b] = 0; }
  else if (peak_hold[b]) peak_hold[b]--;
  else if (++peak_tick[b] >= PEAK_FALL) { peak_tick[b] = 0; if (peak[b]) peak[b]--; }
}

static void P_DrawBars(const float *band_db)
{
  const Rgb fg = Rgb_Make(0, 0, 255), top = Rgb_Make(255, 0, 0);
  Matrix_Clear();
  for (int b = 0; b < BANDS; b++)
  {
    uint8_t h = P_Pixels(band_db[b]);
    height[b] = (h >= height[b]) ? h : (uint8_t)(height[b] - 1u);     /* bars fall one pixel per frame */
    P_Peak(b, height[b]);
    for (int y = 0; y < height[b]; y++) Matrix_Set(b, y, fg);
    if (peak[b] > 0) Matrix_Set(b, peak[b] - 1, top);
  }
}

static void P_DrawVu(const float *band_db)
{
  const Rgb low = Rgb_Make(0, 0, 255), high = Rgb_Make(255, 0, 0);
  const int top_zone = MX_H - MX_H / 3;          /* rows above this are red */
  Matrix_Clear();
  for (int b = 0; b < BANDS; b++)
  {
    uint8_t h = P_Pixels(band_db[b]);
    height[b] = (h >= height[b]) ? h : (uint8_t)(height[b] - 1u);
    P_Peak(b, height[b]);
    for (int y = 0; y < height[b]; y++) Matrix_Set(b, y, y > top_zone ? high : low);
    if (peak[b] > 0 && peak[b] < MX_H) Matrix_Set(b, peak[b], peak[b] > top_zone ? high : low);
  }
}

static uint32_t P_Frame(uint32_t now)
{
  (void)now;
  if (Mic_Read(samples) != 0) return FX_SKIP | 2u;      /* next block not in yet */

  float mean = 0.0f;
  for (unsigned i = 0; i < N; i++) mean += (float)samples[i];
  mean /= (float)N;
  float energy = 0.0f;
  for (unsigned i = 0; i < N; i++)
  {
    float v = (float)samples[i] - mean;
    energy += v * v;
    re[i] = v * window[i];
    im[i] = 0.0f;
  }
  /* overall level (console "fx"), scaled like a full-scale FFT bin so it compares with the gate */
  level_db = (int)P_Db(sqrtf(energy / (float)N) * (float)N / 4.0f);

  float band_db[BANDS];
  float loudest = 0.0f;
  (void)Fft_Run(re, im, N);
  Fft_Magnitude(re, im, re, N);
  for (int b = 0; b < BANDS; b++)
  {
    unsigned lo = band_lo[b], hi = band_lo[b + 1];
    float m = 0.0f;
    for (unsigned k = lo; k <= hi && k < N / 2; k++) if (re[k] > m) m = re[k];
    band_db[b] = P_Db(m);
    if (b == 0 || band_db[b] > loudest) loudest = band_db[b];
  }
  /* automatic gain: follow the loudest value at once, fall back slowly */
  if (loudest > ref_db) ref_db = loudest;
  else ref_db -= REF_FALL_DB;
  if (ref_db < (float)gate_db + RANGE_DB / 2.0f) ref_db = (float)gate_db + RANGE_DB / 2.0f;

  if (mode == FX_EQ_BARS) P_DrawBars(band_db);
  else P_DrawVu(band_db);
  return 2u;
}

const FxOps fx_eq_ops = { P_Start, P_Frame, P_Stop, 0 };
