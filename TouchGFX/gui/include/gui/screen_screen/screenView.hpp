#ifndef SCREENVIEW_HPP
#define SCREENVIEW_HPP

#include <gui_generated/screen_screen/screenViewBase.hpp>
#include <gui/screen_screen/screenPresenter.hpp>

/* Start screen: uptime clock, days counter, "ready" dot once the audio board
 * has sent its playlist, status line (IDLE, or NO SD while no card answers). ENTER = menu; UP / BACK / DOWN = autoplay WAVs / GIFs / themes. */
class screenView : public screenViewBase
{
public:
    screenView();
    virtual ~screenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    void showTime();
    void showStatus();
    uint32_t lastSecond;
    int sdMissing;
};

#endif // SCREENVIEW_HPP
