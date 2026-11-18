#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::MAIN_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::MAIN_ITEMS[itemIndex]);
    }
};

#endif
