#include <gui/screen_screen/screenView.hpp>
#include <gui/common/Ui.hpp>

screenView::screenView() : lastSecond(0xFFFFFFFFu), sdMissing(-1)
{
}

void screenView::setupScreen()
{
    screenViewBase::setupScreen();
    circle1.setVisible(Ui_PlaylistComplete() != 0);
    showStatus();
    showTime();
}

void screenView::tearDownScreen()
{
    screenViewBase::tearDownScreen();
}

void screenView::showTime()
{
    const uint32_t s = Ui_Millis() / 1000u;     /* time since power-up */
    if (s == lastSecond)
    {
        return;
    }
    lastSecond = s;
    digitalClock.setTime24Hour(static_cast<uint8_t>((s / 3600u) % 24u), static_cast<uint8_t>((s / 60u) % 60u),
                               static_cast<uint8_t>(s % 60u));
    Unicode::snprintf(textArea4Buffer, TEXTAREA4_SIZE, "%u", static_cast<unsigned>((s / 86400u) % 1000u));
    textArea4.invalidate();
    const bool ready = Ui_PlaylistComplete() != 0;
    if (circle1.isVisible() != ready)
    {
        circle1.setVisible(ready);
        circle1.invalidate();
    }
}

void screenView::showStatus()
{
    const int missing = Ui_SdMissing() != 0;
    if (missing == sdMissing)
    {
        return;
    }
    sdMissing = missing;
    ui::setText(textArea2Buffer, TEXTAREA2_SIZE, missing ? "NO SD" : "IDLE");
    textArea2.invalidate();
}

void screenView::handleTickEvent()
{
    showStatus();
    showTime();
}

void screenView::handleKeyEvent(uint8_t key)
{
    switch (key)
    {
    case ui::KEY_ENTER:
        application().gotoScreen1ScreenNoTransition();
        break;
    case ui::KEY_UP:                      /* autoplay WAVs: needs the playlist */
        if (Ui_PlaylistComplete() && Ui_TrackCount() > 0)
        {
            application().gotoAutoplaywavScreenNoTransition();
        }
        break;
    case ui::KEY_BACK:                    /* autoplay GIFs */
        if (Ui_GifCount() > 0)
        {
            application().gotoAutogifScreenNoTransition();
        }
        break;
    case ui::KEY_DOWN:                    /* autoplay themes */
        application().gotoAutothemeScreenNoTransition();
        break;
    }
}
