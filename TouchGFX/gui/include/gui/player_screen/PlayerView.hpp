#ifndef PLAYERVIEW_HPP
#define PLAYERVIEW_HPP

#include <gui_generated/player_screen/PlayerViewBase.hpp>
#include <gui/player_screen/PlayerPresenter.hpp>
#include <gui/common/Ui.hpp>

/* Now playing: the track name (ui::page.title), scrolled when it is long. */
class PlayerView : public PlayerViewBase
{
public:
    PlayerView();
    virtual ~PlayerView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    ui::Marquee title;
};

#endif // PLAYERVIEW_HPP
