#ifndef COLORSPRESENTER_HPP
#define COLORSPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class ColorsView;

class ColorsPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    ColorsPresenter(ColorsView& v);

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

    virtual ~ColorsPresenter() {};

private:
    ColorsPresenter();

    ColorsView& view;
};

#endif // COLORSPRESENTER_HPP
