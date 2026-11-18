#ifndef COLORSVIEW_HPP
#define COLORSVIEW_HPP

#include <gui_generated/colors_screen/ColorsViewBase.hpp>
#include <gui/colors_screen/ColorsPresenter.hpp>

class ColorsView : public ColorsViewBase
{
public:
    ColorsView();
    virtual ~ColorsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::COLOR_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::COLOR_ITEMS[itemIndex]);
    }
};

#endif
