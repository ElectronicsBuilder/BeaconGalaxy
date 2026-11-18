/*
 * gif_names.c - see gif_names.h.
 */
#include "gif_names.h"
#include <string.h>
#include <ctype.h>

static int P_CaseCmp(const char *a, const char *b)
{
  while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { a++; b++; }
  return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int GifName_Parse(const char *fname, char *group, size_t group_size, unsigned *frame)
{
  size_t len = strlen(fname);
  if (len < 7 || P_CaseCmp(fname + len - 4, ".bmp") != 0) return 0;   /* "a-0.bmp" at least */
  size_t end = len - 4;                    /* index of '.' */
  size_t d = end;
  while (d > 0 && isdigit((unsigned char)fname[d - 1])) d--;
  if (d == end || d < 2 || fname[d - 1] != '-' || end - d > 5) return 0;
  size_t glen = d - 1;
  if (glen >= group_size || glen > GIF_NAME_MAX) return 0;
  unsigned n = 0;
  for (size_t i = d; i < end; i++) n = n * 10u + (unsigned)(fname[i] - '0');
  memcpy(group, fname, glen);
  group[glen] = '\0';
  *frame = n;
  return 1;
}

void GifTable_Clear(GifTable *t)
{
  t->count = 0;
}

int GifTable_Add(GifTable *t, const char *fname)
{
  char group[GIF_NAME_MAX + 1];
  unsigned frame;
  if (!GifName_Parse(fname, group, sizeof group, &frame)) return 0;
  for (unsigned i = 0; i < t->count; i++)
  {
    if (P_CaseCmp(t->gif[i].name, group) == 0)
    {
      if (frame + 1u > t->gif[i].frames) t->gif[i].frames = frame + 1u;
      return 1;
    }
  }
  if (t->count >= GIF_MAX) return 0;
  GifEntry *e = &t->gif[t->count++];
  strcpy(e->name, group);
  e->frames = frame + 1u;
  return 1;
}

void GifTable_Sort(GifTable *t)
{
  for (unsigned i = 1; i < t->count; i++)     /* insertion sort: at most 100 entries */
  {
    GifEntry e = t->gif[i];
    unsigned j = i;
    while (j > 0 && P_CaseCmp(t->gif[j - 1].name, e.name) > 0)
    {
      t->gif[j] = t->gif[j - 1];
      j--;
    }
    t->gif[j] = e;
  }
}
