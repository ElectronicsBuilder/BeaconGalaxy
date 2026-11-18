#ifndef PLAYLISTVIEW_HPP
#define PLAYLISTVIEW_HPP

#include <gui_generated/playlist_screen/PlaylistViewBase.hpp>
#include <gui/playlist_screen/PlaylistPresenter.hpp>

class PlaylistView : public PlaylistViewBase
{
public:
    PlaylistView();
    virtual ~PlaylistView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

    virtual void scrollWheel1UpdateItem(TextScrollContainer& item, int16_t itemIndex)
    {
        item.setText(Ui_TrackName(static_cast<unsigned>(itemIndex)));
    }

    virtual void scrollWheel1UpdateCenterItem(TextCenterScrollContainer& item, int16_t itemIndex)
    {
        item.setText(Ui_TrackName(static_cast<unsigned>(itemIndex)));
    }
};

#endif
