#ifndef USERNAVVIEW_HPP
#define USERNAVVIEW_HPP

#include <gui_generated/usernav_screen/UsernavViewBase.hpp>
#include <gui/usernav_screen/UsernavPresenter.hpp>

class UsernavView : public UsernavViewBase
{
public:
    UsernavView();
    virtual ~UsernavView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::navItem(ui::nav, itemIndex));
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::navItem(ui::nav, itemIndex));
    }
};

#endif
