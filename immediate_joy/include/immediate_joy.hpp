#ifndef HD_IMMEDIATE_JOY_HPP
#define HD_IMMEDIATE_JOY_HPP

////////////////////////////////////////////////////////////////////////////////
// Library: immediate_joy (Direct Input 8 library for immediate mode interface)
//
// author: Daniel Hug, 08/2022
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// user inteface of immediate_joy library
////////////////////////////////////////////////////////////////////////////////

#include <string>

namespace hd
{

class js
{

  public:
    enum
    {
        MAX_NJOYSTICK = 8, // max. number of supported joysticks
        MAX_NAXIS = 8,     // max. number of supported axes
        MAX_NPOV = 4,      // max. number of supported POV hats
        MAX_NBUTTON = 128  // max. number of supported buttons
    };

    enum axis
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

    struct dev_id // device id
    {
        std::wstring name{L"No Joystick"}; // Name of the joystick
        unsigned int vendorId{0};          // Manufacturer identifier
        unsigned int productId{0};         // Product identifier
    };

    struct dev_caps // device capabilities
    {
        bool axes[MAX_NAXIS]{};  // actual support for number of axis (max. MAX_NAXIS)
        unsigned int nPOV{0};    // actual number of pov hats (max. MAX_NPOV)
        unsigned int nButton{0}; // actual number of buttons supported by the joystick (max. MAX_NBUTTON)
    };

    struct dev_state
    {
        bool connected{false};       // Is the joystick currently connected?
        float axes[MAX_NAXIS]{};     // Current position of each axis, in range [-100.f, 100.f]
        int povs[MAX_NPOV]{};        // Current position of each pov hat
                                     // (-1 for center pos
                                     // otherwise in deg starting from top with 0 in clockwise direction):
                                     // center: -1, up: 0, U/R: 45, R: 90, D/R: 135, D: 180, D/L: 225, L: 270, U/L: 315
        bool buttons[MAX_NBUTTON]{}; // Status of each button (true = pressed)
    };
};

} // namespace hd

#endif // HD_IMMEDIATE_JOY_HPP