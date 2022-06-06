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

#ifndef DI8JOY_IMPL_HPP
#define DI8JOY_IMPL_HPP

// author: Daniel Hug, 2022

// implements the the direct input backend services of the di8joy library

#include "di8joy.hpp"
#include "di8joy_impl.hpp"

// // for static linking
// #pragma comment(lib, "dinput8.lib")

// the next line is essential to find the right windows functions
#define UNICODE

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef WINVER
#define WINVER 0x0501
#endif

#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

#include <iosfwd>

namespace hd
{

std::ostream &err();

namespace priv
{

struct JoystickCaps
{
    unsigned int buttonCount{0};      // actual number of buttons supported by the joystick (max. 128)
    unsigned int povCount{0};         // actual number of pov hats (max. 4)
    bool axes[Joystick::AxisCount]{}; // support for each axis (max. 8)
};

struct JoystickState
{
    bool connected{false};                 // Is the joystick currently connected?
    float axes[Joystick::AxisCount]{};     // Position of each axis, in range [-100.f, 100.f]
    int povs[Joystick::PovCount]{};        // Position of each pov hat (-1 for center pos, otherwise in deg starting from top with 0 in counter clockwise direction):
                                           // center: -1, up: 0, U/R: 45, R: 90, D/R: 135, D: 180, D/L: 225, L: 270, U/L: 315
    bool buttons[Joystick::ButtonCount]{}; // Status of each button (true = pressed)
};

class JoystickImpl
{
  public:
    static void initialize(); // global initialization

    static void cleanup(); // global cleanup

    static bool isConnected(unsigned int index);

    static void setLazyUpdates(bool status); // enable lazy update based on windows triggered enumeration updates

    static void updateConnections(); // Update the connection status of all joysticks

    [[nodiscard]] bool open(unsigned int index); // open joystick for reading status updates

    void close();

    JoystickCaps getCapabilities() const;

    Joystick::Identification getIdentification() const;

    [[nodiscard]] JoystickState update();

    static void initializeDInput(); // global direct input initialization

    static void cleanupDInput(); // global cleanup of direct input

    static bool isConnectedDInput(unsigned int index);

    static void updateConnectionsDInput();

    [[nodiscard]] bool openDInput(unsigned int index);

    void closeDInput();

    JoystickCaps getCapabilitiesDInput() const;

    [[nodiscard]] JoystickState updateDInputBuffered();

    [[nodiscard]] JoystickState updateDInputPolled();

  private:
    static BOOL CALLBACK deviceEnumerationCallback(const DIDEVICEINSTANCE *deviceInstance, void *userData);

    static BOOL CALLBACK deviceObjectEnumerationCallback(const DIDEVICEOBJECTINSTANCE *deviceObjectInstance, void *userData);

    // Member data

    unsigned int m_index;                      // Index of the joystick
    IDirectInputDevice8W *m_device;            // DirectInput 8.x device
    DIDEVCAPS m_deviceCaps;                    // DirectInput device capabilities
    int m_axes[Joystick::AxisCount];           // Offsets to the bytes containing the axes states, -1 if not available
    int m_povs[Joystick::PovCount];            // Offsets to the bytes containing the pov states, -1 if not available
    int m_buttons[Joystick::ButtonCount];      // Offsets to the bytes containing the button states, -1 if not available
    Joystick::Identification m_identification; // Joystick identification
    JoystickState m_state;                     // Buffered joystick state
    bool m_buffered;                           // true if the device uses buffering, false if the device uses polling
};

} // namespace priv

} // namespace hd

#endif // DI8JOY_IMPL_HPP

////////////////////////////////////////////////////////////
///
/// By default, hd::err() outputs to the same location as std::cerr,
/// (-> the stderr descriptor) which is the console if there's
/// one available.
///
/// It is a standard std::ostream instance, so it supports all the
/// insertion operations defined by the STL
/// (operator <<, manipulators, etc.).
///
/// hd::err() can be redirected to write to another output, independently
/// of std::cerr, by using the rdbuf() function provided by the
/// std::ostream class.
///
/// Example:
/// \code
/// // Redirect to a file
/// std::ofstream file("hd-log.txt");
/// std::streambuf* previous = hd::err().rdbuf(file.rdbuf());
///
/// // Redirect to nothing
/// hd::err().rdbuf(nullptr);
///
/// // Restore the original output
/// hd::err().rdbuf(previous);
/// \endcode
///
/// \return Reference to std::ostream representing the error stream
///
////////////////////////////////////////////////////////////