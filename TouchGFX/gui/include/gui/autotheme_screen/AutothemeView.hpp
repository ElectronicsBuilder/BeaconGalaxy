#ifndef AUTOTHEMEVIEW_HPP
#define AUTOTHEMEVIEW_HPP

#include <gui_generated/autotheme_screen/AutothemeViewBase.hpp>
#include <gui/autotheme_screen/AutothemePresenter.hpp>
#include <gui/common/Ui.hpp>

/* Autoplay themes: each one runs for a random 1..MAX TIME minutes (Settings >
 * AUTOPLAY), the clock shows the time left. ENTER = next, UP = random order,
 * DOWN = sequential, BACK = stop and return to the start screen. */
class AutothemeView : public AutothemeViewBase
{
public:
    AutothemeView();
    virtual ~AutothemeView() {}
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
