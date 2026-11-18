#ifndef AUDIOVIEW_HPP
#define AUDIOVIEW_HPP

#include <gui_generated/audio_screen/AudioViewBase.hpp>
#include <gui/audio_screen/AudioPresenter.hpp>

class AudioView : public AudioViewBase
{
public:
    AudioView();
    virtual ~AudioView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::AUDIO_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::AUDIO_ITEMS[itemIndex]);
    }
};

#endif
