#ifndef HD_IMMEDIATE_JOY_IMPL_HPP
#define HD_IMMEDIATE_JOY_IMPL_HPP

////////////////////////////////////////////////////////////////////////////////
// Library: immediate_joy (Direct Input 8 library for immediate mode interface)
//
// author: Daniel Hug, 08/2022
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// implementation of immediate_joy library
////////////////////////////////////////////////////////////////////////////////

#include "immediate_joy.hpp"

// // for static linking
// #pragma comment(lib, "dinput8.lib")
// #pragma comment(lib, "dxguid.lib")

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

namespace hd
{

class js_impl
{
  public:
    static void initialize(); // global initialization

    static void cleanup(); // global cleanup

  private:
    static BOOL CALLBACK deviceEnumerationCallback(const DIDEVICEINSTANCE *deviceInstance, void *userData);

    static BOOL CALLBACK deviceObjectEnumerationCallback(const DIDEVICEOBJECTINSTANCE *deviceObjectInstance, void *userData);

    unsigned int m_index;           // Index of the joystick
    IDirectInputDevice8W *m_device; // DirectInput 8.x device
    DIDEVCAPS m_deviceCaps;         // DirectInput device capabilities
    int m_axes[js::MAX_NAXIS];      // Offsets to the bytes containing the axes states, -1 if not available
    int m_povs[js::MAX_NPOV];       // Offsets to the bytes containing the pov states, -1 if not available
    int m_buttons[js::MAX_NBUTTON]; // Offsets to the bytes containing the button states, -1 if not available
    js::dev_id m_id;                // Joystick identification
    js::dev_state m_state;          // Buffered joystick state
};

} // namespace hd

#endif // HD_IMMEDIATE_JOY_IMPL_HPP