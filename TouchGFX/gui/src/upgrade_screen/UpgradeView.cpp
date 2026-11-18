#include <gui/upgrade_screen/UpgradeView.hpp>
#include <gui/common/Ui.hpp>

namespace
{
const uint32_t STEP_MS = 500;
}

UpgradeView::UpgradeView() : step(IDLE), stepAt(0)
{
}

void UpgradeView::setupScreen()
{
    UpgradeViewBase::setupScreen();
    ui::setText(textArea1Buffer, TEXTAREA1_SIZE, "UPGRADE");
    status("WAITING", 0);
    step = IDLE;
}

void UpgradeView::tearDownScreen()
{
    UpgradeViewBase::tearDownScreen();
}

void UpgradeView::status(const char* text, int progress)
{
    ui::setText(textArea2Buffer, TEXTAREA2_SIZE, text);
    textArea2.invalidate();
    boxProgress1.setValue(progress);
    boxProgress1.invalidate();
}

void UpgradeView::handleTickEvent()
{
    const uint32_t now = Ui_Millis();
    if (step == IDLE || step == DONE || static_cast<int32_t>(now - stepAt) < static_cast<int32_t>(STEP_MS))
    {
        return;
    }
    const UiUpgrade st = Ui_UpgradeState();
    switch (step)
    {
    case CHECKING:
        if (st == UI_UPG_FILE_FOUND)
        {
            status("BIN FOUND", 20);
            Ui_Post(UI_UPGRADE_SET_FUSE, 0);
            step = SETTING_FUSE;
        }
        else if (st == UI_UPG_NO_FILE)
        {
            status("NO BINARY", 0);
            step = DONE;
        }
        break;
    case SETTING_FUSE:
        if (st == UI_UPG_FUSE_SET)
        {
            status("FUSE SET", 75);
            step = FUSE_SET;
        }
        else if (st == UI_UPG_FUSE_FAILED)
        {
            status("FUSE FAIL", 0);
            step = DONE;
        }
        break;
    case FUSE_SET:
        status("REBOOTING", 100);
        Ui_Post(UI_REBOOT, 0);
        step = REBOOTING;
        break;
    default:
        break;
    }
    stepAt = now;
}

void UpgradeView::handleKeyEvent(uint8_t key)
{
    if (key == ui::KEY_BACK && step != REBOOTING)
    {
        ui::nav = ui::NAV_ABOUT;
        application().gotoUsernavScreenNoTransition();
    }
    else if (key == ui::KEY_ENTER && (step == IDLE || step == DONE))
    {
        status("CHECKING", 5);
        Ui_Post(UI_UPGRADE_CHECK, 0);
        step = CHECKING;
        stepAt = Ui_Millis();
    }
}
