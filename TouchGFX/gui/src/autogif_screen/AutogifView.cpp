#include <gui/autogif_screen/AutogifView.hpp>

namespace
{
unsigned position;       /* kept between visits */
bool started;
}

AutogifView::AutogifView() : shownSeconds(0xFFFFFFFFu)
{
}

void AutogifView::setupScreen()
{
    AutogifViewBase::setupScreen();
    showMode();
    if (!started)
    {
        position = Settings_Get()->auto_gif_random ? Ui_Random(Ui_GifCount()) : 0u;
        started = true;
    }
    play(position);
}

void AutogifView::tearDownScreen()
{
    AutogifViewBase::tearDownScreen();
}

void AutogifView::showMode()
{
    ui::setText(textArea4Buffer, TEXTAREA4_SIZE, Settings_Get()->auto_gif_random ? "RAND" : "SEQ");
    textArea4.invalidate();
}

void AutogifView::play(unsigned index)
{
    if (index >= Ui_GifCount())
    {
        index = 0;
    }
    position = index;
    ui::setText(textArea2Buffer, TEXTAREA2_SIZE, Ui_GifName(index));
    textArea2.invalidate();
    Ui_Post(UI_GIF_PLAY, static_cast<int32_t>(index));
    const uint32_t now = Ui_Millis();
    timer.restart(now, Settings_Get()->autoplay_max_min);
    showTime(now);
}

void AutogifView::showTime(uint32_t now)
{
    const uint32_t left = timer.secondsLeft(now);
    if (left == shownSeconds)
    {
        return;
    }
    shownSeconds = left;
    digitalClock.setTime24Hour(static_cast<uint8_t>(left / 3600u), static_cast<uint8_t>((left / 60u) % 60u),
                               static_cast<uint8_t>(left % 60u));
}

void AutogifView::handleTickEvent()
{
    const uint32_t now = Ui_Millis();
    if (timer.expired(now))
    {
        play(ui::nextIndex(position, Ui_GifCount(), Settings_Get()->auto_gif_random != 0));
    }
    showTime(now);
}

void AutogifView::handleKeyEvent(uint8_t key)
{
    Settings* s = Settings_Get();
    switch (key)
    {
    case ui::KEY_ENTER:
        play(ui::nextIndex(position, Ui_GifCount(), s->auto_gif_random != 0));
        break;
    case ui::KEY_UP:
        s->auto_gif_random = 1;
        showMode();
        break;
    case ui::KEY_DOWN:
        s->auto_gif_random = 0;
        showMode();
        break;
    case ui::KEY_BACK:
        Ui_Post(UI_GIF_STOP, 0);
        Ui_Post(UI_SETTINGS_SAVE, 0);
        application().gotoscreenScreenNoTransition();
        break;
    }
}
