#include <gui/page_screen/pageView.hpp>
#include <gui/common/Ui.hpp>

pageView::pageView()
{
}

void pageView::setupScreen()
{
    pageViewBase::setupScreen();
    ui::setText(textArea1Buffer, TEXTAREA1_SIZE, ui::page.title);
    textArea1.invalidate();
}

void pageView::tearDownScreen()
{
    pageViewBase::tearDownScreen();
}

void pageView::handleKeyEvent(uint8_t key)
{
    if (key != ui::KEY_BACK)
    {
        return;
    }
    switch (ui::page.owner)
    {
    case ui::PAGE_THEME:
        Ui_Post(UI_THEME_STOP, 0);
        application().gotothemesScreenNoTransition();
        break;
    case ui::PAGE_GIF:
        Ui_Post(UI_GIF_STOP, 0);
        application().gotoGIFsScreenNoTransition();
        break;
    case ui::PAGE_EQ:
        Ui_Post(UI_EQ_STOP, 0);
        application().gotoEqualizerScreenNoTransition();
        break;
    }
}
