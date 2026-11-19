#include <gui/player_screen/PlayerView.hpp>

PlayerView::PlayerView() : state(PLAYING), waveEnds(0)
{
}

void PlayerView::setupScreen()
{
    PlayerViewBase::setupScreen();
    waveEnds = Ui_WaveEnds();
    show(PLAYING);
    handleTickEvent();
}

void PlayerView::tearDownScreen()
{
    PlayerViewBase::tearDownScreen();
}

void PlayerView::show(State s)
{
    state = s;
    /* the name scrolls while playing; otherwise the state replaces it */
    title.set(s == PAUSED ? "PAUSED" : s == STOPPED ? "STOPPED" : ui::page.title, TEXTAREA1_SIZE - 1);
}

void PlayerView::handleTickEvent()
{
    if (Ui_WaveEnds() != waveEnds)          /* the track finished (repeat off) */
    {
        waveEnds = Ui_WaveEnds();
        if (state == PLAYING)
        {
            show(STOPPED);
        }
    }
    if (title.tick(Ui_Millis(), textArea1Buffer, TEXTAREA1_SIZE))
    {
        textArea1.invalidate();
    }
}

void PlayerView::handleKeyEvent(uint8_t key)
{
    switch (key)
    {
    case ui::KEY_ENTER:
        if (state == PLAYING)
        {
            Ui_Post(UI_AUDIO_PAUSE, 0);
            show(PAUSED);
        }
        else if (state == PAUSED)
        {
            Ui_Post(UI_AUDIO_RESUME, 0);
            show(PLAYING);
        }
        else
        {
            Ui_Post(UI_AUDIO_PLAY, ui::cursor[ui::MENU_PLAYLIST]);
            show(PLAYING);
        }
        break;
    case ui::KEY_DOWN:
        Ui_Post(UI_AUDIO_STOP, 0);
        show(STOPPED);
        break;
    case ui::KEY_UP:
        Ui_Post(UI_AUDIO_PLAY, ui::cursor[ui::MENU_PLAYLIST]);
        show(PLAYING);
        break;
    case ui::KEY_BACK:
        application().gotoPlaylistScreenNoTransition();
        break;
    }
}
