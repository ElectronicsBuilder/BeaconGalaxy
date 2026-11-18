#ifndef THEMESPRESENTER_HPP
#define THEMESPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class themesView;

class themesPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    themesPresenter(themesView& v);

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

    virtual ~themesPresenter() {};

private:
    themesPresenter();

    themesView& view;
};

#endif // THEMESPRESENTER_HPP
