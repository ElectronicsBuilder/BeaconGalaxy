/*
 * link.c - BeaconGalaxy side of the serial link. See link.h.
 */
#include "link.h"
#include "uart_io.h"
#include "log.h"
#include "cmsis_os2.h"
#include <string.h>

static UartIo *uart;
static GxLineAsm rx_line;
static osMutexId_t tx_lock;
static int log_lines;
static LinkStats stats;
static LinkState state = { .playing = -1 };
static char names[GX_PLAYLIST_MAX][GX_NAME_MAX + 1];
static LinkEventFn event_fn;

void Link_Init(LinkEventFn on_event)
{
  event_fn = on_event;
  uart = UartIo_Start(USART2);
  tx_lock = osMutexNew(NULL);
}

void Link_SetLog(int on) { log_lines = on; }
int  Link_GetLog(void)   { return log_lines; }
const LinkState *Link_State(void) { return &state; }

const char *Link_TrackName(int i)
{
  return (i >= 0 && i < state.playlist_count && names[i][0]) ? names[i] : NULL;
}

void Link_GetStats(LinkStats *out)
{
  stats.rx_overruns = UartIo_RxOverruns(uart);
  *out = stats;
}

static void P_Write(const char *line, size_t len)
{
  if (tx_lock) osMutexAcquire(tx_lock, osWaitForever);
  UartIo_Write(uart, line, len);
  stats.tx_lines++;
  if (tx_lock) osMutexRelease(tx_lock);
  if (log_lines)
  {
    char shown[GX_LINE_MAX + 1];
    size_t n = len;
    while (n > 0 && (line[n - 1] == '\r' || line[n - 1] == '\n')) n--;
    if (n > GX_LINE_MAX) n = GX_LINE_MAX;
    memcpy(shown, line, n);
    shown[n] = '\0';
    LOG_I("link> %s", shown);
  }
}

void Link_Send(GxId id, long a)
{
  char line[GX_LINE_MAX + 4];
  size_t n = Gx_FormatId(id, a, 0, NULL, line, sizeof line);
  if (n > 0) P_Write(line, n);
}

void Link_SendRaw(const char *text)
{
  char line[GX_LINE_MAX + 4];
  size_t n = strlen(text);
  if (n > GX_LINE_MAX) n = GX_LINE_MAX;
  memcpy(line, text, n);
  line[n++] = '\r';
  line[n++] = '\n';
  P_Write(line, n);
}

static void P_Handle(const GxMsg *m)
{
  switch (m->id)
  {
    case GX_READY:
      if (state.tracks != (int)m->a) state.playlist_count = 0;   /* list changed: fetch again */
      state.ready = 1;
      state.tracks = (int)m->a;
      break;
    case GX_PLAYLIST:
      if (m->a < GX_PLAYLIST_MAX)
      {
        strcpy(names[m->a], m->text);
        if (m->a + 1 > state.playlist_count) state.playlist_count = (int)m->a + 1;
        state.tracks = (int)m->b;
      }
      break;
    case GX_EV_PLAYING:   state.playing = (int)m->a; break;
    case GX_EV_STOPPED:   state.playing = -1; break;
    case GX_EV_WAVEEND:   state.playing = -1; state.waveends++; break;
    case GX_EV_REPEATSET: state.repeat_ack = 1; break;
    case GX_EV_RECALLVOL: state.recallvol_ack = 1; break;
    case GX_EV_NOUSB:     state.tracks = 0; state.playlist_count = 0; state.playing = -1; break;
    case GX_EV_ERR:
      strcpy(state.last_error, m->text);
      LOG_W("audio board: %s", m->text);
      break;
    default:
      break;
  }
  if (event_fn) event_fn(m);
}

void Link_Poll(void)
{
  uint8_t b;
  while (UartIo_Read(uart, &b))
  {
    if (!Gx_LineFeed(&rx_line, (char)b)) continue;
    if (log_lines) LOG_I("link< %s", rx_line.line);
    GxMsg m;
    GxParseResult r = Gx_Parse(rx_line.line, &m);
    if (r == GX_PARSE_EMPTY) continue;
    stats.rx_lines++;
    if (r != GX_PARSE_OK || m.id < GX_READY) { stats.rx_errors++; continue; }
    P_Handle(&m);
  }
}

void APP_USART2_IRQ(void)
{
  UartIo_Irq(uart);
}
