#ifndef PLAYERVIEW_HPP
#define PLAYERVIEW_HPP

#include <gui_generated/player_screen/PlayerViewBase.hpp>
#include <gui/player_screen/PlayerPresenter.hpp>
#include <gui/common/Ui.hpp>

/* Now playing: the track name (ui::page.title), scrolled when it is long.
 * ENTER = pause / resume, DOWN = stop, UP = play the track again,
 * BACK = back to the playlist (the track keeps playing). */
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
    enum State { PLAYING, PAUSED, STOPPED };
    void show(State s);
    ui::Marquee title;
    State state;
    uint32_t waveEnds;
};

#endif // PLAYERVIEW_HPP
