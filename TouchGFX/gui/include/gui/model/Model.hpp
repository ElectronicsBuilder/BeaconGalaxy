/*
 * Model.hpp - TouchGFX model. BeaconGalaxy keeps its state outside the GUI
 * (App/Ui/ui_bridge.h): views read status through the bridge and post typed
 * commands to it, so the model only holds the listener binding.
 */
#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
