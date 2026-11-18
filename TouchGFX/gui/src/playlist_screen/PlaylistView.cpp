#include <gui/playlist_screen/PlaylistView.hpp>
#include <gui/common/Ui.hpp>

PlaylistView::PlaylistView()
{
}

void PlaylistView::setupScreen()
{
    PlaylistViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_PLAYLIST, static_cast<int16_t>(Ui_TrackCount()));
}

void PlaylistView::tearDownScreen()
{
    PlaylistViewBase::tearDownScreen();
}

void PlaylistView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_PLAYLIST, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoAudioScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER && Ui_TrackCount() > 0)
    {
        const uint16_t i = ui::cursor[ui::MENU_PLAYLIST];
        ui::copy(ui::page.title, sizeof ui::page.title, Ui_TrackName(i));
        Ui_Post(UI_AUDIO_PLAY, i);
        application().gotoPlayerScreenNoTransition();
    }
}
