/*
 * ui_bridge.c - see ui_bridge.h. Commands run in the App task, so the GUI task
 * never touches the UART, the EEPROM or FatFs.
 */
#include "ui_bridge.h"
#include "app_hal.h"
#include "rng.h"
#include "link.h"
#include "settings.h"
#include "gif_list.h"
#include "fx.h"
#include "eeprom.h"
#include "boot_fuse.h"
#include "sdcard.h"
#include "log.h"
#include "ff.h"
#include "cmsis_os2.h"
#include <string.h>
#include <strings.h>

#define QUEUE_LEN        16u
#define REBOOT_DELAY_MS  1000u

static const uint8_t boot_fuse[BOOT_FUSE_LEN] = BOOT_FUSE_SET;   /* GalaxyBoot: "update from SD" */
_Static_assert((int)UI_THEME_COUNT == (int)FX_THEME_COUNT, "UI and fx theme lists differ");
_Static_assert((int)UI_EQ_VU == (int)FX_EQ_VU, "UI and fx equalizer modes differ");

static osMessageQueueId_t queue;
static volatile UiUpgrade upgrade = UI_UPG_IDLE;
static uint32_t reboot_at;          /* tick, 0 = none */

void Ui_Init(void)
{
  queue = osMessageQueueNew(QUEUE_LEN, sizeof(UiCmd), NULL);
}

int Ui_Post(UiCmdId id, int32_t a)
{
  UiCmd c = { id, a };
  if (queue == NULL || osMessageQueuePut(queue, &c, 0, 0) != osOK)
  {
    LOG_W("ui: command %d dropped (queue full)", (int)id);
    return -1;
  }
  return 0;
}

/* ---- status for the GUI ---------------------------------------------------- */

int Ui_LinkReady(void)
{
  return Link_State()->ready;
}

int Ui_PlaylistComplete(void)
{
  const LinkState *s = Link_State();
  return s->tracks > 0 && s->playlist_count >= s->tracks;
}

unsigned Ui_TrackCount(void)
{
  const LinkState *s = Link_State();
  return (unsigned)((s->playlist_count < s->tracks) ? s->playlist_count : s->tracks);
}

const char *Ui_TrackName(unsigned i)
{
  const char *n = Link_TrackName((int)i);
  return n ? n : "";
}

unsigned Ui_GifCount(void)
{
  return GifList_Count();
}

const char *Ui_GifName(unsigned i)
{
  return GifList_Name(i);
}

const char *Ui_ThemeName(unsigned i)
{
  return Fx_ThemeName((int)i);
}

UiUpgrade Ui_UpgradeState(void)
{
  return upgrade;
}

uint32_t Ui_WaveEnds(void)
{
  return (uint32_t)Link_State()->waveends;
}

uint32_t Ui_Random(uint32_t n)
{
  uint32_t r;
  if (n == 0) return 0;
  if (HAL_RNG_GenerateRandomNumber(&hrng, &r) != HAL_OK) r = osKernelGetTickCount() * 2654435761u;
  return r % n;
}

uint32_t Ui_Millis(void)
{
  return osKernelGetTickCount();
}

/* ---- command execution (App task) ------------------------------------------ */

static int P_FindTrack(const char *name)
{
  unsigned n = Ui_TrackCount();
  for (unsigned i = 0; i < n; i++)
  {
    if (strcasecmp(Ui_TrackName(i), name) == 0) return (int)i;
  }
  return -1;
}

static void P_UpgradeCheck(void)
{
  char path[40];
  FILINFO fi;
  upgrade = UI_UPG_CHECKING;
  strcpy(path, Sd_Root());
  strcat(path, UI_UPGRADE_FILE);
  int found = Sd_IsMounted() && f_stat(path, &fi) == FR_OK && fi.fsize > 0;
  LOG_I("upgrade: %s %s", UI_UPGRADE_FILE, found ? "found" : "not found");
  upgrade = found ? UI_UPG_FILE_FOUND : UI_UPG_NO_FILE;
}

static void P_SetFuse(void)
{
  uint8_t check[sizeof boot_fuse];
  int ok = Ee_Write(BOOT_FUSE_ADDR, boot_fuse, sizeof boot_fuse) == 0 &&
           Ee_Read(BOOT_FUSE_ADDR, check, sizeof check) == 0 &&
           memcmp(check, boot_fuse, sizeof check) == 0;
  LOG_I("upgrade: boot fuse %s", ok ? "set" : "FAILED");
  upgrade = ok ? UI_UPG_FUSE_SET : UI_UPG_FUSE_FAILED;
}

static void P_Execute(const UiCmd *c)
{
  Settings *s = Settings_Get();
  switch (c->id)
  {
    /* matrix: the fx task owns it; a new request replaces whatever runs */
    case UI_GIF_PLAY:
      LOG_I("ui: GIF %s", Ui_GifName((unsigned)c->a));
      (void)Fx_Request(FX_GIF, c->a);
      break;
    case UI_THEME_PLAY:
      LOG_I("ui: theme %s", Fx_ThemeName((int)c->a));
      (void)Fx_Request(FX_THEME, c->a);
      break;
    case UI_EQ_START:
      LOG_I("ui: %s", c->a == UI_EQ_VU ? "VU meter" : "equalizer");
      (void)Fx_Request(FX_EQ, c->a);
      break;
    case UI_GIF_STOP:
    case UI_THEME_STOP:
    case UI_EQ_STOP:
      (void)Fx_Request(FX_OFF, 0);
      break;
    case UI_COLOR_PREVIEW: (void)Fx_Request(FX_COLOR, 0); break;

    case UI_SETTINGS_SAVE: (void)Settings_Save(); break;

    case UI_AUDIO_GET_PLAYLIST: Link_Send(GX_GETPLAYLIST, 0); break;
    case UI_AUDIO_PLAY:    Link_Send(GX_PLAY, c->a); break;
    case UI_AUDIO_STOP:    Link_Send(GX_STOP, 0); break;
    case UI_AUDIO_START:   Link_Send(GX_START, 0); break;
    case UI_AUDIO_PAUSE:   Link_Send(GX_PAUSE, 0); break;
    case UI_AUDIO_RESUME:  Link_Send(GX_RESUME, 0); break;
    case UI_AUDIO_VOLUME:  Link_Send(GX_VOL, c->a); break;
    case UI_AUDIO_RECALL:
      Link_Send(GX_RECALLVOL, s->volume);
      Link_Send(s->repeat ? GX_REPEATON : GX_REPEATOFF, 0);
      break;
    case UI_AUDIO_REPEAT:  Link_Send(c->a ? GX_REPEATON : GX_REPEATOFF, 0); break;
    case UI_RECORD_START:  Link_Send(GX_RECORD, 0); break;
    case UI_RECORD_STOP:   Link_Send(GX_STOPRECORD, 0); break;
    case UI_RECORD_PLAY:
    {
      int i = P_FindTrack("rec.wav");
      if (i >= 0) Link_Send(GX_PLAY, i);
      else LOG_W("ui: rec.wav is not in the playlist");
      break;
    }

    case UI_UPGRADE_CHECK:    P_UpgradeCheck(); break;
    case UI_UPGRADE_SET_FUSE: P_SetFuse(); break;
    case UI_REBOOT:
      LOG_I("ui: reboot");
      reboot_at = osKernelGetTickCount() + REBOOT_DELAY_MS;
      if (reboot_at == 0) reboot_at = 1;
      break;
  }
}

void Ui_Poll(void)
{
  UiCmd c;
  while (queue && osMessageQueueGet(queue, &c, NULL, 0) == osOK) P_Execute(&c);
  if (reboot_at && (int32_t)(osKernelGetTickCount() - reboot_at) >= 0) NVIC_SystemReset();
}
