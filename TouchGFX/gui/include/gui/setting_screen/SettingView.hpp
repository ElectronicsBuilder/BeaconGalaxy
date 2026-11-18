#ifndef SETTINGVIEW_HPP
#define SETTINGVIEW_HPP

#include <gui_generated/setting_screen/SettingViewBase.hpp>
#include <gui/setting_screen/SettingPresenter.hpp>

class SettingView : public SettingViewBase
{
public:
    SettingView();
    virtual ~SettingView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::SETTINGS_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::SETTINGS_ITEMS[itemIndex]);
    }
};

#endif
