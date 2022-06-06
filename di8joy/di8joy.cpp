/////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2022 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
//
// The SFML routines related to joysticks on WIN32 from
// SFML V2.5.1 have been used as a basis for this library
//
////////////////////////////////////////////////////////////
//
// author of modifications: Daniel Hug, 2022
//
// changes vs. orginial code documented here:
// "Changes_vs_SFML_2_5_1.txt"
//
////////////////////////////////////////////////////////////

// implements the user API of the di8joy library

#include "di8joy.hpp"
#include "di8joy_mngr.hpp"

#include <cassert>

namespace hd
{

bool Joystick::isConnected(unsigned int joystick)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getState(joystick).connected;
}

unsigned int Joystick::getButtonCount(unsigned int joystick)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getCapabilities(joystick).buttonCount;
}

unsigned int Joystick::getPovCount(unsigned int joystick)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getCapabilities(joystick).povCount;
}

bool Joystick::hasAxis(unsigned int joystick, Axis axis)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getCapabilities(joystick).axes[axis];
}

bool Joystick::isButtonPressed(unsigned int joystick, unsigned int button)
{
    assert(joystick < Joystick::Count);
    assert(button < Joystick::ButtonCount);
    return priv::JoystickMngr::getInstance().getState(joystick).buttons[button];
}

int Joystick::getPovPosition(unsigned int joystick, unsigned int pov)
{
    assert(joystick < Joystick::Count);
    assert(pov < Joystick::PovCount);
    return priv::JoystickMngr::getInstance().getState(joystick).povs[pov];
}

float Joystick::getAxisPosition(unsigned int joystick, Axis axis)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getState(joystick).axes[axis];
}

Joystick::Identification Joystick::getIdentification(unsigned int joystick)
{
    assert(joystick < Joystick::Count);
    return priv::JoystickMngr::getInstance().getIdentification(joystick);
}

void Joystick::update()
{
    return priv::JoystickMngr::getInstance().update();
}

Joystick::Identification::Identification() : name{L"No Joystick"},
                                             vendorId{0},
                                             productId{0}
{
}

} // namespace hd