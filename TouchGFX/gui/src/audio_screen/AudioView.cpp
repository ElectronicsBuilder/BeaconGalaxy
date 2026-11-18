#include <gui/audio_screen/AudioView.hpp>
#include <gui/common/Ui.hpp>

AudioView::AudioView()
{
}

void AudioView::setupScreen()
{
    AudioViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_AUDIO, ui::AUDIO_COUNT);
    if (!Ui_PlaylistComplete())
    {
        Ui_Post(UI_AUDIO_GET_PLAYLIST, 0);
    }
}

void AudioView::tearDownScreen()
{
    AudioViewBase::tearDownScreen();
}

void AudioView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_AUDIO, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
        return;
    }
    if (key != ui::KEY_ENTER)
    {
        return;
    }
    switch (ui::cursor[ui::MENU_AUDIO])
    {
    case ui::AUDIO_PLAYLIST:
        application().gotoPlaylistScreenNoTransition();
        return;
    case ui::AUDIO_RECORD:
        ui::nav = ui::NAV_RECORD;
        application().gotoUsernavScreenNoTransition();
        return;
    case ui::AUDIO_VOLUME:
        ui::setEdit(ui::ED_VOLUME, "VOLUME", "VOL");
        break;
    case ui::AUDIO_STOP:
        Ui_Post(UI_AUDIO_STOP, 0);
        ui::setEdit(ui::ED_AUDIO_STOP, "AUDIO", "STOP", "OFF");
        break;
    case ui::AUDIO_START:
        Ui_Post(UI_AUDIO_START, 0);    /* the old code sent "STOPSTART", which did nothing */
        ui::setEdit(ui::ED_AUDIO_START, "AUDIO", "START", "ON");
        break;
    case ui::AUDIO_REPEAT:
        ui::setEdit(ui::ED_REPEAT, "REPEAT", "STAT");
        break;
    }
    application().gotoUsersetScreenNoTransition();
}
