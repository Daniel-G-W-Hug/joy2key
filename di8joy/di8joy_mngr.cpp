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

JoystickMngr &JoystickMngr::getInstance()
{
    static JoystickMngr instance;
    return instance;
}

const JoystickCaps &JoystickMngr::getCapabilities(unsigned int joystick) const
{
    return m_joysticks[joystick].capabilities;
}

const JoystickState &JoystickMngr::getState(unsigned int joystick) const
{
    return m_joysticks[joystick].state;
}

const Joystick::Identification &JoystickMngr::getIdentification(unsigned int joystick) const
{
    return m_joysticks[joystick].identification;
}

void JoystickMngr::update()
{
    for (unsigned int i = 0; i < Joystick::Count; ++i)
    {
        Item &item = m_joysticks[i];

        if (item.state.connected)
        {
            // Get the current state of the joystick
            item.state = item.joystick.update();

            // Check if it's still connected
            if (!item.state.connected)
            {
                item.joystick.close();
                item.capabilities = JoystickCaps();
                item.state = JoystickState();
                item.identification = Joystick::Identification();
            }
        }
        else
        {
            // Check if the joystick was connected since last update
            if (JoystickImpl::isConnected(i))
            {
                if (item.joystick.open(i))
                {
                    item.capabilities = item.joystick.getCapabilities();
                    item.state = item.joystick.update();
                    item.identification = item.joystick.getIdentification();
                }
            }
        }
    }
}

JoystickMngr::JoystickMngr()
{
    JoystickImpl::initialize();
}

JoystickMngr::~JoystickMngr()
{
    for (Item &item : m_joysticks)
    {
        if (item.state.connected)
            item.joystick.close();
    }

    JoystickImpl::cleanup();
}

} // namespace priv

} // namespace hd