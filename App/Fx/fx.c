/*
 * fx.c - the fx task: owns the matrix, runs the requested effect.
 */
#include "fx_int.h"
#include "settings.h"
#include "app_hal.h"
#include "log.h"
#include "watchdog.h"
#include "cmsis_os2.h"
#include <stddef.h>

#define QUEUE_LEN   8u
#define IDLE_WAIT   500u          /* wake up now and then to report to the watchdog */

typedef struct { FxKind kind; int32_t arg; } FxReq;

static osMessageQueueId_t queue;
static volatile FxStatus status;
static const FxOps *ops;
static uint32_t rng_state;
static int wdg_id = -1;

static const char *const kind_names[FX_KIND_COUNT] = { "off", "color", "gif", "theme", "eq", "test" };

const char *Fx_KindName(FxKind k)
{
  return (k < FX_KIND_COUNT) ? kind_names[k] : "?";
}

uint32_t Fx_Random(uint32_t n)
{
  /* xorshift32, seeded once from the hardware RNG */
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return n ? rng_state % n : 0u;
}

void Fx_CountError(void)
{
  status.errors++;
}

static const FxOps *P_Ops(FxKind k)
{
  switch (k)
  {
    case FX_COLOR: return &fx_color_ops;
    case FX_GIF:   return &fx_gif_ops;
    case FX_THEME: return &fx_theme_ops;
    case FX_EQ:    return &fx_eq_ops;
    case FX_TEST:  return &fx_test_ops;
    default:       return NULL;
  }
}

static void P_Stop(void)
{
  if (ops && ops->stop) ops->stop();
  ops = NULL;
  status.kind = FX_OFF;
  status.arg = 0;
  Matrix_Clear();
  (void)Matrix_Show(0);
}

static void P_Start(const FxReq *r)
{
  P_Stop();
  const FxOps *o = P_Ops(r->kind);
  if (o == NULL) return;                     /* FX_OFF */
  status.frames = 0;
  status.errors = 0;
  Matrix_Clear();
  if (o->start(r->arg) != 0)
  {
    LOG_W("fx: %s %ld cannot start", Fx_KindName(r->kind), (long)r->arg);
    if (o->stop) o->stop();
    return;
  }
  ops = o;
  status.kind = r->kind;
  status.arg = r->arg;
}

static void P_Task(void *arg)
{
  (void)arg;
  uint32_t seed = 0;
  for (int i = 0; i < 100 && !(RNG->SR & RNG_SR_DRDY); i++) { }
  seed = (RNG->SR & RNG_SR_DRDY) ? RNG->DR : osKernelGetTickCount() * 2654435761u;
  rng_state = seed ? seed : 0x1234567u;

  Matrix_Init();
  uint32_t next = 0;
  for (;;)
  {
    Wdg_Alive(wdg_id);
    uint32_t wait = IDLE_WAIT;
    if (ops)
    {
      int32_t left = (int32_t)(next - osKernelGetTickCount());
      wait = left > 0 ? (uint32_t)left : 0u;
    }
    FxReq r;
    if (osMessageQueueGet(queue, &r, NULL, wait) == osOK)
    {
      P_Start(&r);
      next = osKernelGetTickCount();
      continue;
    }
    if (ops == NULL) continue;

    uint32_t t0 = osKernelGetTickCount();
    uint32_t d = ops->frame(t0);
    if (d == FX_END)
    {
      LOG_I("fx: %s finished", Fx_KindName(status.kind));
      P_Stop();
      continue;
    }
    if (!(d & FX_SKIP))
    {
      uint8_t level = ops->full_level ? 255u : Settings_Get()->intensity;
      if (Matrix_Show(level) != 0) Fx_CountError();
      status.frames++;
      status.last_frame_ms = osKernelGetTickCount() - t0;
    }
    d &= ~FX_SKIP;
    next = t0 + (d ? d : 1u);
    if ((int32_t)(osKernelGetTickCount() - next) > 0) next = osKernelGetTickCount();   /* fell behind */
  }
}

void Fx_Init(void)
{
  static const osThreadAttr_t attr = {
    .name = "fx",
    .stack_size = 3072,
    .priority = osPriorityBelowNormal,
  };
  queue = osMessageQueueNew(QUEUE_LEN, sizeof(FxReq), NULL);
  wdg_id = Wdg_Register("fx");
  if (queue == NULL || osThreadNew(P_Task, NULL, &attr) == NULL) LOG_E("fx task not started");
}

int Fx_Request(FxKind kind, int32_t arg)
{
  FxReq r = { kind, arg };
  if (queue == NULL || osMessageQueuePut(queue, &r, 0, 0) != osOK)
  {
    LOG_W("fx: request dropped (queue full)");
    return -1;
  }
  return 0;
}

void Fx_GetStatus(FxStatus *out)
{
  out->kind = status.kind;
  out->arg = status.arg;
  out->frames = status.frames;
  out->errors = status.errors;
  out->last_frame_ms = status.last_frame_ms;
}
