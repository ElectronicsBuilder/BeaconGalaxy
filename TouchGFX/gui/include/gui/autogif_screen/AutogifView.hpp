#ifndef AUTOGIFVIEW_HPP
#define AUTOGIFVIEW_HPP

#include <gui_generated/autogif_screen/AutogifViewBase.hpp>
#include <gui/autogif_screen/AutogifPresenter.hpp>
#include <gui/common/Ui.hpp>

/* Autoplay GIFs: each one runs for a random 1..MAX TIME minutes (Settings >
 * AUTOPLAY), the clock shows the time left. ENTER = next, UP = random order,
 * DOWN = sequential, BACK = stop and return to the start screen. */
class AutogifView : public AutogifViewBase
{
public:
    AutogifView();
    virtual ~AutogifView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    void play(unsigned index);
    void showMode();
    void showTime(uint32_t now);
    ui::AutoTimer timer;
    uint32_t shownSeconds;
};

#endif
