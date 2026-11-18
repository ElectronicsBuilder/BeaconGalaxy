/*
 * settings.c - see settings.h.
 * Record: magic "GXST", version, payload length, payload, CRC16 of the payload.
 */
#include "settings.h"
#include "eeprom.h"
#include "crc16.h"
#include "log.h"
#include <string.h>

#define MAGIC   0x54535847u   /* "GXST" */
#define VERSION 1u

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t len;
} Header;

static Settings live;

void Settings_Defaults(Settings *s)
{
  memset(s, 0, sizeof *s);
  s->red = s->green = s->blue = 255;
  s->intensity = 50;
  s->volume = 70;
  s->ui_step = 5;
  s->gif_fps = 10;
  s->autoplay_max_min = 5;
  s->led_delay = 30;
  s->led_dma = 1;
}

Settings *Settings_Get(void)
{
  return &live;
}

void Settings_Load(void)
{
  Header h;
  Settings s;
  uint16_t crc;
  if (Ee_Read(SETTINGS_EE_ADDR, &h, sizeof h) == 0 && h.magic == MAGIC && h.version == VERSION &&
      h.len == sizeof s &&
      Ee_Read(SETTINGS_EE_ADDR + sizeof h, &s, sizeof s) == 0 &&
      Ee_Read(SETTINGS_EE_ADDR + sizeof h + sizeof s, &crc, sizeof crc) == 0 &&
      crc == Crc16(&s, sizeof s))
  {
    live = s;
    return;
  }
  LOG_W("settings: no valid record, using defaults");
  Settings_Defaults(&live);
}

int Settings_Save(void)
{
  uint8_t buf[sizeof(Header) + sizeof(Settings) + 2];
  Header h = { MAGIC, VERSION, sizeof(Settings) };
  uint16_t crc = Crc16(&live, sizeof live);
  memcpy(buf, &h, sizeof h);
  memcpy(buf + sizeof h, &live, sizeof live);
  memcpy(buf + sizeof h + sizeof live, &crc, sizeof crc);
  int rc = Ee_Write(SETTINGS_EE_ADDR, buf, sizeof buf);
  if (rc != 0) LOG_E("settings: EEPROM write failed");
  return rc;
}
