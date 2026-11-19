/*
 * commands.c - BeaconGalaxy console commands (USART1 via the FT232, 115200,
 * or tools/swd_console.py over the ST-LINK).
 */
#include "commands.h"
#include "cli.h"
#include "log.h"
#include "version.h"
#include "app_hal.h"
#include "i2c.h"
#include "buttons.h"
#include "leds.h"
#include "oled.h"
#include "eeprom.h"
#include "sdcard.h"
#include "link.h"
#include "status_leds.h"
#include "fx.h"
#include "watchdog.h"
#include "gif_list.h"
#include "ws2812.h"
#include "mic.h"
#include <strings.h>
#include "ff.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int C_Ver(int argc, char **argv)
{
  (void)argc; (void)argv;
  Cli_Printf("name=%s", APP_NAME);
  Cli_Printf("version=%s", APP_VERSION);
  Cli_Printf("board=%s", APP_BOARD);
  Cli_Printf("built=%s %s", __DATE__, __TIME__);
  Cli_Printf("sysclk=%lu", (unsigned long)HAL_RCC_GetSysClockFreq());
  Cli_Printf("vtor=0x%08lX", (unsigned long)SCB->VTOR);
  Cli_Printf("last_reset=%s", Wdg_ResetCause());
  return 0;
}

static int C_I2cScan(int argc, char **argv)
{
  (void)argc; (void)argv;
  int found = 0;
  for (uint16_t a = 0x08; a < 0x78; a++)
  {
    if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(a << 1), 2, 5) == HAL_OK)
    {
      Cli_Printf("device=0x%02X (8-bit 0x%02X)", a, a << 1);
      found++;
    }
  }
  Cli_Printf("found=%d", found);
  return 0;
}

/* ee read <addr> [len] | ee write <addr> <byte>... */
static int C_Ee(int argc, char **argv)
{
  long addr, n = 16;
  if (argc < 3 || Cli_ParseInt(argv[2], 0, EE_SIZE - 1, &addr) != 0)
    return Cli_Error("usage: ee read <addr> [len] | ee write <addr> <byte>...");
  if (strcmp(argv[1], "read") == 0)
  {
    if (argc > 3 && Cli_ParseInt(argv[3], 1, 256, &n) != 0) return Cli_Error("len is 1..256");
    uint8_t buf[256];
    if (Ee_Read((uint32_t)addr, buf, (size_t)n) != 0) return Cli_Error("EEPROM read failed");
    for (long i = 0; i < n; i += 16)
    {
      char line[80];
      int pos = snprintf(line, sizeof line, "%04lX:", (unsigned long)(addr + i));
      for (long j = i; j < n && j < i + 16; j++) pos += snprintf(line + pos, sizeof line - (size_t)pos, " %02X", buf[j]);
      Cli_Printf("%s", line);
    }
    return 0;
  }
  if (strcmp(argv[1], "write") == 0)
  {
    uint8_t buf[CLI_MAX_ARGS];
    int count = 0;
    for (int i = 3; i < argc; i++)
    {
      long v;
      if (Cli_ParseInt(argv[i], 0, 255, &v) != 0) return Cli_Error("bytes are 0..255");
      buf[count++] = (uint8_t)v;
    }
    if (count == 0) return Cli_Error("no data");
    if (Ee_Write((uint32_t)addr, buf, (size_t)count) != 0) return Cli_Error("EEPROM write failed");
    return 0;
  }
  return Cli_Error("usage: ee read|write ...");
}

/* sd [ls [path] | info | mount] */
static int C_Sd(int argc, char **argv)
{
  const char *sub = argc > 1 ? argv[1] : "info";
  if (strcmp(sub, "mount") == 0)
  {
    Sd_Unmount();
    return Sd_Mount() == 0 ? 0 : Cli_Error("mount failed");
  }
  if (!Sd_IsMounted() && Sd_Mount() != 0) return Cli_Error("no SD card");
  if (strcmp(sub, "info") == 0)
  {
    uint32_t total, free_kb;
    if (Sd_Info(&total, &free_kb) != 0) return Cli_Error("cannot read the card");
    Cli_Printf("total_kb=%lu", (unsigned long)total);
    Cli_Printf("free_kb=%lu", (unsigned long)free_kb);
    return 0;
  }
  if (strcmp(sub, "ls") == 0)
  {
    char path[64];
    snprintf(path, sizeof path, "%s%s", Sd_Root(), argc > 2 ? argv[2] : "");
    const char *pattern = argc > 3 ? argv[3] : "*";
    DIR dir;
    FILINFO fno;
    if (f_findfirst(&dir, &fno, path, pattern) != FR_OK) return Cli_Error("cannot open %s", path);
    int files = 0;
    while (fno.fname[0] != '\0')
    {
      if (files < 60) Cli_Printf("%s%-24s %lu", (fno.fattrib & AM_DIR) ? "[d] " : "    ", fno.fname,
                                 (unsigned long)fno.fsize);
      files++;
      if (f_findnext(&dir, &fno) != FR_OK) break;
    }
    f_closedir(&dir);
    if (files > 60) Cli_Printf("... (%d more)", files - 60);
    Cli_Printf("files=%d", files);
    return 0;
  }
  if (strcmp(sub, "mv") == 0 || strcmp(sub, "rm") == 0)
  {
    char a[80], b[80];
    if (argc < (sub[0] == 'm' ? 4 : 3)) return Cli_Error("usage: sd mv <from> <to> | sd rm <file>");
    snprintf(a, sizeof a, "%s%s", Sd_Root(), argv[2]);
    FRESULT r;
    if (sub[0] == 'm')
    {
      snprintf(b, sizeof b, "%s%s", Sd_Root(), argv[3]);
      r = f_rename(a, b);
    }
    else
    {
      r = f_unlink(a);
    }
    return r == FR_OK ? 0 : Cli_Error("failed (FatFs error %d)", (int)r);
  }
  return Cli_Error("usage: sd [info|ls [dir] [pattern]|mount|mv|rm]");
}

static int C_Key(int argc, char **argv)
{
  if (argc < 2) return Cli_Error("usage: key up|down|back|enter");
  for (int i = 1; i < argc; i++)
  {
    Key k = Buttons_FromName(argv[i]);
    if (k == KEY_NONE) return Cli_Error("unknown key '%s'", argv[i]);
    Buttons_Inject(k);
  }
  return 0;
}

static int C_Led(int argc, char **argv)
{
  if (argc < 3) return Cli_Error("usage: led green|orange|red|blue on|off");
  int led = Leds_FromName(argv[1]);
  if (led < 0) return Cli_Error("unknown led");
  StatusLeds_Hold(10000u);       /* keep the LED effect off them for 10 s */
  Leds_Set((Led)led, strcmp(argv[2], "on") == 0);
  return 0;
}

/* oled test patterns (until the UI owns the screen) */
static int C_Oled(int argc, char **argv)
{
  static uint8_t fb[OLED_FB_SIZE];
  const char *sub = argc > 1 ? argv[1] : "border";
  memset(fb, 0, sizeof fb);
  if (strcmp(sub, "contrast") == 0)
  {
    long c;
    if (argc < 3 || Cli_ParseInt(argv[2], 0, 255, &c) != 0) return Cli_Error("contrast 0..255");
    Oled_SetContrast((uint8_t)c);
    return 0;
  }
  if (strcmp(sub, "fill") == 0) memset(fb, 0xFF, sizeof fb);
  else if (strcmp(sub, "checker") == 0)
  {
    for (int y = 0; y < OLED_HEIGHT; y++)
      for (int b = 0; b < OLED_WIDTH / 8; b++) fb[y * 8 + b] = ((y / 8 + b) & 1) ? 0xFF : 0x00;
  }
  else if (strcmp(sub, "border") == 0)
  {
    /* frame + a filled 8x8 block in the top-left corner (orientation check) */
    for (int x = 0; x < OLED_WIDTH; x++)
    {
      fb[x / 8] |= (uint8_t)(0x80u >> (x & 7));
      fb[(OLED_HEIGHT - 1) * 8 + x / 8] |= (uint8_t)(0x80u >> (x & 7));
    }
    for (int y = 0; y < OLED_HEIGHT; y++)
    {
      fb[y * 8] |= 0x80u;
      fb[y * 8 + 7] |= 0x01u;
    }
    for (int y = 2; y < 10; y++) fb[y * 8] |= 0x3Fu;
  }
  else if (strcmp(sub, "clear") != 0) return Cli_Error("usage: oled border|fill|checker|clear|contrast <n>");
  Oled_Flush(fb);
  return 0;
}

static int C_Gx(int argc, char **argv)
{
  if (argc < 2) return Cli_Error("usage: gx GALAXYPLAY 3");
  char line[GX_LINE_MAX + 1];
  size_t pos = 0;
  for (int i = 1; i < argc && pos < GX_LINE_MAX; i++)
  {
    int quote = strchr(argv[i], ' ') != NULL;
    pos += (size_t)snprintf(line + pos, sizeof line - pos, quote ? "%s\"%s\"" : "%s%s",
                            i > 1 ? " " : "", argv[i]);
  }
  Link_SendRaw(line);
  return 0;
}

static int C_Link(int argc, char **argv)
{
  if (argc >= 3 && strcmp(argv[1], "log") == 0)
  {
    Link_SetLog(strcmp(argv[2], "on") == 0);
    return 0;
  }
  const LinkState *s = Link_State();
  LinkStats st;
  Link_GetStats(&st);
  Cli_Printf("audio_ready=%d", s->ready);
  Cli_Printf("tracks=%d", s->tracks);
  Cli_Printf("playlist_received=%d", s->playlist_count);
  Cli_Printf("playing=%d", s->playing);
  Cli_Printf("log=%s", Link_GetLog() ? "on" : "off");
  Cli_Printf("rx_lines=%lu tx_lines=%lu rx_errors=%lu rx_overruns=%lu",
             st.rx_lines, st.tx_lines, st.rx_errors, st.rx_overruns);
  return 0;
}

static int C_Playlist(int argc, char **argv)
{
  (void)argc; (void)argv;
  const LinkState *s = Link_State();
  if (s->playlist_count == 0) return Cli_Error("no playlist yet (gx GALAXYGETPLAYLIST)");
  for (int i = 0; i < s->playlist_count; i++)
  {
    const char *n = Link_TrackName(i);
    Cli_Printf("%3d %s", i, n ? n : "?");
  }
  return 0;
}

static int C_Tasks(int argc, char **argv)
{
  (void)argc; (void)argv;
  Cli_Printf("heap_free=%u", (unsigned)xPortGetFreeHeapSize());
  Cli_Printf("heap_min=%u", (unsigned)xPortGetMinimumEverFreeHeapSize());
  TaskStatus_t list[12];
  UBaseType_t n = uxTaskGetSystemState(list, 12, NULL);
  for (UBaseType_t i = 0; i < n; i++)
  {
    Cli_Printf("task=%-12s prio=%lu stack_free=%u", list[i].pcTaskName,
               (unsigned long)list[i].uxCurrentPriority, (unsigned)(list[i].usStackHighWaterMark * 4u));
  }
  return 0;
}

static int C_Log(int argc, char **argv)
{
  static const char *names[] = { "error", "warn", "info", "debug" };
  if (argc >= 2)
  {
    for (int i = 0; i < 4; i++)
    {
      if (strcmp(argv[1], names[i]) == 0) { log_level = (LogLevel)i; return 0; }
    }
    return Cli_Error("usage: log error|warn|info|debug");
  }
  Cli_Printf("log=%s", names[log_level]);
  return 0;
}

static int C_Reset(int argc, char **argv)
{
  (void)argc; (void)argv;
  Cli_Printf("restarting");
  osDelay(50);
  NVIC_SystemReset();
  return 0;
}

/* index from a number or a name; -1 if neither */
static int P_FindIndex(const char *arg, unsigned count, const char *(*name)(unsigned))
{
  long v;
  if (Cli_ParseInt(arg, 0, (long)count - 1, &v) == 0) return (int)v;
  for (unsigned i = 0; i < count; i++)
    if (strcasecmp(arg, name(i)) == 0) return (int)i;
  return -1;
}

static const char *P_ThemeName(unsigned i) { return Fx_ThemeName((int)i); }

static int C_Fx(int argc, char **argv)
{
  if (argc < 2)
  {
    FxStatus st;
    unsigned long frames, irqs;
    Fx_GetStatus(&st);
    Ws2812_Stats(&frames, &irqs);
    Cli_Printf("effect=%s %ld", Fx_KindName(st.kind), (long)st.arg);
    Cli_Printf("frames=%lu errors=%lu last_frame_ms=%lu", st.frames, st.errors, (unsigned long)st.last_frame_ms);
    Cli_Printf("ws2812_frames=%lu ws2812_irqs=%lu", frames, irqs);
    uint32_t om, rm;
    Fx_GifTiming(&om, &rm);
    Cli_Printf("gif_open_ms=%lu gif_read_ms=%lu", (unsigned long)om, (unsigned long)rm);
    Cli_Printf("mic_blocks=%lu level_db=%d gate_db=%d", Mic_Blocks(), Fx_EqLevel(), Fx_EqGate());
    return 0;
  }
  const char *sub = argv[1];
  if (strcmp(sub, "off") == 0) return Fx_Request(FX_OFF, 0);
  if (strcmp(sub, "color") == 0) return Fx_Request(FX_COLOR, 0);
  if (strcmp(sub, "test") == 0)
  {
    long n = 0;
    if (argc > 2 && Cli_ParseInt(argv[2], 0, 2, &n) != 0) return Cli_Error("test 0 (chain walk) | 1 (orientation) | 2 (rainbow)");
    return Fx_Request(FX_TEST, n);
  }
  if (strcmp(sub, "gif") == 0)
  {
    if (argc < 3)
    {
      for (unsigned i = 0; i < GifList_Count(); i++) Cli_Printf("%u %s %u", i, GifList_Name(i), GifList_Frames(i));
      return 0;
    }
    int i = P_FindIndex(argv[2], GifList_Count(), GifList_Name);
    return i < 0 ? Cli_Error("no such GIF") : Fx_Request(FX_GIF, i);
  }
  if (strcmp(sub, "theme") == 0)
  {
    if (argc < 3)
    {
      for (int i = 0; i < FX_THEME_COUNT; i++) Cli_Printf("%d %s", i, Fx_ThemeName(i));
      return 0;
    }
    int i = P_FindIndex(argv[2], FX_THEME_COUNT, P_ThemeName);
    return i < 0 ? Cli_Error("no such theme") : Fx_Request(FX_THEME, i);
  }
  if (strcmp(sub, "eq") == 0)
  {
    int vu = argc > 2 && strcmp(argv[2], "vu") == 0;
    return Fx_Request(FX_EQ, vu ? FX_EQ_VU : FX_EQ_BARS);
  }
  if (strcmp(sub, "gate") == 0)
  {
    long db;
    if (argc < 3 || Cli_ParseInt(argv[2], 0, 120, &db) != 0) return Cli_Error("gate <dB 0..120>");
    Fx_EqSetGate((int)db);
    return 0;
  }
  return Cli_Error("unknown fx command");
}

/* wdg | wdg stall <task> | wdg hang: watchdog status and tests */
static int C_Wdg(int argc, char **argv)
{
  if (argc < 2)
  {
    Cli_Printf("last_reset=%s", Wdg_ResetCause());
    for (int i = 0; i < Wdg_Count(); i++) Cli_Printf("task=%s age_ms=%lu", Wdg_Name(i), (unsigned long)Wdg_Age(i));
    return 0;
  }
  if (strcmp(argv[1], "stall") == 0 && argc > 2)
  {
    if (Wdg_Stall(argv[2]) != 0) return Cli_Error("no such task");
    Cli_Printf("%s no longer reports: reset in about %u s", argv[2], (WDG_WINDOW_MS + 4000u) / 1000u);
    return 0;
  }
  if (strcmp(argv[1], "hang") == 0)
  {
    Cli_Printf("the App task stops here: reset in about 4 s");
    for (;;) { }
  }
  return Cli_Error("usage: wdg [stall <task> | hang]");
}

static const CliCommand commands[] = {
  { "ver",      "",                   "firmware version", C_Ver, NULL },
  { "i2cscan",  "",                   "list I2C2 devices", C_I2cScan, NULL },
  { "ee",       "read|write <addr> ...", "EEPROM bytes", C_Ee,
    "ee read 0 32          dump 32 bytes from address 0\n"
    "ee write 0x200 1 2 3  write bytes" },
  { "sd",       "[info|ls|mount|mv|rm]", "SD card", C_Sd,
    "sd info               size / free\n"
    "sd ls                 root folder\n"
    "sd ls \"\" BAT-*        files matching a pattern\n"
    "sd mount              mount again (after a card change)\n"
    "sd mv <from> <to>     rename a file in the root folder\n"
    "sd rm <file>          delete a file in the root folder" },
  { "key",      "up|down|back|enter", "press a front button", C_Key, NULL },
  { "led",      "<color> on|off",     "status LEDs", C_Led, NULL },
  { "fx",       "[off|color|gif|theme|eq|test|gate] ...", "LED matrix effects", C_Fx,
    "fx                 status\n"
    "fx gif [name|n]    list the GIFs / play one\n"
    "fx theme [name|n]  list the themes / run one\n"
    "fx eq [bars|vu]    equalizer from the microphone\n"
    "fx gate <dB>       equalizer noise gate\n"
    "fx color           the Colors setting, full level\n"
    "fx test [0|1|2]    chain walk / orientation / rainbow\n"
    "fx off" },
  { "oled",     "[pattern]",          "OLED test: border|fill|checker|clear|contrast <n>", C_Oled, NULL },
  { "gx",       "<GALAXY...>",        "send a raw line to the audio board", C_Gx,
    "gx GALAXYHELLO\n"
    "gx GALAXYGETPLAYLIST\n"
    "gx GALAXYPLAY 3\n"
    "'link log on' shows every line on the link." },
  { "link",     "[log on|off]",       "link to the audio board: state and statistics", C_Link, NULL },
  { "playlist", "",                   "tracks reported by the audio board", C_Playlist, NULL },
  { "tasks",    "",                   "heap and task stacks", C_Tasks, NULL },
  { "log",      "[level]",            "show / set the log level", C_Log, NULL },
  { "reset",    "",                   "restart the board", C_Reset, NULL },
  { "wdg",      "[stall <task> | hang]", "watchdog: tasks, reset cause, tests", C_Wdg, NULL },
};

void Commands_Register(void)
{
  Cli_Register(commands, sizeof commands / sizeof commands[0]);
}
