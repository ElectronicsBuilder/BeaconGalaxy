/*
 * TextScrollContainer.hpp - one line of text in a scroll wheel. The owning view supplies
 * the label (see gui/common/Ui.hpp).
 */
#ifndef TEXTSCROLLCONTAINER_HPP
#define TEXTSCROLLCONTAINER_HPP

#include <gui_generated/containers/TextScrollContainerBase.hpp>
#include <gui/common/Ui.hpp>

class TextScrollContainer : public TextScrollContainerBase
{
public:
    TextScrollContainer();
    virtual ~TextScrollContainer() {}
    virtual void initialize();

    void setText(const char* text)
    {
        ui::setText(textArea1Buffer, TEXTAREA1_SIZE, text);
        textArea1.invalidate();
    }
};

#endif
