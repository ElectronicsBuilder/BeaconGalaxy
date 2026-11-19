/*
 * sdcard.c - the microSD card. See sdcard.h.
 */
#include "sdcard.h"
#include "fatfs.h"
#include "log.h"
#include "sdio.h"

static int mounted;

int Sd_Mount(void)
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
    LOG_W("SD card: mount failed (%d)", (int)r);
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
