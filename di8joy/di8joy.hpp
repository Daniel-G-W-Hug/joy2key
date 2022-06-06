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

#ifndef DI8JOY_HPP
#define DI8JOY_HPP

// author: Daniel Hug, 2022

// implements the user API of the di8joy library

#include <string>

namespace hd
{

class Joystick
{

  public:
    enum
    {
        Count = 8,         // max. number of supported joysticks
        ButtonCount = 128, // max. number of supported buttons (incl. POV states mapped to buttons)
        PovCount = 4,      // max. number of supported pov hats
        AxisCount = 8      // max. number of supported axes
    };

    enum Axis
    {
        X,  // X axis
        Y,  // Y axis
        Z,  // Z axis
        Rx, // Rx axis
        Ry, // Ry axis
        Rz, // Rz axis
        S0, // first slider
        S1  // second slider
    };

    struct Identification
    {
        Identification();

        std::wstring name;      // Name of the joystick
        unsigned int vendorId;  // Manufacturer identifier
        unsigned int productId; // Product identifier
    };

    static bool isConnected(unsigned int joystick);

    static unsigned int getButtonCount(unsigned int joystick);

    static unsigned int getPovCount(unsigned int joystick);

    static bool hasAxis(unsigned int joystick, Axis axis);

    static bool isButtonPressed(unsigned int joystick, unsigned int button);

    static int getPovPosition(unsigned int joystick, unsigned int pov);

    static float getAxisPosition(unsigned int joystick, Axis axis);

    static Identification getIdentification(unsigned int joystick);

    static void update(); // normally used internally.
                          // to be called if you have no window
                          // joystick state is not updated automatically
};

} // namespace hd

#endif // DI8JOY_HPP