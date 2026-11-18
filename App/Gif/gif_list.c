/*
 * gif_list.c - see gif_list.h. The table is built in a scratch copy and
 * published by setting the count last, so readers never see a half-built list.
 */
#include "gif_list.h"
#include "sdcard.h"
#include "log.h"
#include "ff.h"
#include "cmsis_os2.h"

static GifTable table;
static volatile unsigned published;

int GifList_Scan(void)
{
  published = 0;
  if (!Sd_IsMounted()) return 0;
  DIR dir;
  FILINFO fi;
  uint32_t t0 = osKernelGetTickCount();
  GifTable_Clear(&table);
  if (f_opendir(&dir, Sd_Root()) != FR_OK) return 0;
  while (f_readdir(&dir, &fi) == FR_OK && fi.fname[0] != '\0')
  {
    if (!(fi.fattrib & AM_DIR)) (void)GifTable_Add(&table, fi.fname);
  }
  f_closedir(&dir);
  GifTable_Sort(&table);
  published = table.count;
  LOG_I("GIFs: %u found in %lu ms", table.count, (unsigned long)(osKernelGetTickCount() - t0));
  return (int)table.count;
}

unsigned GifList_Count(void)
{
  return published;
}

const char *GifList_Name(unsigned i)
{
  return (i < published) ? table.gif[i].name : "";
}

unsigned GifList_Frames(unsigned i)
{
  return (i < published) ? table.gif[i].frames : 0u;
}
