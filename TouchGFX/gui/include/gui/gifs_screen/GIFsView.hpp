#ifndef GIFSVIEW_HPP
#define GIFSVIEW_HPP

#include <gui_generated/gifs_screen/GIFsViewBase.hpp>
#include <gui/gifs_screen/GIFsPresenter.hpp>

class GIFsView : public GIFsViewBase
{
public:
    GIFsView();
    virtual ~GIFsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextContainer& item, int16_t itemIndex)
    {
        item.setText(Ui_GifName(static_cast<unsigned>(itemIndex)));
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterContainer& item, int16_t itemIndex)
    {
        item.setText(Ui_GifName(static_cast<unsigned>(itemIndex)));
    }
};

#endif
