/*
 * gif_names.h - GIF frame file names: "<NAME>-<n>.bmp" in the SD root
 * (see SD CARD/ANIMATION.txt). Pure functions, covered by the host tests.
 */
#ifndef APP_GIF_NAMES_H
#define APP_GIF_NAMES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GIF_NAME_MAX  16    /* group name characters (without the terminator) */
#define GIF_MAX       100   /* groups kept */

/* 1 if fname is "<group>-<digits>.bmp" (extension in any case); fills group and frame. */
int GifName_Parse(const char *fname, char *group, size_t group_size, unsigned *frame);

typedef struct {
  char name[GIF_NAME_MAX + 1];
  unsigned frames;          /* highest frame number + 1 */
} GifEntry;

typedef struct {
  GifEntry gif[GIF_MAX];
  unsigned count;
} GifTable;

void GifTable_Clear(GifTable *t);
/* Adds one directory entry; ignores names that are not GIF frames. Returns 1 if used. */
int  GifTable_Add(GifTable *t, const char *fname);
void GifTable_Sort(GifTable *t);    /* by name, case-insensitive */

#ifdef __cplusplus
}
#endif

#endif
