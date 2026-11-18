#ifndef AUTOPLAYWAVVIEW_HPP
#define AUTOPLAYWAVVIEW_HPP

#include <gui_generated/autoplaywav_screen/AutoplaywavViewBase.hpp>
#include <gui/autoplaywav_screen/AutoplaywavPresenter.hpp>
#include <gui/common/Ui.hpp>

/* Autoplay WAVs: plays the playlist, moving on at every WAVEEND from the audio
 * board. ENTER = next, UP = random order, DOWN = sequential, BACK = start screen. */
class AutoplaywavView : public AutoplaywavViewBase
{
public:
    AutoplaywavView();
    virtual ~AutoplaywavView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    void play(unsigned index);
    void showMode();
    ui::Marquee title;
    uint32_t waveEnds;
};

#endif // AUTOPLAYWAVVIEW_HPP
