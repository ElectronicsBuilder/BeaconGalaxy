#include <gui/usernav_screen/UsernavView.hpp>
#include <gui/common/Ui.hpp>
#include "gif_names.h"
#include "galaxy_link.h"
#include "version.h"

UsernavView::UsernavView()
{
}

void UsernavView::setupScreen()
{
    UsernavViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::navMenu(ui::nav), ui::navCount(ui::nav));
}

void UsernavView::tearDownScreen()
{
    UsernavViewBase::tearDownScreen();
}

void UsernavView::handleKeyEvent(uint8_t key)
{
    const ui::Menu menu = ui::navMenu(ui::nav);
    if (ui::menuKey(scrollWheel1, menu, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        switch (ui::nav)
        {
        case ui::NAV_RECORD:   application().gotoAudioScreenNoTransition(); break;
        case ui::NAV_ABOUT:    application().gotoScreen1ScreenNoTransition(); break;
        case ui::NAV_LEDS:
        case ui::NAV_AUTOPLAY: application().gotoSettingScreenNoTransition(); break;
        }
        return;
    }
    if (key != ui::KEY_ENTER)
    {
        return;
    }
    const uint16_t i = ui::cursor[menu];
    switch (ui::nav)
    {
    case ui::NAV_RECORD:
        if (i == ui::REC_RECORD)
        {
            Ui_Post(UI_RECORD_START, 0);
            ui::setEdit(ui::ED_RECORD, "REC.WAV", "REC", "ON");
        }
        else
        {
            Ui_Post(UI_RECORD_PLAY, 0);
            ui::setEdit(ui::ED_PLAY_RECORD, "REC.WAV", "PLAY", "ON");
        }
        break;
    case ui::NAV_ABOUT:
        switch (i)
        {
        case ui::ABOUT_UPGRADE:
            application().gotoUpgradeScreenNoTransition();
            return;
        case ui::ABOUT_VERSION:  ui::setEdit(ui::ED_INFO, "GALAXY", "VERS", APP_VERSION); break;
        case ui::ABOUT_MAX_WAVS: ui::setEditInt(ui::ED_INFO, "MAXIMUM", "WAVs", GX_PLAYLIST_MAX); break;
        case ui::ABOUT_WAV_NAME: ui::setEditInt(ui::ED_INFO, "WAV NAME", "MAX", GX_NAME_MAX); break;
        case ui::ABOUT_DEV_WAVS: ui::setEditInt(ui::ED_INFO, "DEVICE", "WAVs", static_cast<int>(Ui_TrackCount())); break;
        case ui::ABOUT_MAX_GIFS: ui::setEditInt(ui::ED_INFO, "MAXIMUM", "GIFs", GIF_MAX); break;
        case ui::ABOUT_GIF_NAME: ui::setEditInt(ui::ED_INFO, "GIF NAME", "MAX", GIF_NAME_MAX); break;
        case ui::ABOUT_DEV_GIFS: ui::setEditInt(ui::ED_INFO, "DEVICE", "GIFs", static_cast<int>(Ui_GifCount())); break;
        }
        break;
    case ui::NAV_LEDS:
        switch (i)
        {
        case ui::LEDS_DMA:    ui::setEdit(ui::ED_LED_DMA, "DMA LEDS", "STAT"); break;
        case ui::LEDS_EFFECT: ui::setEdit(ui::ED_LED_EFFECT, "EFFECTS", "NUMB"); break;
        case ui::LEDS_DELAY:  ui::setEdit(ui::ED_LED_DELAY, "DELAY", "VAL"); break;
        }
        break;
    case ui::NAV_AUTOPLAY:
        ui::setEdit(ui::ED_AUTOPLAY_MAX, "AUTOPLAY", "MAX");
        break;
    }
    application().gotoUsersetScreenNoTransition();
}
