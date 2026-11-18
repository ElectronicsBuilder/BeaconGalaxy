/*
 * TextContainer.hpp - one line of text in a scroll wheel. The owning view supplies
 * the label (see gui/common/Ui.hpp).
 */
#ifndef TEXTCONTAINER_HPP
#define TEXTCONTAINER_HPP

#include <gui_generated/containers/TextContainerBase.hpp>
#include <gui/common/Ui.hpp>

class TextContainer : public TextContainerBase
{
public:
    TextContainer();
    virtual ~TextContainer() {}
    virtual void initialize();

    void setText(const char* text)
    {
        ui::setText(textArea1Buffer, TEXTAREA1_SIZE, text);
        textArea1.invalidate();
    }
};

#endif
