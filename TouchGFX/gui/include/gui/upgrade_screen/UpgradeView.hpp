#ifndef UPGRADEVIEW_HPP
#define UPGRADEVIEW_HPP

#include <gui_generated/upgrade_screen/UpgradeViewBase.hpp>
#include <gui/upgrade_screen/UpgradePresenter.hpp>

/* Firmware update: ENTER looks for BeaconGalaxy.bin on the SD card; if found,
 * sets the GalaxyBoot fuse and reboots into the bootloader. Each step is shown
 * for half a second (the old screen's pacing) without blocking the GUI task. */
class UpgradeView : public UpgradeViewBase
{
public:
    UpgradeView();
    virtual ~UpgradeView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    enum Step { IDLE, CHECKING, FOUND, SETTING_FUSE, FUSE_SET, REBOOTING, DONE };
    void status(const char* text, int progress);
    Step step;
    uint32_t stepAt;
};

#endif // UPGRADEVIEW_HPP
