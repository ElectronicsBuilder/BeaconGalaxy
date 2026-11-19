/*
 * fx_theme.c - the animated themes, the plain colour and the test patterns.
 *
 *  FIRE      the original flame field (12 rows of value/hue masks), drawn over
 *            the full height
 *  METEOR    magenta meteors falling down the columns with random fading trails
 *            (the original ran one meteor along the first row only)
 *  BOUNCING  red, white and blue balls under gravity, losing energy at each
 *            bounce (the original used the first row only)
 *  PLASMA    four interfering sine waves, colours drifting through the rainbow
 *  STARS     a starfield: stars fade in and out at random places, white to blue
 *  RAIN      falling green code: drops of different speeds with fading trails
 *  SWIRL     a rainbow spiral turning around the centre
 *  LIFE      the Game of Life on a wrapped board, cells coloured by age;
 *            reseeded when it dies out or settles
 */
#include "fx_int.h"
#include "settings.h"
#include "cmsis_os2.h"
#include <string.h>
#include <math.h>

/* ---- FIRE ------------------------------------------------------------------ */

#define FIRE_H      12
#define FIRE_W      MX_W
#define FIRE_MS     40u
#define FIRE_STEP   30          /* % of a row the flames rise per frame */

static const uint8_t value_mask[FIRE_H][FIRE_W] = {
  { 32,  0,   0,   0,   0,   0,   0,   0,   0,   0,   32,  32,  0,   0,   0,   0,   0,   0,   0,   0,   0,   32  },
  { 64,  52,  0,   0,   0,   0,   0,   0,   0,   0,   64,  64,  0,   0,   0,   0,   0,   0,   0,   0,   52,  64  },
  { 96,  86,  52,  0,   32,  0,   0,   0,   0,   32,  96,  96,  32,  0,   0,   0,   0,   32,  0,   52,  89,  96  },
  { 128, 118, 86,  52,  64,  32,  0,   0,   32,  64,  128, 128, 64,  32,  0,   0,   32,  64,  52,  86,  118, 128 },
  { 160, 150, 118, 86,  96,  64,  32,  32,  64,  96,  160, 160, 96,  64,  32,  32,  64,  96,  86,  118, 150, 160 },
  { 160, 150, 150, 118, 96,  72,  32,  32,  64,  96,  160, 160, 96,  64,  32,  32,  64,  96,  118, 150, 150, 160 },
  { 168, 158, 150, 150, 104, 80,  40,  40,  72,  104, 168, 160, 104, 72,  40,  40,  72,  104, 150, 150, 158, 168 },
  { 176, 166, 158, 150, 112, 88,  48,  48,  80,  112, 176, 160, 112, 80,  48,  48,  80,  112, 150, 158, 166, 176 },
  { 184, 174, 166, 158, 120, 64,  56,  56,  88,  120, 184, 160, 88,  88,  56,  56,  88,  120, 158, 166, 174, 184 },
  { 192, 182, 174, 166, 128, 96,  64,  64,  96,  128, 192, 192, 128, 96,  64,  64,  96,  128, 166, 174, 182, 192 },
  { 255, 255, 182, 174, 160, 128, 96,  96,  128, 160, 255, 255, 160, 128, 96,  96,  128, 160, 174, 182, 255, 255 },
  { 255, 255, 255, 182, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 182, 255, 255, 255 }
};

static const uint8_t hue_mask[FIRE_H][FIRE_W] = {
  { 1, 3, 5, 7, 11, 19, 25, 25, 22, 11, 1, 1, 11, 19, 25, 25, 22, 11, 7, 5, 3, 1 },
  { 1, 2, 4, 6, 8, 13, 19, 25, 19, 8, 1, 1, 8, 13, 19, 25, 19, 8, 6, 4, 2, 1 },
  { 1, 2, 4, 6, 8, 13, 16, 19, 16, 8, 1, 1, 8, 13, 16, 19, 16, 8, 6, 4, 2, 1 },
  { 1, 2, 3, 4, 5, 11, 13, 13, 13, 5, 1, 1, 5, 11, 13, 13, 13, 5, 4, 3, 2, 1 },
  { 1, 2, 3, 4, 5, 11, 11, 11, 11, 5, 1, 1, 5, 11, 11, 11, 11, 5, 4, 3, 2, 1 },
  { 1, 2, 3, 4, 5, 9, 11, 11, 11, 5, 1, 1, 9, 11, 11, 11, 11, 5, 4, 3, 2, 1 },
  { 1, 1, 2, 3, 4, 8, 11, 11, 11, 4, 1, 1, 8, 11, 11, 11, 11, 4, 3, 2, 1, 1 },
  { 1, 1, 1, 1, 2, 7, 10, 10, 10, 3, 1, 1, 7, 10, 10, 10, 10, 3, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 2, 6, 9, 9, 9, 2, 1, 1, 6, 9, 9, 9, 9, 2, 1, 1, 1, 1 },
  { 0, 0, 0, 1, 1, 5, 8, 8, 5, 1, 0, 0, 1, 5, 8, 8, 5, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 5, 5, 1, 0, 0, 0, 0, 1, 5, 5, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 }
};

static uint8_t fire_value[FIRE_H][FIRE_W];
static uint8_t fire_line[FIRE_W];
static int fire_pcnt;

static void P_FireLine(void)
{
  for (int x = 0; x < FIRE_W; x++) fire_line[x] = (uint8_t)(64u + Fx_Random(192u));   /* 64..255 */
}

static void P_FireStart(void)
{
  memset(fire_value, 0, sizeof fire_value);
  P_FireLine();
  fire_pcnt = 0;
}

static uint8_t P_FireCell(int fy, int x)
{
  int v;
  if (fy == 0) v = ((100 - fire_pcnt) * fire_value[0][x] + fire_pcnt * fire_line[x]) / 100;
  else v = ((100 - fire_pcnt) * fire_value[fy][x] + fire_pcnt * fire_value[fy - 1][x]) / 100 - value_mask[fy][x];
  return (uint8_t)(v < 0 ? 0 : v);
}

static uint32_t P_FireFrame(void)
{
  if (fire_pcnt >= 100)
  {
    memmove(fire_value[1], fire_value[0], sizeof fire_value - sizeof fire_value[0]);   /* shift up */
    memcpy(fire_value[0], fire_line, sizeof fire_line);
    P_FireLine();
    fire_pcnt = 0;
  }
  for (int y = 0; y < MX_H; y++)
  {
    int fy = y * FIRE_H / MX_H;                 /* stretch the 12-row field */
    for (int x = 0; x < MX_W; x++) Matrix_Set(x, y, Hsv_ToRgb(hue_mask[fy][x], 255, P_FireCell(fy, x)));
  }
  fire_pcnt += FIRE_STEP;
  return FIRE_MS;
}

/* ---- METEOR ---------------------------------------------------------------- */

#define METEORS       7
#define METEOR_MS     30u
#define METEOR_DECAY  64u          /* /256 per frame, as the original */

typedef struct { int8_t x, y; uint8_t wait; } Meteor;
static Meteor meteor[METEORS];

static void P_MeteorSpawn(Meteor *m)
{
  m->x = (int8_t)Fx_Random(MX_W);
  m->y = (int8_t)(MX_H + Fx_Random(4u));
  m->wait = (uint8_t)Fx_Random(20u);
}

static void P_MeteorStart(void)
{
  for (int i = 0; i < METEORS; i++) P_MeteorSpawn(&meteor[i]);
}

static uint8_t P_Fade(uint8_t v)
{
  return v <= 10u ? 0u : (uint8_t)(v - (v * METEOR_DECAY) / 256u);
}

static uint32_t P_MeteorFrame(void)
{
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      if (Fx_Random(10u) <= 5u) continue;      /* random decay: fade about half the pixels */
      Rgb c = Matrix_Get(x, y);
      Matrix_Set(x, y, Rgb_Make(P_Fade(c.r), P_Fade(c.g), P_Fade(c.b)));
    }
  const Rgb head = Rgb_Make(255, 0, 255);
  for (int i = 0; i < METEORS; i++)
  {
    Meteor *m = &meteor[i];
    if (m->wait) { m->wait--; continue; }
    Matrix_Set(m->x, m->y, head);
    Matrix_Set(m->x, m->y + 1, head);
    if (--m->y < -2) P_MeteorSpawn(m);
  }
  return METEOR_MS;
}

/* ---- BOUNCING -------------------------------------------------------------- */

#define BALLS       3
#define BALL_MS     20u
#define GRAVITY     (-9.81f)

typedef struct {
  float impact;          /* velocity at the last bounce */
  float damping;
  uint32_t t0;           /* ms of the last bounce */
  float x, dx;           /* horizontal drift, columns per frame */
  Rgb color;
} Ball;
static Ball ball[BALLS];
static float impact_start;

static void P_BallsStart(uint32_t now)
{
  static const Rgb colors[BALLS] = { { 255, 0, 0 }, { 255, 255, 255 }, { 0, 0, 255 } };
  impact_start = sqrtf(-2.0f * GRAVITY * 1.0f);           /* from height 1 */
  for (int i = 0; i < BALLS; i++)
  {
    ball[i].impact = impact_start;
    ball[i].damping = 0.90f - (float)i / (float)(BALLS * BALLS);
    ball[i].t0 = now;
    ball[i].x = (float)(3 + i * 7);
    ball[i].dx = (i & 1) ? -0.15f : 0.2f;
    ball[i].color = colors[i];
  }
}

static uint32_t P_BallsFrame(uint32_t now)
{
  Matrix_Clear();
  for (int i = 0; i < BALLS; i++)
  {
    Ball *b = &ball[i];
    float t = (float)(now - b->t0) / 1000.0f;
    float h = 0.5f * GRAVITY * t * t + b->impact * t;
    if (h < 0.0f)
    {
      h = 0.0f;
      b->impact *= b->damping;
      b->t0 = now;
      if (b->impact < 0.15f * impact_start) b->impact = impact_start;   /* nearly at rest: throw it again */
    }
    b->x += b->dx;
    if (b->x < 0.0f) { b->x = 0.0f; b->dx = -b->dx; }
    if (b->x > (float)(MX_W - 2)) { b->x = (float)(MX_W - 2); b->dx = -b->dx; }
    int y = (int)lroundf(h * (float)(MX_H - 2));
    int x = (int)b->x;
    Matrix_Set(x, y, b->color);
    Matrix_Set(x + 1, y, b->color);
    Matrix_Set(x, y + 1, b->color);
    Matrix_Set(x + 1, y + 1, b->color);
  }
  return BALL_MS;
}


/* ---- PLASMA ------------------------------------------------------------------- */

#define PLASMA_MS 30u

static uint32_t P_PlasmaFrame(uint32_t now)
{
  const float t = (float)now / 1000.0f;
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      float dx = (float)x - 10.5f + 4.0f * sinf(t * 0.5f), dy = (float)y - 10.5f + 4.0f * cosf(t * 0.4f);
      float v = sinf((float)x * 0.35f + t) + sinf((float)y * 0.28f - t * 1.3f) +
                sinf(((float)x + (float)y) * 0.21f + t * 0.7f) + sinf(sqrtf(dx * dx + dy * dy) * 0.45f - t * 1.1f);
      uint8_t hue = (uint8_t)((int)((v + 4.0f) * 32.0f) + (int)(t * 12.0f));
      Matrix_Set(x, y, Hsv_ToRgb(hue, 255, 255));
    }
  return PLASMA_MS;
}

/* ---- STARS -------------------------------------------------------------------- */

#define STARS     36
#define STARS_MS  40u

typedef struct { int8_t x, y; uint8_t phase, speed, tint; } Star;
static Star star[STARS];

static void P_StarSpawn(Star *st)
{
  st->x = (int8_t)Fx_Random(MX_W);
  st->y = (int8_t)Fx_Random(MX_H);
  st->phase = 0;
  st->speed = (uint8_t)(2u + Fx_Random(6u));
  st->tint = (uint8_t)Fx_Random(3u);
}

static void P_StarsStart(void)
{
  for (int i = 0; i < STARS; i++)
  {
    P_StarSpawn(&star[i]);
    star[i].phase = (uint8_t)Fx_Random(256u);
  }
}

static uint32_t P_StarsFrame(void)
{
  Matrix_Clear();
  for (int i = 0; i < STARS; i++)
  {
    Star *st = &star[i];
    unsigned p = st->phase;                               /* 0..255: rise, then fall */
    uint8_t v = (uint8_t)(p < 128u ? p * 2u : (255u - p) * 2u);
    Rgb c = st->tint == 0 ? Rgb_Make(v, v, v)
          : st->tint == 1 ? Rgb_Make((uint8_t)(v / 2u), (uint8_t)(v / 2u), v)
          : Rgb_Make(v, v, (uint8_t)(v / 2u));
    Matrix_Set(st->x, st->y, c);
    if (p + st->speed > 255u) P_StarSpawn(st);
    else st->phase = (uint8_t)(p + st->speed);
  }
  return STARS_MS;
}

/* ---- RAIN --------------------------------------------------------------------- */

#define DROPS    12
#define RAIN_MS  25u

typedef struct { int8_t x; int16_t y16; uint8_t speed; } Drop;   /* y in 1/16 pixel */
static Drop drop[DROPS];

static void P_DropSpawn(Drop *d)
{
  d->x = (int8_t)Fx_Random(MX_W);
  d->y16 = (int16_t)((MX_H + (int)Fx_Random(8u)) * 16);
  d->speed = (uint8_t)(5u + Fx_Random(12u));
}

static void P_RainStart(void)
{
  for (int i = 0; i < DROPS; i++)
  {
    P_DropSpawn(&drop[i]);
    drop[i].y16 = (int16_t)(Fx_Random(MX_H * 2u) * 16u);
  }
}

static uint32_t P_RainFrame(void)
{
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      Rgb c = Matrix_Get(x, y);
      Matrix_Set(x, y, Rgb_Make((uint8_t)(c.r * 3u / 5u), (uint8_t)(c.g * 7u / 8u), (uint8_t)(c.b * 3u / 5u)));
    }
  for (int i = 0; i < DROPS; i++)
  {
    Drop *d = &drop[i];
    d->y16 = (int16_t)(d->y16 - d->speed);
    int y = d->y16 / 16;
    Matrix_Set(d->x, y, Rgb_Make(180, 255, 180));          /* bright head, green trail */
    if (y < 0) P_DropSpawn(d);
  }
  return RAIN_MS;
}

/* ---- SWIRL -------------------------------------------------------------------- */

#define SWIRL_MS 30u

static uint32_t P_SwirlFrame(uint32_t now)
{
  const float t = (float)now / 1000.0f;
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      float dx = (float)x - 10.5f, dy = (float)y - 10.5f;
      float angle = atan2f(dy, dx);                       /* -pi..pi */
      float dist = sqrtf(dx * dx + dy * dy);
      int hue = (int)(angle * 40.74f) + (int)(dist * 9.0f) - (int)(t * 90.0f);
      uint8_t v = (uint8_t)(dist > 15.0f ? 0 : 255 - (int)(dist * 6.0f));
      Matrix_Set(x, y, Hsv_ToRgb((uint8_t)hue, 255, v));
    }
  return SWIRL_MS;
}

/* ---- LIFE --------------------------------------------------------------------- */

#define LIFE_MS        150u
#define LIFE_MAX_GENS  600u         /* reseed even a busy board now and then */
#define LIFE_STILL     12u          /* generations without change (or period 2) */

static uint8_t life_age[MX_H][MX_W];     /* 0 = dead, else age (capped) */
static uint32_t life_hash[2];
static unsigned life_gens, life_still;
static uint8_t life_hue;

static void P_LifeSeed(void)
{
  memset(life_age, 0, sizeof life_age);
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
      if (Fx_Random(100u) < 32u) life_age[y][x] = 1;
  life_gens = 0;
  life_still = 0;
  life_hue = (uint8_t)Fx_Random(256u);
  memset(life_hash, 0, sizeof life_hash);
}

static uint32_t P_LifeFrame(void)
{
  static uint8_t next[MX_H][MX_W];
  uint32_t hash = 2166136261u;
  unsigned alive = 0;
  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      int n = 0;
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
          if ((dx || dy) && life_age[(y + dy + MX_H) % MX_H][(x + dx + MX_W) % MX_W]) n++;
      uint8_t a = life_age[y][x];
      int live = a ? (n == 2 || n == 3) : (n == 3);
      next[y][x] = live ? (uint8_t)(a < 250u ? a + 1u : a) : 0u;
      if (live)
      {
        alive++;
        hash = (hash ^ (uint32_t)(y * MX_W + x)) * 16777619u;
      }
    }
  memcpy(life_age, next, sizeof life_age);

  /* stuck: no change, a two-step blinker, or (almost) nothing left */
  if (hash == life_hash[0] || hash == life_hash[1]) life_still++;
  else life_still = 0;
  life_hash[1] = life_hash[0];
  life_hash[0] = hash;
  if (alive < 4u || life_still >= LIFE_STILL || ++life_gens >= LIFE_MAX_GENS) P_LifeSeed();

  for (int y = 0; y < MX_H; y++)
    for (int x = 0; x < MX_W; x++)
    {
      uint8_t a = life_age[y][x];
      if (!a)
      {
        Matrix_Set(x, y, RGB_OFF);
        continue;
      }
      /* newborn cells are nearly white, older ones settle into the board colour */
      uint8_t sat = (uint8_t)(a >= 8u ? 255u : 60u + a * 24u);
      Matrix_Set(x, y, Hsv_ToRgb((uint8_t)(life_hue + a * 3u), sat, 255));
    }
  return LIFE_MS;
}

/* ---- theme dispatch ----------------------------------------------------------- */

static const char *const theme_names[FX_THEME_COUNT] = {
  "FIRE", "METEOR", "BOUNCING", "PLASMA", "STARS", "RAIN", "SWIRL", "LIFE"
};

const char *Fx_ThemeName(int theme_index)
{
  return (theme_index >= 0 && theme_index < FX_THEME_COUNT) ? theme_names[theme_index] : "";
}

static int theme;

static int P_ThemeStart(int32_t arg)
{
  if (arg < 0 || arg >= FX_THEME_COUNT) return -1;
  theme = (int)arg;
  if (theme == FX_THEME_FIRE) P_FireStart();
  else if (theme == FX_THEME_METEOR) P_MeteorStart();
  else if (theme == FX_THEME_BOUNCING) P_BallsStart(osKernelGetTickCount());
  else if (theme == FX_THEME_STARS) P_StarsStart();
  else if (theme == FX_THEME_RAIN) P_RainStart();
  else if (theme == FX_THEME_LIFE) P_LifeSeed();
  return 0;
}

static uint32_t P_ThemeFrame(uint32_t now)
{
  switch (theme)
  {
    case FX_THEME_FIRE:     return P_FireFrame();
    case FX_THEME_METEOR:   return P_MeteorFrame();
    case FX_THEME_BOUNCING: return P_BallsFrame(now);
    case FX_THEME_PLASMA:   return P_PlasmaFrame(now);
    case FX_THEME_STARS:    return P_StarsFrame();
    case FX_THEME_RAIN:     return P_RainFrame();
    case FX_THEME_SWIRL:    return P_SwirlFrame(now);
    case FX_THEME_LIFE:     return P_LifeFrame();
    default:                return FX_END;
  }
}

const FxOps fx_theme_ops = { P_ThemeStart, P_ThemeFrame, NULL, 0 };

/* ---- plain colour ------------------------------------------------------------- */

static uint32_t P_ColorFrame(uint32_t now)
{
  (void)now;
  const Settings *s = Settings_Get();
  Matrix_Fill(Rgb_Make(s->red, s->green, s->blue));
  return 100u;                                  /* follows the Colors editor live */
}

static int P_ColorStart(int32_t arg)
{
  (void)arg;
  return 0;
}

const FxOps fx_color_ops = { P_ColorStart, P_ColorFrame, NULL, 1 };

/* ---- test patterns (console "mx test n") ---------------------------------------- */

static int test;
static unsigned test_step;

static int P_TestStart(int32_t arg)
{
  if (arg < 0 || arg > 2) return -1;
  test = (int)arg;
  test_step = 0;
  return 0;
}

static uint32_t P_TestFrame(uint32_t now)
{
  (void)now;
  Matrix_Clear();
  switch (test)
  {
    case 0:   /* chain walk: one white LED along the chain, shows the wiring order */
    {
      unsigned i = test_step % MX_LEDS;
      int y = (int)(i / MX_W), x = (int)(i % MX_W);
      if (y & 1) x = MX_W - 1 - x;
      Matrix_Set(x, y, Rgb_Make(255, 255, 255));
      test_step++;
      return 30u;
    }
    case 1:   /* orientation: red bottom row, green left column, blue top-right pixel */
      for (int i = 0; i < MX_W; i++) Matrix_Set(i, 0, Rgb_Make(255, 0, 0));
      for (int i = 0; i < MX_H; i++) Matrix_Set(0, i, Rgb_Make(0, 255, 0));
      Matrix_Set(MX_W - 1, MX_H - 1, Rgb_Make(0, 0, 255));
      return 200u;
    default:  /* rainbow */
      for (int y = 0; y < MX_H; y++)
        for (int x = 0; x < MX_W; x++)
          Matrix_Set(x, y, Hsv_ToRgb((uint8_t)((x + y) * 6 + test_step), 255, 255));
      test_step += 3;
      return 30u;
  }
}

const FxOps fx_test_ops = { P_TestStart, P_TestFrame, NULL, 0 };
