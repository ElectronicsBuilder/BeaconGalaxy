/*
 * Ui.cpp - see Ui.hpp.
 */
#include <gui/common/Ui.hpp>
#include <string.h>
#include <stdio.h>

namespace ui
{

const char* const MAIN_ITEMS[MAIN_COUNT] = { "THEMES", "PLAY GIFS", "EQUALIZER", "COLORS", "AUDIO", "SETTINGS", "ABOUT" };
const char* const EQ_ITEMS[EQ_COUNT] = { "EQ BARS", "VU METER" };
const char* const COLOR_ITEMS[COLORS_COUNT] = { "CL RED", "CL GREEN", "CL BLUE" };
const char* const AUDIO_ITEMS[AUDIO_COUNT] = { "PLAYLIST", "RECORD", "VOLUME", "STOP", "START", "REPEAT" };
const char* const SETTINGS_ITEMS[SET_COUNT] = { "INCREMENT", "INTENSITY", "GIFs FPS", "AUTOPLAY", "LEDS" };

static const char* const RECORD_ITEMS[REC_COUNT] = { "RECORD", "PLAY" };
static const char* const ABOUT_ITEMS[ABOUT_COUNT] = { "UPGRADE", "GALAXY VER", "MAX WAVs", "WAV NAME",
                                                      "DEVC WAVs", "MAX GIFs", "GIF NAME", "DEVC GIFs" };
static const char* const LEDS_ITEMS[LEDS_COUNT] = { "DMA STAT", "EFFECTS", "LED DELAY" };
static const char* const AUTO_ITEMS[AUTO_COUNT] = { "MAX TIME" };

uint16_t cursor[MENU_COUNT];
PageContext page;
NavMode nav;
EditContext edit;

void menuSetup(touchgfx::ScrollWheelWithSelectionStyle& wheel, Menu m, int16_t items)
{
    if (items < 1)
    {
        items = 1;
    }
    if (cursor[m] >= items)
    {
        cursor[m] = static_cast<uint16_t>(items - 1);
    }
    wheel.setNumberOfItems(items);
    wheel.animateToItem(cursor[m], 0);
}

bool menuKey(touchgfx::ScrollWheelWithSelectionStyle& wheel, Menu m, uint8_t key)
{
    const int16_t items = wheel.getNumberOfItems();
    if (key == KEY_DOWN && cursor[m] + 1 < items)
    {
        cursor[m]++;
    }
    else if (key == KEY_UP && cursor[m] > 0)
    {
        cursor[m]--;
    }
    else
    {
        return key == KEY_UP || key == KEY_DOWN;
    }
    wheel.animateToItem(cursor[m]);
    wheel.invalidate();
    return true;
}

int16_t navCount(NavMode m)
{
    switch (m)
    {
    case NAV_RECORD:   return REC_COUNT;
    case NAV_ABOUT:    return ABOUT_COUNT;
    case NAV_LEDS:     return LEDS_COUNT;
    case NAV_AUTOPLAY: return AUTO_COUNT;
    }
    return 0;
}

const char* navItem(NavMode m, int16_t i)
{
    if (i < 0 || i >= navCount(m))
    {
        return "";
    }
    switch (m)
    {
    case NAV_RECORD:   return RECORD_ITEMS[i];
    case NAV_ABOUT:    return ABOUT_ITEMS[i];
    case NAV_LEDS:     return LEDS_ITEMS[i];
    case NAV_AUTOPLAY: return AUTO_ITEMS[i];
    }
    return "";
}

Menu navMenu(NavMode m)
{
    switch (m)
    {
    case NAV_RECORD:   return MENU_RECORD;
    case NAV_ABOUT:    return MENU_ABOUT;
    case NAV_LEDS:     return MENU_LEDS;
    case NAV_AUTOPLAY: return MENU_AUTOPLAY;
    }
    return MENU_ABOUT;
}

void setEdit(Edit e, const char* title, const char* item, const char* status)
{
    edit.edit = e;
    copy(edit.title, sizeof edit.title, title);
    copy(edit.item, sizeof edit.item, item);
    copy(edit.status, sizeof edit.status, status);
}

void setEditInt(Edit e, const char* title, const char* item, int value)
{
    char v[12];
    snprintf(v, sizeof v, "%d", value);
    setEdit(e, title, item, v);
}

void copy(char* dst, uint16_t size, const char* s)
{
    if (size == 0)
    {
        return;
    }
    strncpy(dst, s ? s : "", size - 1);
    dst[size - 1] = '\0';
}

void setText(touchgfx::Unicode::UnicodeChar* buf, uint16_t size, const char* s)
{
    touchgfx::Unicode::strncpy(buf, s ? s : "", size - 1);
    buf[size - 1] = 0;
}

/* ---- Marquee ---------------------------------------------------------------- */

static const uint32_t MARQUEE_STEP_MS = 170;    /* the old 10 frames at 60 Hz */
static const uint32_t MARQUEE_PAUSE_MS = 500;   /* rest at the start and the end */

void Marquee::set(const char* t, uint16_t w)
{
    copy(full, sizeof full, t);
    len = static_cast<uint16_t>(strlen(full));
    window = w;
    offset = 0;
    next = 0;
    dirty = true;
}

bool Marquee::tick(uint32_t now, touchgfx::Unicode::UnicodeChar* buf, uint16_t size)
{
    bool changed = dirty;
    dirty = false;
    if (len > window && (next == 0 || static_cast<int32_t>(now - next) >= 0))
    {
        if (next != 0)
        {
            offset = (offset + window >= len) ? 0 : static_cast<uint16_t>(offset + 1);
            changed = true;
        }
        const bool atEnd = offset == 0 || offset + window >= len;
        next = now + (atEnd ? MARQUEE_PAUSE_MS : MARQUEE_STEP_MS);
        if (next == 0)
        {
            next = 1;
        }
    }
    if (changed)
    {
        char part[sizeof full];
        const unsigned n = window + 1u < sizeof part ? window + 1u : sizeof part;
        copy(part, static_cast<uint16_t>(n), full + offset);
        setText(buf, size, part);
    }
    return changed;
}

unsigned nextIndex(unsigned current, unsigned count, bool random)
{
    if (count == 0)
    {
        return 0;
    }
    if (random)
    {
        return Ui_Random(count);
    }
    return (current + 1 < count) ? current + 1 : 0;
}

void AutoTimer::restart(uint32_t now, unsigned maxMinutes)
{
    const unsigned minutes = 1u + Ui_Random(maxMinutes ? maxMinutes : 1u);
    deadline = now + minutes * 60000u;
}

uint32_t AutoTimer::secondsLeft(uint32_t now) const
{
    return expired(now) ? 0u : (deadline - now + 999u) / 1000u;
}

} // namespace ui
