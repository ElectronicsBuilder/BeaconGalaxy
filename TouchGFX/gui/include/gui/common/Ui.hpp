/*
 * Ui.hpp - state and helpers shared by the BeaconGalaxy screens.
 *
 * Navigation state lives here (one place, typed) instead of the old global
 * cursor[] / current_page[] / previous_page[] arrays. Screens that are reused
 * for several purposes (page, Usernav, Userset) get their context through
 * ui::page / ui::nav / ui::edit before the transition.
 *
 * Everything outside the GUI goes through ui_bridge.h.
 */
#ifndef GUI_UI_HPP
#define GUI_UI_HPP

#include <touchgfx/Unicode.hpp>
#include <touchgfx/containers/scrollers/ScrollWheelWithSelectionStyle.hpp>
#include "ui_bridge.h"
#include "settings.h"

namespace ui
{

/* TouchGFX key codes (PhysicalButtons in BeaconGalaxy.touchgfx) */
enum Key : uint8_t { KEY_ENTER = 1, KEY_BACK = 2, KEY_UP = 3, KEY_DOWN = 4 };

/* Menus whose position is remembered when leaving and coming back */
enum Menu
{
    MENU_MAIN, MENU_THEMES, MENU_GIFS, MENU_EQUALIZER, MENU_COLORS, MENU_AUDIO,
    MENU_SETTINGS, MENU_PLAYLIST, MENU_RECORD, MENU_ABOUT, MENU_LEDS, MENU_AUTOPLAY,
    MENU_COUNT
};

/* Item positions */
enum { MAIN_THEMES, MAIN_GIFS, MAIN_EQUALIZER, MAIN_COLORS, MAIN_AUDIO, MAIN_SETTINGS, MAIN_ABOUT, MAIN_COUNT };
enum { AUDIO_PLAYLIST, AUDIO_RECORD, AUDIO_VOLUME, AUDIO_STOP, AUDIO_START, AUDIO_REPEAT, AUDIO_COUNT };
enum { SET_INCREMENT, SET_INTENSITY, SET_GIF_FPS, SET_AUTOPLAY, SET_LEDS, SET_COUNT };
enum { REC_RECORD, REC_PLAY, REC_COUNT };
enum { ABOUT_UPGRADE, ABOUT_VERSION, ABOUT_MAX_WAVS, ABOUT_WAV_NAME, ABOUT_DEV_WAVS,
       ABOUT_MAX_GIFS, ABOUT_GIF_NAME, ABOUT_DEV_GIFS, ABOUT_COUNT };
enum { LEDS_DMA, LEDS_EFFECT, LEDS_DELAY, LEDS_COUNT };
enum { AUTO_MAX_TIME, AUTO_COUNT };
enum { EQ_COUNT = 2, COLORS_COUNT = 3 };

extern const char* const MAIN_ITEMS[MAIN_COUNT];
extern const char* const THEME_ITEMS[UI_THEME_COUNT];
extern const char* const EQ_ITEMS[EQ_COUNT];
extern const char* const COLOR_ITEMS[COLORS_COUNT];
extern const char* const AUDIO_ITEMS[AUDIO_COUNT];
extern const char* const SETTINGS_ITEMS[SET_COUNT];

extern uint16_t cursor[MENU_COUNT];

/* Scroll-wheel menu: set it up on the remembered item / move with UP and DOWN */
void menuSetup(touchgfx::ScrollWheelWithSelectionStyle& wheel, Menu m, int16_t items);
bool menuKey(touchgfx::ScrollWheelWithSelectionStyle& wheel, Menu m, uint8_t key);

/* What the shared "page" screen is showing (BACK stops it) */
enum PageOwner { PAGE_THEME, PAGE_GIF, PAGE_EQ };
struct PageContext
{
    PageOwner owner;
    char title[16];
};
extern PageContext page;

/* What the shared Usernav (sub-menu) screen lists */
enum NavMode { NAV_RECORD, NAV_ABOUT, NAV_LEDS, NAV_AUTOPLAY };
extern NavMode nav;
int16_t navCount(NavMode m);
const char* navItem(NavMode m, int16_t i);
Menu navMenu(NavMode m);

/* What the shared Userset (value) screen edits */
enum Edit
{
    ED_INFO,                                   /* read-only (About) */
    ED_COLOR_RED, ED_COLOR_GREEN, ED_COLOR_BLUE,
    ED_VOLUME, ED_REPEAT, ED_AUDIO_STOP, ED_AUDIO_START,
    ED_RECORD, ED_PLAY_RECORD,
    ED_UI_STEP, ED_INTENSITY, ED_GIF_FPS,
    ED_LED_DMA, ED_LED_EFFECT, ED_LED_DELAY,
    ED_AUTOPLAY_MAX
};
struct EditContext
{
    Edit edit;
    char title[16];
    char item[16];
    char status[16];       /* fixed text; empty = show the value */
};
extern EditContext edit;
void setEdit(Edit e, const char* title, const char* item, const char* status = "");
void setEditInt(Edit e, const char* title, const char* item, int value);

/* Text helpers */
void setText(touchgfx::Unicode::UnicodeChar* buf, uint16_t size, const char* s);
void copy(char* dst, uint16_t size, const char* s);

/* Long names on a narrow screen: shows a window that slides left, pausing at
 * both ends. Time based (ms), never blocks the GUI task. */
class Marquee
{
public:
    void set(const char* text, uint16_t window);
    /* Fills buf with the visible part; returns true when it changed. */
    bool tick(uint32_t now, touchgfx::Unicode::UnicodeChar* buf, uint16_t size);
    const char* text() const { return full; }

private:
    char full[72];
    uint16_t len;
    uint16_t window;
    uint16_t offset;
    uint32_t next;
    bool dirty;
};

/* Autoplay helpers: next index in sequential or random order */
unsigned nextIndex(unsigned current, unsigned count, bool random);

/* Autoplay of GIFs / themes: each item runs for a random 1..max minutes. */
class AutoTimer
{
public:
    void restart(uint32_t now, unsigned maxMinutes);
    bool expired(uint32_t now) const { return static_cast<int32_t>(now - deadline) >= 0; }
    uint32_t secondsLeft(uint32_t now) const;

private:
    uint32_t deadline;
};

} // namespace ui

#endif
