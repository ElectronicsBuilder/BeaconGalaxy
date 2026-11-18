/*
 * TextCenterScrollContainer.hpp - one line of text in a scroll wheel. The owning view supplies
 * the label (see gui/common/Ui.hpp).
 */
#ifndef TEXTCENTERSCROLLCONTAINER_HPP
#define TEXTCENTERSCROLLCONTAINER_HPP

#include <gui_generated/containers/TextCenterScrollContainerBase.hpp>
#include <gui/common/Ui.hpp>

class TextCenterScrollContainer : public TextCenterScrollContainerBase
{
public:
    TextCenterScrollContainer();
    virtual ~TextCenterScrollContainer() {}
    virtual void initialize();

    void setText(const char* text)
    {
        ui::setText(textArea1Buffer, TEXTAREA1_SIZE, text);
        textArea1.invalidate();
    }
};

#endif
