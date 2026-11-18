#ifndef USERSETVIEW_HPP
#define USERSETVIEW_HPP

#include <gui_generated/userset_screen/UsersetViewBase.hpp>
#include <gui/userset_screen/UsersetPresenter.hpp>

/* Shared value screen: title, item name and one value (or status text).
 * What it edits comes from ui::edit (gui/common/Ui.hpp). UP/DOWN change the
 * value, BACK keeps it and returns to the menu it came from. */
class UsersetView : public UsersetViewBase
{
public:
    UsersetView();
    virtual ~UsersetView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);

protected:
    void show();
    void change(int dir);
    void commit();
    void goBack();
    int value;
};

#endif // USERSETVIEW_HPP
