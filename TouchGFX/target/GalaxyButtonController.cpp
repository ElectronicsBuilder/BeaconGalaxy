/*
 * GalaxyButtonController.cpp - see GalaxyButtonController.hpp.
 */
#include <GalaxyButtonController.hpp>
extern "C" {
#include "buttons.h"
}

bool GalaxyButtonController::sample(uint8_t& key)
{
    switch (Buttons_Get())   /* one queued press per frame */
    {
    case KEY_ENTER: key = 1; return true;
    case KEY_BACK:  key = 2; return true;
    case KEY_UP:    key = 3; return true;
    case KEY_DOWN:  key = 4; return true;
    default:        return false;
    }
}
