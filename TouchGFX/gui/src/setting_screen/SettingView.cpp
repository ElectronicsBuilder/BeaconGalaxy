#include <gui/setting_screen/SettingView.hpp>
#include <gui/common/Ui.hpp>

SettingView::SettingView()
{
}

void SettingView::setupScreen()
{
    SettingViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_SETTINGS, ui::SET_COUNT);
}

void SettingView::tearDownScreen()
{
    SettingViewBase::tearDownScreen();
}

void SettingView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_SETTINGS, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
        return;
    }
    if (key != ui::KEY_ENTER)
    {
        return;
    }
    switch (ui::cursor[ui::MENU_SETTINGS])
    {
    case ui::SET_INCREMENT:
        ui::setEdit(ui::ED_UI_STEP, "UI MULTP", "INC");
        break;
    case ui::SET_INTENSITY:
        ui::setEdit(ui::ED_INTENSITY, "INTENSITY", "INC");
        break;
    case ui::SET_GIF_FPS:
        ui::setEdit(ui::ED_GIF_FPS, "REFRESH", "FPS");
        break;
    case ui::SET_AUTOPLAY:
        ui::nav = ui::NAV_AUTOPLAY;
        application().gotoUsernavScreenNoTransition();
        return;
    case ui::SET_LEDS:
        ui::nav = ui::NAV_LEDS;
        application().gotoUsernavScreenNoTransition();
        return;
    }
    application().gotoUsersetScreenNoTransition();
}
