#include <gui/player_screen/PlayerView.hpp>

PlayerView::PlayerView()
{
}

void PlayerView::setupScreen()
{
    PlayerViewBase::setupScreen();
    title.set(ui::page.title, TEXTAREA1_SIZE - 1);
    handleTickEvent();
}

void PlayerView::tearDownScreen()
{
    PlayerViewBase::tearDownScreen();
}

void PlayerView::handleTickEvent()
{
    if (title.tick(Ui_Millis(), textArea1Buffer, TEXTAREA1_SIZE))
    {
        textArea1.invalidate();
    }
}

void PlayerView::handleKeyEvent(uint8_t key)
{
    if (key == ui::KEY_BACK)
    {
        application().gotoPlaylistScreenNoTransition();
    }
}
