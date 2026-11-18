#include <gui/equalizer_screen/EqualizerView.hpp>
#include <gui/common/Ui.hpp>

EqualizerView::EqualizerView()
{
}

void EqualizerView::setupScreen()
{
    EqualizerViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_EQUALIZER, ui::EQ_COUNT);
}

void EqualizerView::tearDownScreen()
{
    EqualizerViewBase::tearDownScreen();
}

void EqualizerView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_EQUALIZER, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER)
    {
        const uint16_t i = ui::cursor[ui::MENU_EQUALIZER];
        ui::page.owner = ui::PAGE_EQ;
        ui::copy(ui::page.title, sizeof ui::page.title, ui::EQ_ITEMS[i]);
        Ui_Post(UI_EQ_START, i == 0 ? UI_EQ_BARS : UI_EQ_VU);
        application().gotopageScreenNoTransition();
    }
}
