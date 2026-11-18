#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/common/Ui.hpp>

Screen1View::Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_MAIN, ui::MAIN_COUNT);
    if (!Ui_PlaylistComplete() && Ui_LinkReady())
    {
        Ui_Post(UI_AUDIO_GET_PLAYLIST, 0);    /* the Audio menu needs it */
    }
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_MAIN, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoscreenScreenNoTransition();
        return;
    }
    if (key != ui::KEY_ENTER)
    {
        return;
    }
    switch (ui::cursor[ui::MENU_MAIN])
    {
    case ui::MAIN_THEMES:
        application().gotothemesScreenNoTransition();
        break;
    case ui::MAIN_GIFS:
        if (Ui_GifCount() > 0)
        {
            application().gotoGIFsScreenNoTransition();
        }
        break;
    case ui::MAIN_EQUALIZER:
        application().gotoEqualizerScreenNoTransition();
        break;
    case ui::MAIN_COLORS:
        application().gotoColorsScreenNoTransition();
        break;
    case ui::MAIN_AUDIO:
        if (Ui_TrackCount() > 0)
        {
            application().gotoAudioScreenNoTransition();
        }
        break;
    case ui::MAIN_SETTINGS:
        application().gotoSettingScreenNoTransition();
        break;
    case ui::MAIN_ABOUT:
        ui::nav = ui::NAV_ABOUT;
        application().gotoUsernavScreenNoTransition();
        break;
    }
}
