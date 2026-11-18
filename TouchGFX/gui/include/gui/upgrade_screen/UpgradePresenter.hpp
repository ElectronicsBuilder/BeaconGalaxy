#ifndef UPGRADEPRESENTER_HPP
#define UPGRADEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class UpgradeView;

class UpgradePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    UpgradePresenter(UpgradeView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~UpgradePresenter() {};

private:
    UpgradePresenter();

    UpgradeView& view;
};

#endif // UPGRADEPRESENTER_HPP
