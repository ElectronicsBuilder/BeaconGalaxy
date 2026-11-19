/*
 * sdcard.c - the microSD card. See sdcard.h.
 */
#include "sdcard.h"
#include "fatfs.h"
#include "log.h"
#include "sdio.h"

static int mounted;
static volatile int error_noted;
static volatile int missing;

static int P_Mount(int quiet);

int Sd_Mount(void)      { return P_Mount(0); }
int Sd_MountQuiet(void) { return P_Mount(1); }

static int P_Mount(int quiet)
{
  if (mounted) return 0;
  FRESULT r = f_mount(&SDFatFS, SDPath, 1);
  if (r != FR_OK)
  {
    /* After a card swap the SDIO/HAL state still describes the old card:
     * reset the peripheral so the next mount starts the card from scratch. */
    HAL_SD_DeInit(&hsd);
    r = f_mount(&SDFatFS, SDPath, 1);
  }
  if (r != FR_OK)
  {
    if (!quiet) LOG_W("SD card: mount failed (%d)", (int)r);
    return -1;
  }
  mounted = 1;
  return 0;
}

int Sd_IsMounted(void) { return mounted; }
const char *Sd_Root(void) { return SDPath; }

void Sd_Unmount(void)
{
  if (!mounted) return;
  f_mount(NULL, SDPath, 0);
  mounted = 0;
}

int Sd_Check(int fresult)
{
  if (fresult == FR_DISK_ERR || fresult == FR_NOT_READY) error_noted = 1;
  return fresult;
}

int Sd_TakeError(void)
{
  if (!error_noted) return 0;
  error_noted = 0;
  return 1;
}

int Sd_Probe(void)
{
  if (!mounted) return -1;
  if (!ff_req_grant(SDFatFS.sobj)) return 0;          /* busy for long: someone is using it, so it is there */
  HAL_SD_CardStateTypeDef st = HAL_SD_GetCardState(&hsd);
  ff_rel_grant(SDFatFS.sobj);
  return (st == HAL_SD_CARD_TRANSFER || st == HAL_SD_CARD_SENDING || st == HAL_SD_CARD_RECEIVING ||
          st == HAL_SD_CARD_PROGRAMMING) ? 0 : -1;
}

void Sd_SetMissing(int m) { missing = m; }
int  Sd_Missing(void)     { return missing; }

int Sd_Info(uint32_t *total_kb, uint32_t *free_kb)
{
  FATFS *fs;
  DWORD free_clusters;
  if (!mounted || f_getfree(SDPath, &free_clusters, &fs) != FR_OK) return -1;
  uint32_t kb_per_cluster = fs->csize / 2u;     /* 512-byte sectors */
  *total_kb = (uint32_t)(fs->n_fatent - 2u) * kb_per_cluster;
  *free_kb = (uint32_t)free_clusters * kb_per_cluster;
  return 0;
}
