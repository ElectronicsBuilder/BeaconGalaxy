#include <gui/gifs_screen/GIFsView.hpp>
#include <gui/common/Ui.hpp>

GIFsView::GIFsView()
{
}

void GIFsView::setupScreen()
{
    GIFsViewBase::setupScreen();
    ui::menuSetup(scrollWheel1, ui::MENU_GIFS, static_cast<int16_t>(Ui_GifCount()));
}

void GIFsView::tearDownScreen()
{
    GIFsViewBase::tearDownScreen();
}

void GIFsView::handleKeyEvent(uint8_t key)
{
    if (ui::menuKey(scrollWheel1, ui::MENU_GIFS, key))
    {
        return;
    }
    if (key == ui::KEY_BACK)
    {
        application().gotoScreen1ScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER && Ui_GifCount() > 0)
    {
        const uint16_t i = ui::cursor[ui::MENU_GIFS];
        ui::page.owner = ui::PAGE_GIF;
        ui::copy(ui::page.title, sizeof ui::page.title, Ui_GifName(i));
        Ui_Post(UI_GIF_PLAY, i);
        application().gotopageScreenNoTransition();
    }
}
