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

#ifndef DI8JOY_MNGR_HPP
#define DI8JOY_MNGR_HPP

#include "di8joy.hpp"
#include "di8joy_impl.hpp"

namespace hd
{
namespace priv
{

class JoystickMngr
{
  public:
    static JoystickMngr &getInstance();

    const JoystickCaps &getCapabilities(unsigned int joystick) const;

    const JoystickState &getState(unsigned int joystick) const;

    const Joystick::Identification &getIdentification(unsigned int joystick) const;

    void update();

  private:
    JoystickMngr();
    ~JoystickMngr();
    JoystickMngr(const JoystickMngr &) = delete;
    JoystickMngr &operator=(const JoystickMngr &) = delete;

    struct Item
    {
        JoystickImpl joystick;                   // Joystick implementation
        JoystickState state;                     // Current joystick state
        JoystickCaps capabilities;               // Joystick capabilities
        Joystick::Identification identification; // Joystick identification
    };

    Item m_joysticks[Joystick::Count]; // Joysticks information and state
};

} // namespace priv

} // namespace hd

#endif // DI8JOY_MNGR_HPP