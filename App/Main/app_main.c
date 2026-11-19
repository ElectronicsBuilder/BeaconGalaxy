/*
 * app_main.c - BeaconGalaxy application: start-up and the App task loop.
 *
 * Called from the CubeMX default task (Core/Src/freertos.c). The App task runs
 * every 2 ms: console input (USART1 and SWD), link input, buttons, commands
 * from the UI, status LEDs. The UI (TouchGFX) runs in its own task and talks to
 * the rest only through App/Ui/ui_bridge.h; the matrix renderer gets its own task.
 */
#include "app_main.h"
#include "app_hal.h"
#include "version.h"
#include "cli.h"
#include "log.h"
#include "uart_io.h"
#include "swd_console.h"
#include "commands.h"
#include "buttons.h"
#include "leds.h"
#include "oled.h"
#include "eeprom.h"
#include "sdcard.h"
#include "link.h"
#include "settings.h"
#include "gif_list.h"
#include "ui_bridge.h"
#include "fx.h"
#include "status_leds.h"
#include "app_touchgfx.h"
#include "cmsis_os2.h"

#define CONSOLE_PORT 0     /* USART1 -> FT232 (USB serial) */
#define SWD_PORT     1     /* RAM mailbox over the ST-LINK (tools/swd_console.py) */
#define TICK_MS      2u

static UartIo *console;
static osMutexId_t console_lock;

static void P_ConsoleWrite(const char *s, size_t len)
{
  if (console_lock) osMutexAcquire(console_lock, osWaitForever);
  UartIo_Write(console, s, len);
  if (console_lock) osMutexRelease(console_lock);
}

static void P_SwdWrite(const char *s, size_t len)
{
  if (console_lock) osMutexAcquire(console_lock, osWaitForever);
  SwdConsole_Write(s, len);
  if (console_lock) osMutexRelease(console_lock);
}

void APP_USART1_IRQ(void)
{
  UartIo_Irq(console);
}

/* Handshake with AudioGalaxy. When it reports READY (at its boot, or as the
 * answer to HELLO) fetch the playlist and send the stored volume and repeat
 * mode. P_LinkSupervise() repeats whatever did not get through: HELLO until
 * the board answers, the playlist while lines are missing, the settings until
 * they are acknowledged. */
#define HELLO_EVERY_MS     2000u
#define PLAYLIST_STALL_MS  1500u
#define RECALL_WAIT_MS     1000u
#define RETRIES            5u

static uint32_t hello_at, playlist_at, recall_at;
static unsigned playlist_tries, recall_tries;

static void P_LinkEvent(const GxMsg *m)
{
  if (m->id == GX_READY)
  {
    uint32_t now = osKernelGetTickCount();
    LOG_I("audio board ready: %ld tracks", m->a);
    playlist_tries = recall_tries = 0;
    if (m->a > 0)
    {
      Link_Send(GX_GETPLAYLIST, 0);
      playlist_at = now;
    }
    Ui_Post(UI_AUDIO_RECALL, 0);
    recall_at = now;
  }
}

static void P_LinkSupervise(uint32_t now)
{
  const LinkState *s = Link_State();
  if (!s->ready)
  {
    if ((int32_t)(now - hello_at) >= 0)
    {
      Link_Send(GX_HELLO, 0);
      hello_at = now + HELLO_EVERY_MS;
    }
    return;
  }
  if (s->tracks > 0 && s->playlist_count < s->tracks && playlist_tries < RETRIES)
  {
    /* stalled = nothing new since the request or the last line, whichever is later */
    uint32_t last = (int32_t)(s->last_playlist_tick - playlist_at) > 0 ? s->last_playlist_tick : playlist_at;
    if ((int32_t)(now - last) >= (int32_t)PLAYLIST_STALL_MS)
    {
      playlist_tries++;
      LOG_W("playlist incomplete (%d/%d): asking again", s->playlist_count, s->tracks);
      Link_Send(GX_GETPLAYLIST, 0);
      playlist_at = now;
    }
  }
  if ((!s->recallvol_ack || !s->repeat_ack) && recall_tries < RETRIES &&
      (int32_t)(now - recall_at) >= (int32_t)RECALL_WAIT_MS)
  {
    recall_tries++;
    LOG_W("audio board did not confirm volume/repeat: sending again");
    Ui_Post(UI_AUDIO_RECALL, 0);
    recall_at = now;
  }
}

static void P_StartGui(void)
{
  static const osThreadAttr_t attr = {
    .name = "gui",
    .stack_size = 4096,
    .priority = osPriorityNormal,
  };
  if (osThreadNew(TouchGFX_Task, NULL, &attr) == NULL) LOG_E("GUI task not started");
}

void App_Init(void)
{
  console_lock = osMutexNew(NULL);
  console = UartIo_Start(USART1);
  Cli_SetWriter(CONSOLE_PORT, P_ConsoleWrite);
  SwdConsole_Init();
  Cli_SetWriter(SWD_PORT, P_SwdWrite);
  Commands_Register();
  Cli_Init(APP_NAME " " APP_VERSION);
  LOG_I("%s %s on %s, %lu MHz, vector table 0x%08lX", APP_NAME, APP_VERSION, APP_BOARD,
        (unsigned long)(HAL_RCC_GetSysClockFreq() / 1000000u), (unsigned long)SCB->VTOR);

  Buttons_Init();
  Oled_Init();
  if (Ee_Init() != 0) LOG_W("EEPROM not answering at 0x%02X", EE_I2C_ADDR);
  if (Sd_Mount() == 0)
  {
    uint32_t total, free_kb;
    if (Sd_Info(&total, &free_kb) == 0) LOG_I("SD card: %lu MB, %lu MB free", (unsigned long)(total / 1024u),
                                             (unsigned long)(free_kb / 1024u));
  }
  Settings_Load();
  (void)GifList_Scan();
  Ui_Init();
  Fx_Init();
  StatusLeds_Init();
  Link_Init(P_LinkEvent);
  P_StartGui();
}

void App_Run(void)
{
  uint32_t next = osKernelGetTickCount();
  for (;;)
  {
    next += TICK_MS;
    uint32_t now = osKernelGetTickCount();
    if ((int32_t)(next - now) > 0) osDelayUntil(next);
    else next = now;                          /* fell behind: resync */

    uint8_t b;
    while (UartIo_Read(console, &b)) Cli_InputFrom(CONSOLE_PORT, (char)b);
    while (SwdConsole_Read(&b)) Cli_InputFrom(SWD_PORT, (char)b);
    Link_Poll();
    P_LinkSupervise(now);
    Buttons_Poll(TICK_MS);        /* presses are read by the GUI (GalaxyButtonController) */
    Ui_Poll();
    StatusLeds_Poll(now);
  }
}
