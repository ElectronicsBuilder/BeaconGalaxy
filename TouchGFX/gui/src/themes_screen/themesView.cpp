#include <gui/themes_screen/themesView.hpp>
#include <gui/common/Ui.hpp>

themesView::themesView()
{
}

void themesView::setupScreen()
{
    themesViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_THEMES, UI_THEME_COUNT);
}

void themesView::tearDownScreen()
{
    themesViewBase::tearDownScreen();
}

void themesView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_THEMES, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER)
    {
        const uint16_t i = ui::cursor[ui::MENU_THEMES];
        ui::page.owner = ui::PAGE_THEME;
        ui::copy(ui::page.title, sizeof ui::page.title, Ui_ThemeName(i));
        Ui_Post(UI_THEME_PLAY, i);
        application().gotopageScreenNoTransition();
    }
}
