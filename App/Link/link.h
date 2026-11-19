/*
 * link.h - BeaconGalaxy side of the serial link to AudioGalaxy (USART2).
 * Sends GALAXY* commands and keeps what the audio board reports: whether it
 * is ready, the playlist, what is playing. See galaxy_link.h / docs/protocol.md.
 */
#ifndef APP_LINK_H
#define APP_LINK_H

#include "galaxy_link.h"
#include <stdint.h>

typedef struct {
  int  ready;             /* GALAXYREADY seen since boot */
  int  tracks;            /* from GALAXYREADY */
  int  playlist_count;    /* PLAYLIST lines received */
  int  playing;           /* index, -1 = none */
  int  repeat_ack;        /* REPEATSET seen */
  int  recallvol_ack;     /* RECALLVOL seen */
  unsigned long waveends;
  uint32_t last_playlist_tick;   /* when the last PLAYLIST line arrived */
  char last_error[GX_NAME_MAX + 1];
} LinkState;

typedef void (*LinkEventFn)(const GxMsg *m);   /* called in the App task for every event */

void Link_Init(LinkEventFn on_event);
void Link_Poll(void);                          /* App task */
void Link_Send(GxId id, long a);               /* any task */
void Link_SendRaw(const char *line);           /* console: a raw line (terminator added) */
const LinkState *Link_State(void);
const char *Link_TrackName(int index);         /* NULL if not received */
void Link_SetLog(int on);
int  Link_GetLog(void);

typedef struct { unsigned long rx_lines, tx_lines, rx_errors, rx_overruns; } LinkStats;
void Link_GetStats(LinkStats *out);

void APP_USART2_IRQ(void);

#endif
