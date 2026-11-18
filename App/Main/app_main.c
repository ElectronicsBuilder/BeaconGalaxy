/*
 * app_main.c - BeaconGalaxy application: start-up and the App task loop.
 *
 * Called from the CubeMX default task (Core/Src/freertos.c). The App task runs
 * every 2 ms: console input (USART1 and SWD), link input, buttons, heartbeat.
 * The UI (TouchGFX) and the matrix renderer get their own tasks.
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

static void P_LinkEvent(const GxMsg *m)
{
  if (m->id == GX_READY) LOG_I("audio board ready: %ld tracks", m->a);
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
  Link_Init(P_LinkEvent);
  Link_Send(GX_HELLO, 0);
  Leds_Set(LED_G, 1);
}

void App_Run(void)
{
  uint32_t next = osKernelGetTickCount();
  uint32_t beat = 0;
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
    Buttons_Poll(TICK_MS);

    Key k = Buttons_Get();        /* until the UI exists: show presses */
    if (k != KEY_NONE) LOG_I("key %s", Buttons_Name(k));

    if (++beat >= 250u)           /* 0.5 s heartbeat */
    {
      beat = 0;
      Leds_Toggle(LED_O);
    }
  }
}
