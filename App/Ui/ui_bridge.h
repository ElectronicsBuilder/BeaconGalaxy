/*
 * ui_bridge.h - the only interface between the TouchGFX GUI and the rest of
 * BeaconGalaxy.
 *
 *  GUI -> App:  Ui_Post() queues a typed command; the App task executes it
 *               (link to AudioGalaxy, EEPROM, SD card, matrix).
 *  App -> GUI:  the Ui_* getters return status the GUI polls in Model::tick().
 *
 * Callable from C and C++.
 */
#ifndef APP_UI_BRIDGE_H
#define APP_UI_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UI_GIF_PLAY = 1,        /* a = GIF index (GifList) */
  UI_GIF_STOP,
  UI_THEME_PLAY,          /* a = theme index (UI_THEME_*) */
  UI_THEME_STOP,
  UI_EQ_START,            /* a = UI_EQ_BARS / UI_EQ_VU */
  UI_EQ_STOP,
  UI_COLOR_PREVIEW,       /* the colour settings changed: show them on the matrix */
  UI_SETTINGS_SAVE,       /* store Settings_Get() in the EEPROM */
  UI_AUDIO_GET_PLAYLIST,
  UI_AUDIO_PLAY,          /* a = track index */
  UI_AUDIO_STOP,
  UI_AUDIO_START,         /* play, or resume when paused */
  UI_AUDIO_PAUSE,
  UI_AUDIO_RESUME,
  UI_AUDIO_VOLUME,        /* a = 0..100, live (not stored) */
  UI_AUDIO_RECALL,        /* send the stored volume and repeat mode to the audio board */
  UI_AUDIO_REPEAT,        /* a = 0 off / 1 on */
  UI_RECORD_START,
  UI_RECORD_STOP,
  UI_RECORD_PLAY,         /* play rec.wav */
  UI_UPGRADE_CHECK,       /* look for the update file on the SD card */
  UI_UPGRADE_SET_FUSE,    /* tell GalaxyBoot to update at the next reset */
  UI_REBOOT,
} UiCmdId;

enum { UI_THEME_FIRE, UI_THEME_METEOR, UI_THEME_BOUNCING, UI_THEME_4, UI_THEME_5, UI_THEME_COUNT };
enum { UI_EQ_BARS, UI_EQ_VU };

typedef enum {
  UI_UPG_IDLE,
  UI_UPG_CHECKING,
  UI_UPG_FILE_FOUND,
  UI_UPG_NO_FILE,
  UI_UPG_FUSE_SET,
  UI_UPG_FUSE_FAILED,
} UiUpgrade;

#define UI_UPGRADE_FILE "BeaconGalaxy.bin"

typedef struct {
  UiCmdId id;
  int32_t a;
} UiCmd;

/* GUI side */
int  Ui_Post(UiCmdId id, int32_t a);          /* 0 = queued (never blocks) */
int  Ui_LinkReady(void);                      /* audio board answered */
int  Ui_PlaylistComplete(void);               /* every track name received */
unsigned Ui_TrackCount(void);
const char *Ui_TrackName(unsigned i);         /* "" if unknown */
unsigned Ui_GifCount(void);
const char *Ui_GifName(unsigned i);
UiUpgrade Ui_UpgradeState(void);
uint32_t Ui_WaveEnds(void);                   /* increments on every WAVEEND from the audio board */
uint32_t Ui_Random(uint32_t n);               /* 0..n-1 (0 if n == 0) */
uint32_t Ui_Millis(void);

/* App side */
void Ui_Init(void);
void Ui_Poll(void);                           /* App task: runs queued commands */

#ifdef __cplusplus
}
#endif

#endif
