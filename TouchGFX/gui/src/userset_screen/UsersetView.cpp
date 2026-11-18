#include <gui/userset_screen/UsersetView.hpp>
#include <gui/common/Ui.hpp>

using ui::edit;

namespace
{
int wrap(int v, int lo, int hi)
{
    if (v > hi)
    {
        return lo;
    }
    if (v < lo)
    {
        return hi;
    }
    return v;
}

int clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

bool isOnOff(ui::Edit e)
{
    return e == ui::ED_REPEAT || e == ui::ED_LED_DMA;
}

bool isFixedText(ui::Edit e)
{
    return e == ui::ED_INFO || e == ui::ED_AUDIO_STOP || e == ui::ED_AUDIO_START || e == ui::ED_RECORD ||
           e == ui::ED_PLAY_RECORD;
}

uint8_t* field(ui::Edit e)
{
    Settings* s = Settings_Get();
    switch (e)
    {
    case ui::ED_COLOR_RED:    return &s->red;
    case ui::ED_COLOR_GREEN:  return &s->green;
    case ui::ED_COLOR_BLUE:   return &s->blue;
    case ui::ED_VOLUME:       return &s->volume;
    case ui::ED_REPEAT:       return &s->repeat;
    case ui::ED_UI_STEP:      return &s->ui_step;
    case ui::ED_INTENSITY:    return &s->intensity;
    case ui::ED_GIF_FPS:      return &s->gif_fps;
    case ui::ED_LED_DMA:      return &s->led_dma;
    case ui::ED_LED_EFFECT:   return &s->led_effect;
    case ui::ED_LED_DELAY:    return &s->led_delay;
    case ui::ED_AUTOPLAY_MAX: return &s->autoplay_max_min;
    default:                  return 0;
    }
}
} // namespace

UsersetView::UsersetView() : value(0)
{
}

void UsersetView::setupScreen()
{
    UsersetViewBase::setupScreen();
    ui::setText(textArea1Buffer, TEXTAREA1_SIZE, edit.title);
    ui::setText(textArea2Buffer, TEXTAREA2_SIZE, edit.item);
    const uint8_t* f = field(edit.edit);
    value = f ? *f : 0;
    show();
}

void UsersetView::tearDownScreen()
{
    UsersetViewBase::tearDownScreen();
}

void UsersetView::show()
{
    if (isFixedText(edit.edit))
    {
        ui::setText(textArea3Buffer, TEXTAREA3_SIZE, edit.status);
    }
    else if (isOnOff(edit.edit))
    {
        ui::setText(textArea3Buffer, TEXTAREA3_SIZE, value ? "ON" : "OFF");
    }
    else
    {
        Unicode::snprintf(textArea3Buffer, TEXTAREA3_SIZE, "%d", value);
    }
    textArea3.invalidate();
}

/* dir = +1 (UP) or -1 (DOWN). Limits and wrap-around follow the original UI. */
void UsersetView::change(int dir)
{
    const Settings* s = Settings_Get();
    const int step = s->ui_step ? s->ui_step : 1;
    switch (edit.edit)
    {
    case ui::ED_COLOR_RED:
    case ui::ED_COLOR_GREEN:
    case ui::ED_COLOR_BLUE:
        value = clamp(value + dir * step, 0, 255);
        *field(edit.edit) = static_cast<uint8_t>(value);   /* live preview */
        Ui_Post(UI_COLOR_PREVIEW, 0);
        break;
    case ui::ED_INTENSITY:
        value = clamp(value + dir * step, 0, 255);
        break;
    case ui::ED_VOLUME:
        value = wrap(value + dir * step, 0, 100);
        Ui_Post(UI_AUDIO_VOLUME, value);                     /* heard right away, stored on BACK */
        break;
    case ui::ED_REPEAT:
    case ui::ED_LED_DMA:
        value = dir > 0 ? 1 : 0;
        break;
    case ui::ED_LED_EFFECT:
        value = wrap(value + dir, 0, 1);
        break;
    case ui::ED_LED_DELAY:
        value = wrap(value + dir * step, 10, 100);
        break;
    case ui::ED_UI_STEP:
        value = wrap(value + dir, 1, 99);
        break;
    case ui::ED_GIF_FPS:
        value = wrap(value + dir, 1, 100);
        break;
    case ui::ED_AUTOPLAY_MAX:
        value = wrap(value + dir * step, 1, 60);
        break;
    default:
        return;                                              /* read-only screens */
    }
    show();
}

void UsersetView::commit()
{
    uint8_t* f = field(edit.edit);
    if (f)
    {
        *f = static_cast<uint8_t>(value);
        Ui_Post(UI_SETTINGS_SAVE, 0);
    }
    if (edit.edit == ui::ED_REPEAT)
    {
        Ui_Post(UI_AUDIO_REPEAT, value);
    }
    else if (edit.edit == ui::ED_RECORD)
    {
        Ui_Post(UI_RECORD_STOP, 0);
    }
}

void UsersetView::goBack()
{
    switch (edit.edit)
    {
    case ui::ED_COLOR_RED:
    case ui::ED_COLOR_GREEN:
    case ui::ED_COLOR_BLUE:
        application().gotoColorsScreenNoTransition();
        break;
    case ui::ED_VOLUME:
    case ui::ED_REPEAT:
    case ui::ED_AUDIO_STOP:
    case ui::ED_AUDIO_START:
        application().gotoAudioScreenNoTransition();
        break;
    case ui::ED_UI_STEP:
    case ui::ED_INTENSITY:
    case ui::ED_GIF_FPS:
        application().gotoSettingScreenNoTransition();
        break;
    default:                     /* record, LEDs, autoplay, about: back to the sub-menu */
        application().gotoUsernavScreenNoTransition();
        break;
    }
}

void UsersetView::handleKeyEvent(uint8_t key)
{
    switch (key)
    {
    case ui::KEY_UP:
        change(+1);
        break;
    case ui::KEY_DOWN:
        change(-1);
        break;
    case ui::KEY_BACK:
        commit();
        goBack();
        break;
    }
}
