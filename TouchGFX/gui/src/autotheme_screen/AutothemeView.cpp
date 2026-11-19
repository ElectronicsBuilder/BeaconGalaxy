#include <gui/autotheme_screen/AutothemeView.hpp>

namespace
{
unsigned position;       /* kept between visits */
bool started;
}

AutothemeView::AutothemeView() : shownSeconds(0xFFFFFFFFu)
{
}

void AutothemeView::setupScreen()
{
    AutothemeViewBase::setupScreen();
    showMode();
    if (!started)
    {
        position = Settings_Get()->auto_theme_random ? Ui_Random(static_cast<unsigned>(UI_THEME_COUNT)) : 0u;
        started = true;
    }
    play(position);
}

void AutothemeView::tearDownScreen()
{
    AutothemeViewBase::tearDownScreen();
}

void AutothemeView::showMode()
{
    ui::setText(textArea4Buffer, TEXTAREA4_SIZE, Settings_Get()->auto_theme_random ? "RAND" : "SEQ");
    textArea4.invalidate();
}

void AutothemeView::play(unsigned index)
{
    if (index >= static_cast<unsigned>(UI_THEME_COUNT))
    {
        index = 0;
    }
    position = index;
    ui::setText(textArea2Buffer, TEXTAREA2_SIZE, Ui_ThemeName(index));
    textArea2.invalidate();
    Ui_Post(UI_THEME_STOP, 0);
    Ui_Post(UI_THEME_PLAY, static_cast<int32_t>(index));
    const uint32_t now = Ui_Millis();
    timer.restart(now, Settings_Get()->autoplay_max_min);
    showTime(now);
}

void AutothemeView::showTime(uint32_t now)
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

void AutothemeView::handleTickEvent()
{
    const uint32_t now = Ui_Millis();
    if (timer.expired(now))
    {
        play(ui::nextIndex(position, static_cast<unsigned>(UI_THEME_COUNT), Settings_Get()->auto_theme_random != 0));
    }
    showTime(now);
}

void AutothemeView::handleKeyEvent(uint8_t key)
{
    Settings* s = Settings_Get();
    switch (key)
    {
    case ui::KEY_ENTER:
        play(ui::nextIndex(position, static_cast<unsigned>(UI_THEME_COUNT), s->auto_theme_random != 0));
        break;
    case ui::KEY_UP:
        s->auto_theme_random = 1;
        showMode();
        break;
    case ui::KEY_DOWN:
        s->auto_theme_random = 0;
        showMode();
        break;
    case ui::KEY_BACK:
        Ui_Post(UI_THEME_STOP, 0);
        Ui_Post(UI_SETTINGS_SAVE, 0);
        application().gotoscreenScreenNoTransition();
        break;
    }
}
