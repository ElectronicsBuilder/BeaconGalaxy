/*
 * Horizontallist.hpp - one line of text in a scroll wheel. The owning view supplies
 * the label (see gui/common/Ui.hpp).
 */
#ifndef HORIZONTALLIST_HPP
#define HORIZONTALLIST_HPP

#include <gui_generated/containers/HorizontallistBase.hpp>
#include <gui/common/Ui.hpp>

class Horizontallist : public HorizontallistBase
{
public:
    Horizontallist();
    virtual ~Horizontallist() {}
    virtual void initialize();

    void setText(const char* text)
    {
        ui::setText(textArea1Buffer, TEXTAREA1_SIZE, text);
        textArea1.invalidate();
    }
};

#endif
