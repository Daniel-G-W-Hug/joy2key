////////////////////////////////////////////////////////////
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

#include "di8joy_mngr.hpp"

namespace hd
{
namespace priv
{

jsMngr &jsMngr::getInstance()
{
    static jsMngr instance;
    return instance;
}

const jsCaps &jsMngr::getCapabilities(unsigned int jsIdx) const
{
    return m_joysticks[jsIdx].capabilities;
}

const jsState &jsMngr::getState(unsigned int jsIdx) const
{
    return m_joysticks[jsIdx].state;
}

const js::Id &jsMngr::getId(unsigned int jsIdx) const
{
    return m_joysticks[jsIdx].identification;
}

void jsMngr::update()
{
    for (unsigned int i = 0; i < js::max_nJoystick; ++i)
    {
        jsDevice &device = m_joysticks[i];

        if (device.state.connected)
        {
            // Get the current state of the joystick
            device.state = device.joystick.update();

            // Check if it's still connected
            if (!device.state.connected)
            {
                device.joystick.close();
                device.capabilities = jsCaps();
                device.state = jsState();
                device.identification = js::Id();
            }
        }
        else
        {
            // Check if the joystick was connected since last update
            if (jsImpl::isConnected(i))
            {
                if (device.joystick.open(i))
                {
                    device.capabilities = device.joystick.getCapabilities();
                    device.state = device.joystick.update();
                    device.identification = device.joystick.getId();
                }
            }
        }
    }
}

jsMngr::jsMngr()
{
    jsImpl::initialize();
}

jsMngr::~jsMngr()
{
    for (jsDevice &device : m_joysticks)
    {
        if (device.state.connected)
            device.joystick.close();
    }

    jsImpl::cleanup();
}

} // namespace priv

} // namespace hd