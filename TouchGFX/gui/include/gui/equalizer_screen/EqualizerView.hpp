#ifndef EQUALIZERVIEW_HPP
#define EQUALIZERVIEW_HPP

#include <gui_generated/equalizer_screen/EqualizerViewBase.hpp>
#include <gui/equalizer_screen/EqualizerPresenter.hpp>

class EqualizerView : public EqualizerViewBase
{
public:
    EqualizerView();
    virtual ~EqualizerView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(ui::EQ_ITEMS[itemIndex]);
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(ui::EQ_ITEMS[itemIndex]);
    }
};

#endif
