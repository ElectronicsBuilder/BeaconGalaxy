/*
 * sdcard.h - the microSD card (SDIO 4-bit + DMA, FatFs with long names).
 *
 * The board has no card-detect switch (PC13 is a pulled-down, unconnected pin
 * so the CubeMX FatFs code always sees "present"). A card is found by mounting
 * it, and found missing when an access fails: code that reads the card passes
 * its FatFs result to Sd_Check(), and the App task (app_main.c) unmounts and
 * keeps trying to mount again until a card answers.
 */
#ifndef APP_SDCARD_H
#define APP_SDCARD_H

#include <stdint.h>

int  Sd_Mount(void);              /* 0 = mounted */
int  Sd_MountQuiet(void);         /* same, no warning when there is no card */
int  Sd_IsMounted(void);          /* mounted and answering: safe to start an access */
int  Sd_Lost(void);               /* still mounted, but stopped answering: App task unmounts */
const char *Sd_Root(void);         /* "0:/" */
void Sd_Unmount(void);
int  Sd_Info(uint32_t *total_kb, uint32_t *free_kb);

/* Any task: give the FatFs result of a card access; a disk error (card pulled
 * out?) is noted for the App task. Returns fresult unchanged. */
int  Sd_Check(int fresult);
int  Sd_TakeError(void);          /* App task: 1 once per noted error */
/* App task: asks the card for its status (one SEND_STATUS command, under the
 * FatFs volume lock). 0 = it answered, -1 = gone (or not mounted). */
int  Sd_Probe(void);

/* For the UI: set by the App task when no card answered for a while */
void Sd_SetMissing(int missing);
int  Sd_Missing(void);

#endif
