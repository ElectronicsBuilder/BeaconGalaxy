/*
 * gif_list.h - the GIFs found on the SD card (scanned once by the App task).
 */
#ifndef APP_GIF_LIST_H
#define APP_GIF_LIST_H

#include "gif_names.h"

#ifdef __cplusplus
extern "C" {
#endif

int GifList_Scan(void);                 /* App task; returns the number of GIFs */
unsigned GifList_Count(void);           /* any task, after the scan */
const char *GifList_Name(unsigned i);   /* "" if out of range */
unsigned GifList_Frames(unsigned i);

#ifdef __cplusplus
}
#endif

#endif
