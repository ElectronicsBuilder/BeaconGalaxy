#include <gui/colors_screen/ColorsView.hpp>
#include <gui/common/Ui.hpp>

ColorsView::ColorsView()
{
}

void ColorsView::setupScreen()
{
    ColorsViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_COLORS, ui::COLORS_COUNT);
}

void ColorsView::tearDownScreen()
{
    ColorsViewBase::tearDownScreen();
}

void ColorsView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_COLORS, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER)
    {
        static const ui::Edit edits[ui::COLORS_COUNT] = { ui::ED_COLOR_RED, ui::ED_COLOR_GREEN, ui::ED_COLOR_BLUE };
        const uint16_t i = ui::cursor[ui::MENU_COLORS];
        ui::setEdit(edits[i], ui::COLOR_ITEMS[i], "LVL");
        application().gotoUsersetScreenNoTransition();
    }
}
