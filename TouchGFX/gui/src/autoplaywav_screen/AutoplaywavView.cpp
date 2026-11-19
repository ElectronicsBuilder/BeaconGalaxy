#include <gui/autoplaywav_screen/AutoplaywavView.hpp>

namespace
{
unsigned position;       /* kept between visits */
bool started;
}

AutoplaywavView::AutoplaywavView() : waveEnds(0), paused(false)
{
}

void AutoplaywavView::setupScreen()
{
    AutoplaywavViewBase::setupScreen();
    const Settings* s = Settings_Get();
    showMode();
    Ui_Post(UI_AUDIO_REPEAT, 0);            /* WAVEEND only comes with repeat off */
    waveEnds = Ui_WaveEnds();
    if (!started)
    {
        position = s->auto_wav_random ? Ui_Random(Ui_TrackCount()) : 0u;
        started = true;
    }
    play(position);
}

void AutoplaywavView::tearDownScreen()
{
    AutoplaywavViewBase::tearDownScreen();
}

void AutoplaywavView::showMode()
{
    ui::setText(textArea4Buffer, TEXTAREA4_SIZE, Settings_Get()->auto_wav_random ? "RAND" : "SEQ");
    textArea4.invalidate();
    ui::setText(textArea1Buffer, TEXTAREA1_SIZE, paused ? "PAUSED" : "AUTOWAV");
    textArea1.invalidate();
}

void AutoplaywavView::play(unsigned index)
{
    if (index >= Ui_TrackCount())
    {
        index = 0;
    }
    position = index;
    paused = false;
    showMode();
    title.set(Ui_TrackName(index), TEXTAREA2_SIZE - 1);
    Ui_Post(UI_AUDIO_PLAY, static_cast<int32_t>(index));
}

void AutoplaywavView::handleTickEvent()
{
    const uint32_t ends = Ui_WaveEnds();
    if (ends != waveEnds)                    /* track finished: next one */
    {
        waveEnds = ends;
        play(ui::nextIndex(position, Ui_TrackCount(), Settings_Get()->auto_wav_random != 0));
    }
    if (title.tick(Ui_Millis(), textArea2Buffer, TEXTAREA2_SIZE))
    {
        textArea2.invalidate();
    }
}

void AutoplaywavView::handleKeyEvent(uint8_t key)
{
    Settings* s = Settings_Get();
    switch (key)
    {
    case ui::KEY_ENTER:
        paused = !paused;
        Ui_Post(paused ? UI_AUDIO_PAUSE : UI_AUDIO_RESUME, 0);
        showMode();
        break;
    case ui::KEY_UP:
        play(ui::nextIndex(position, Ui_TrackCount(), s->auto_wav_random != 0));
        break;
    case ui::KEY_DOWN:
        s->auto_wav_random = s->auto_wav_random ? 0 : 1;
        showMode();
        break;
    case ui::KEY_BACK:
        Ui_Post(UI_AUDIO_STOP, 0);
        Ui_Post(UI_AUDIO_REPEAT, s->repeat);  /* back to the user's repeat setting */
        Ui_Post(UI_SETTINGS_SAVE, 0);
        application().gotoscreenScreenNoTransition();
        break;
    }
}
