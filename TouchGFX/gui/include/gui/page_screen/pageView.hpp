#ifndef PAGEVIEW_HPP
#define PAGEVIEW_HPP

#include <gui_generated/page_screen/pageViewBase.hpp>
#include <gui/page_screen/pagePresenter.hpp>

/* "Now running" page for a theme, GIF or equalizer (ui::page). BACK stops it. */
class pageView : public pageViewBase
{
public:
    pageView();
    virtual ~pageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);
};

#endif // PAGEVIEW_HPP
