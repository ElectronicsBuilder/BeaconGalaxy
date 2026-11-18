#ifndef AUTOPLAYWAVPRESENTER_HPP
#define AUTOPLAYWAVPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class AutoplaywavView;

class AutoplaywavPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    AutoplaywavPresenter(AutoplaywavView& v);

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

    virtual ~AutoplaywavPresenter() {};

private:
    AutoplaywavPresenter();

    AutoplaywavView& view;
};

#endif // AUTOPLAYWAVPRESENTER_HPP
