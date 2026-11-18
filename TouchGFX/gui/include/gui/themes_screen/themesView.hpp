#ifndef THEMESVIEW_HPP
#define THEMESVIEW_HPP

#include <gui_generated/themes_screen/themesViewBase.hpp>
#include <gui/themes_screen/themesPresenter.hpp>

class themesView : public themesViewBase
{
public:
    themesView();
    virtual ~themesView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::THEME_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::THEME_ITEMS[itemIndex]);
    }
};

#endif
