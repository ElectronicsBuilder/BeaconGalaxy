/*
 * sdcard.h - the microSD card (SDIO 4-bit + DMA, FatFs with long names).
 */
#ifndef APP_SDCARD_H
#define APP_SDCARD_H

#include <stdint.h>

int  Sd_Mount(void);              /* 0 = mounted */
int  Sd_IsMounted(void);
const char *Sd_Root(void);         /* "0:/" */
void Sd_Unmount(void);
int  Sd_Info(uint32_t *total_kb, uint32_t *free_kb);

#endif
