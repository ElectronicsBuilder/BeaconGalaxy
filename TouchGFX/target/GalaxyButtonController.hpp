/*
 * GalaxyButtonController.hpp - feeds the front buttons (and keys injected from
 * the console with "key ...") to TouchGFX. Lives outside target/generated so
 * code generation never removes it.
 *
 * TouchGFX key codes (PhysicalButtons in BeaconGalaxy.touchgfx):
 *   1 = ENTER, 2 = BACK, 3 = UP, 4 = DOWN
 */
#ifndef GALAXYBUTTONCONTROLLER_HPP
#define GALAXYBUTTONCONTROLLER_HPP

#include <platform/driver/button/ButtonController.hpp>

class GalaxyButtonController : public touchgfx::ButtonController
{
public:
    virtual void init() {}
    virtual bool sample(uint8_t& key);
};

#endif
