/*
 * TextCenterContainer.hpp - one line of text in a scroll wheel. The owning view supplies
 * the label (see gui/common/Ui.hpp).
 */
#ifndef TEXTCENTERCONTAINER_HPP
#define TEXTCENTERCONTAINER_HPP

#include <gui_generated/containers/TextCenterContainerBase.hpp>
#include <gui/common/Ui.hpp>

class TextCenterContainer : public TextCenterContainerBase
{
public:
    TextCenterContainer();
    virtual ~TextCenterContainer() {}
    virtual void initialize();

    void setText(const char* text)
    {
        ui::setText(textArea1Buffer, TEXTAREA1_SIZE, text);
        textArea1.invalidate();
    }
};

#endif
