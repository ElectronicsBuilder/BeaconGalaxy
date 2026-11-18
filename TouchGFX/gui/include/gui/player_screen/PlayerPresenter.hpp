#ifndef PLAYERPRESENTER_HPP
#define PLAYERPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class PlayerView;

class PlayerPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    PlayerPresenter(PlayerView& v);

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

    virtual ~PlayerPresenter() {};

private:
    PlayerPresenter();

    PlayerView& view;
};

#endif // PLAYERPRESENTER_HPP
