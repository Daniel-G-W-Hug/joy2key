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

bool js::isConnected(unsigned int jsIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getState(jsIdx).connected;
}

unsigned int js::getButtonCount(unsigned int jsIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getCapabilities(jsIdx).nButton;
}

unsigned int js::getPovCount(unsigned int jsIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getCapabilities(jsIdx).nPOV;
}

bool js::hasAxis(unsigned int jsIdx, Axis axisIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getCapabilities(jsIdx).axes[axisIdx];
}

bool js::isButtonPressed(unsigned int jsIdx, unsigned int buttonIdx)
{
    assert(jsIdx < js::max_nJoystick);
    assert(buttonIdx < js::max_nButton);
    return priv::jsMngr::getInstance().getState(jsIdx).buttons[buttonIdx];
}

int js::getPovPosition(unsigned int jsIdx, unsigned int povIdx)
{
    assert(jsIdx < js::max_nJoystick);
    assert(povIdx < js::max_nPOV);
    return priv::jsMngr::getInstance().getState(jsIdx).povs[povIdx];
}

float js::getAxisPosition(unsigned int jsIdx, Axis axisIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getState(jsIdx).axes[axisIdx];
}

js::Id js::getId(unsigned int jsIdx)
{
    assert(jsIdx < js::max_nJoystick);
    return priv::jsMngr::getInstance().getId(jsIdx);
}

void js::update()
{
    return priv::jsMngr::getInstance().update();
}

} // namespace hd