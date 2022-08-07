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

// implements the the direct input 8 backend services of the di8joy library

#include "di8joy_impl.hpp"

// all the stuff for err()
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <streambuf>
#include <vector>

////////////////////////////////////////////////////////////
// DirectInput
////////////////////////////////////////////////////////////

#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL 0x80000000
#endif

namespace
{

// anonymous namespace for things to be kept private to this translation unit

namespace guids
{
const GUID IID_IDirectInput8W = {0xbf798031, 0x483a, 0x4da2, {0xaa, 0x99, 0x5d, 0x64, 0xed, 0x36, 0x97, 0x00}};

const GUID GUID_XAxis = {0xa36d02e0, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_YAxis = {0xa36d02e1, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_ZAxis = {0xa36d02e2, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_RxAxis = {0xa36d02f4, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_RyAxis = {0xa36d02f5, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_RzAxis = {0xa36d02e3, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_Slider = {0xa36d02e4, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
const GUID GUID_POV = {0xa36d02f2, 0xc9f3, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

} // namespace guids

HMODULE dinput8dll = nullptr;
IDirectInput8W *directInput = nullptr;

struct jsRecord
{
    GUID guid;
    unsigned int index;
    bool plugged;
};

using JoystickList = std::vector<jsRecord>;
JoystickList jsList;

struct jsBlacklistEntry
{
    unsigned int vendorId;
    unsigned int productId;
};

using JoystickBlacklist = std::vector<jsBlacklistEntry>;
JoystickBlacklist jsBlacklist;

const DWORD directInputEventBufferSize = 32;

// This class will be used as the default streambuf of hd::Err,
// it outputs to stderr by default (to keep the default behavior)
class DefaultErrStreamBuf : public std::streambuf
{
  public:
    DefaultErrStreamBuf()
    {
        // Allocate the write buffer
        constexpr int size = 64;
        char *buffer = new char[size];
        setp(buffer, buffer + size);
    }

    ~DefaultErrStreamBuf() override
    {
        // Synchronize
        sync();

        // Delete the write buffer
        delete[] pbase();
    }

  private:
    int overflow(int character) override
    {
        if ((character != EOF) && (pptr() != epptr()))
        {
            // Valid character
            return sputc(static_cast<char>(character));
        }
        else if (character != EOF)
        {
            // Not enough space in the buffer: synchronize output and try again
            sync();
            return overflow(character);
        }
        else
        {
            // Invalid character: synchronize output
            return sync();
        }
    }

    int sync() override
    {
        // Check if there is something into the write buffer
        if (pbase() != pptr())
        {
            // Print the contents of the write buffer into the standard error output
            auto size = static_cast<std::size_t>(pptr() - pbase());
            fwrite(pbase(), 1, size, stderr);

            // Reset the pointer position to the beginning of the write buffer
            setp(pbase(), epptr());
        }

        return 0;
    }
};

} // anonymous namespace

namespace hd
{

std::ostream &err()
{
    static DefaultErrStreamBuf buffer;
    static std::ostream stream(&buffer);

    return stream;
}

namespace priv
{
////////////////////////////////////////////////////////////
void jsImpl::initialize()
{
    // Try to initialize DirectInput
    initializeDInput();

    if (!directInput)
        err() << "DirectInput not available" << std::endl;

    // Perform the initial scan and populate the connection cache
    updateConnectionsDInput();
}

////////////////////////////////////////////////////////////
void jsImpl::cleanup()
{
    // Clean up DirectInput
    cleanupDInput();
}

////////////////////////////////////////////////////////////
bool jsImpl::isConnected(unsigned int index)
{
    return isConnectedDInput(index);
}

////////////////////////////////////////////////////////////
bool jsImpl::open(unsigned int index)
{
    return openDInput(index);
}

////////////////////////////////////////////////////////////
void jsImpl::close()
{
    if (directInput)
        closeDInput();
}

////////////////////////////////////////////////////////////
jsCaps jsImpl::getCapabilities() const
{
    return getCapabilitiesDInput();
}

////////////////////////////////////////////////////////////
js::Id jsImpl::getId() const
{
    return m_identification;
}

////////////////////////////////////////////////////////////
jsState jsImpl::update()
{

    if (m_buffered)
    {
        return updateDInputBuffered();
    }
    else
    {
        return updateDInputPolled();
    }
}

////////////////////////////////////////////////////////////
void jsImpl::initializeDInput()
{
    // Try to load dinput8.dll
    dinput8dll = LoadLibraryA("dinput8.dll");

    if (dinput8dll)
    {
        // Try to get the address of the DirectInput8Create entry point
        using DirectInput8CreateFunc = HRESULT(WINAPI *)(HINSTANCE, DWORD, const IID &, LPVOID *, LPUNKNOWN);
        auto directInput8Create = reinterpret_cast<DirectInput8CreateFunc>(reinterpret_cast<void *>(GetProcAddress(dinput8dll, "DirectInput8Create")));

        if (directInput8Create)
        {
            // Try to acquire a DirectInput 8.x interface
            HRESULT result = directInput8Create(GetModuleHandleW(nullptr), 0x0800, guids::IID_IDirectInput8W, reinterpret_cast<void **>(&directInput), nullptr);

            if (FAILED(result))
            {
                // De-initialize everything
                directInput = nullptr;
                FreeLibrary(dinput8dll);
                dinput8dll = nullptr;

                err() << "Failed to initialize DirectInput: " << result << std::endl;
            }
        }
        else
        {
            // Unload dinput8.dll
            FreeLibrary(dinput8dll);
            dinput8dll = nullptr;
        }
    }
}

////////////////////////////////////////////////////////////
void jsImpl::cleanupDInput()
{
    // Release the DirectInput interface
    if (directInput)
    {
        directInput->Release();
        directInput = nullptr;
    }

    // Unload dinput8.dll
    if (dinput8dll)
        FreeLibrary(dinput8dll);
}

////////////////////////////////////////////////////////////
bool jsImpl::isConnectedDInput(unsigned int index)
{
    // Check if a joystick with the given index is in the connected list
    for (const jsRecord &record : jsList)
    {
        if (record.index == index)
            return true;
    }

    return false;
}

////////////////////////////////////////////////////////////
void jsImpl::updateConnectionsDInput()
{
    // Clear plugged flags so we can determine which devices were added/removed
    for (jsRecord &record : jsList)
        record.plugged = false;

    // Enumerate devices
    HRESULT result = directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, &jsImpl::deviceEnumerationCallback, nullptr, DIEDFL_ATTACHEDONLY);

    // Remove devices that were not connected during the enumeration
    for (auto i = jsList.begin(); i != jsList.end();)
    {
        if (!i->plugged)
            i = jsList.erase(i);
        else
            ++i;
    }

    if (FAILED(result))
    {
        err() << "Failed to enumerate DirectInput devices: " << result << std::endl;

        return;
    }

    // Assign unused joystick indices to devices that were newly connected
    for (unsigned int i = 0; i < js::max_nJoystick; ++i)
    {
        for (jsRecord &record : jsList)
        {
            if (record.index == i)
                break;

            if (record.index == js::max_nJoystick)
            {
                record.index = i;
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////
bool jsImpl::openDInput(unsigned int index)
{
    // Initialize DirectInput members
    m_device = nullptr;

    for (int &axis : m_axes)
        axis = -1;

    for (int &pov : m_povs)
        pov = -1;

    for (int &button : m_buttons)
        button = -1;

    std::memset(&m_deviceCaps, 0, sizeof(DIDEVCAPS));
    m_deviceCaps.dwSize = sizeof(DIDEVCAPS);
    m_state = jsState();
    m_buffered = false;

    // Search for a joystick with the given index in the connected list
    for (const jsRecord &record : jsList)
    {
        if (record.index == index)
        {
            // Create device
            HRESULT result = directInput->CreateDevice(record.guid, &m_device, nullptr);

            if (FAILED(result))
            {
                err() << "Failed to create DirectInput device: " << result << std::endl;

                return false;
            }

            // Get vendor and product id of the device
            DIPROPDWORD property;
            std::memset(&property, 0, sizeof(property));
            property.diph.dwSize = sizeof(property);
            property.diph.dwHeaderSize = sizeof(property.diph);
            property.diph.dwHow = DIPH_DEVICE;

            if (SUCCEEDED(m_device->GetProperty(DIPROP_VIDPID, &property.diph)))
            {
                m_identification.productId = HIWORD(property.dwData);
                m_identification.vendorId = LOWORD(property.dwData);

                // Check if device is already blacklisted
                if (m_identification.productId && m_identification.vendorId)
                {
                    for (const jsBlacklistEntry &blacklistEntry : jsBlacklist)
                    {
                        if ((m_identification.productId == blacklistEntry.productId) &&
                            (m_identification.vendorId == blacklistEntry.vendorId))
                        {
                            // Device is blacklisted
                            m_device->Release();
                            m_device = nullptr;

                            return false;
                        }
                    }
                }
            }

            // Get friendly product name of the device
            DIPROPSTRING stringProperty;
            std::memset(&stringProperty, 0, sizeof(stringProperty));
            stringProperty.diph.dwSize = sizeof(stringProperty);
            stringProperty.diph.dwHeaderSize = sizeof(stringProperty.diph);
            stringProperty.diph.dwHow = DIPH_DEVICE;
            stringProperty.diph.dwObj = 0;

            if (SUCCEEDED(m_device->GetProperty(DIPROP_PRODUCTNAME, &stringProperty.diph)))
                m_identification.name = stringProperty.wsz;

            static bool formatInitialized = false;
            static DIDATAFORMAT format;

            if (!formatInitialized)
            {
                const DWORD axisType = DIDFT_AXIS | DIDFT_OPTIONAL | DIDFT_ANYINSTANCE;
                const DWORD povType = DIDFT_POV | DIDFT_OPTIONAL | DIDFT_ANYINSTANCE;
                const DWORD buttonType = DIDFT_BUTTON | DIDFT_OPTIONAL | DIDFT_ANYINSTANCE;

                static DIOBJECTDATAFORMAT data[8 * 4 + 4 + hd::js::max_nButton];

                for (int i = 0; i < 4; ++i)
                {
                    data[8 * i + 0].pguid = &guids::GUID_XAxis;
                    data[8 * i + 1].pguid = &guids::GUID_YAxis;
                    data[8 * i + 2].pguid = &guids::GUID_ZAxis;
                    data[8 * i + 3].pguid = &guids::GUID_RxAxis;
                    data[8 * i + 4].pguid = &guids::GUID_RyAxis;
                    data[8 * i + 5].pguid = &guids::GUID_RzAxis;
                    data[8 * i + 6].pguid = &guids::GUID_Slider;
                    data[8 * i + 7].pguid = &guids::GUID_Slider;
                }

                data[0].dwOfs = DIJOFS_X;
                data[1].dwOfs = DIJOFS_Y;
                data[2].dwOfs = DIJOFS_Z;
                data[3].dwOfs = DIJOFS_RX;
                data[4].dwOfs = DIJOFS_RY;
                data[5].dwOfs = DIJOFS_RZ;
                data[6].dwOfs = DIJOFS_SLIDER(0);
                data[7].dwOfs = DIJOFS_SLIDER(1);
                data[8].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVX);
                data[9].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVY);
                data[10].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVZ);
                data[11].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVRx);
                data[12].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVRy);
                data[13].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lVRz);
                data[14].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglVSlider[0]);
                data[15].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglVSlider[1]);
                data[16].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lAX);
                data[17].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lAY);
                data[18].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lAZ);
                data[19].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lARx);
                data[20].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lARy);
                data[21].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lARz);
                data[22].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglASlider[0]);
                data[23].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglASlider[1]);
                data[24].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFX);
                data[25].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFY);
                data[26].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFZ);
                data[27].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFRx);
                data[28].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFRy);
                data[29].dwOfs = FIELD_OFFSET(DIJOYSTATE2, lFRz);
                data[30].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglFSlider[0]);
                data[31].dwOfs = FIELD_OFFSET(DIJOYSTATE2, rglFSlider[1]);

                for (int i = 0; i < 8 * 4; ++i)
                {
                    data[i].dwType = axisType;
                    data[i].dwFlags = 0;
                }

                for (int i = 0; i < 4; ++i)
                {
                    data[8 * 4 + i].pguid = &guids::GUID_POV;
                    data[8 * 4 + i].dwOfs = static_cast<DWORD>(DIJOFS_POV(static_cast<unsigned int>(i)));
                    data[8 * 4 + i].dwType = povType;
                    data[8 * 4 + i].dwFlags = 0;
                }

                for (int i = 0; i < hd::js::max_nButton; ++i)
                {
                    data[8 * 4 + 4 + i].pguid = nullptr;
                    data[8 * 4 + 4 + i].dwOfs = static_cast<DWORD>(DIJOFS_BUTTON(i));
                    data[8 * 4 + 4 + i].dwType = buttonType;
                    data[8 * 4 + 4 + i].dwFlags = 0;
                }

                format.dwSize = sizeof(DIDATAFORMAT);
                format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
                format.dwFlags = DIDFT_ABSAXIS;
                format.dwDataSize = sizeof(DIJOYSTATE2);
                format.dwNumObjs = 8 * 4 + 4 + hd::js::max_nButton;
                format.rgodf = data;

                formatInitialized = true;
            }

            // Set device data format
            result = m_device->SetDataFormat(&format);

            if (FAILED(result))
            {
                err() << "Failed to set DirectInput device data format: " << result << std::endl;

                m_device->Release();
                m_device = nullptr;

                return false;
            }

            // Get device capabilities
            result = m_device->GetCapabilities(&m_deviceCaps);

            if (FAILED(result))
            {
                err() << "Failed to get DirectInput device capabilities: " << result << std::endl;

                m_device->Release();
                m_device = nullptr;

                return false;
            }

            // set the cooperative level to let Direct Input know how this device
            // interacts with the system and with other Direct Input devices (non-exlusive & background)
            // result = m_device->SetCooperativeLevel(???, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND );
            // if (FAILED(result))
            //     return result;

            // Enumerate device objects (axes/povs/buttons)
            result = m_device->EnumObjects(&jsImpl::deviceObjectEnumerationCallback, this, DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);

            if (FAILED(result))
            {
                err() << "Failed to enumerate DirectInput device objects: " << result << std::endl;

                m_device->Release();
                m_device = nullptr;

                return false;
            }

            // Set device's axis mode to absolute if the device reports having at least one axis
            for (int axis : m_axes)
            {
                if (axis != -1)
                {
                    std::memset(&property, 0, sizeof(property));
                    property.diph.dwSize = sizeof(property);
                    property.diph.dwHeaderSize = sizeof(property.diph);
                    property.diph.dwHow = DIPH_DEVICE;
                    property.diph.dwObj = 0;

                    result = m_device->GetProperty(DIPROP_AXISMODE, &property.diph);

                    if (FAILED(result))
                    {
                        std::string outstr;
                        std::transform(m_identification.name.begin(), m_identification.name.end(),
                                       std::back_inserter(outstr),
                                       [](wchar_t c) { return (char)c; });
                        err() << "Failed to get DirectInput device axis mode for device "
                              << outstr << ": " << result << std::endl;

                        m_device->Release();
                        m_device = nullptr;

                        return false;
                    }

                    // If the axis mode is already set to absolute we don't need to set it again ourselves
                    if (property.dwData == DIPROPAXISMODE_ABS)
                        break;

                    std::memset(&property, 0, sizeof(property));
                    property.diph.dwSize = sizeof(property);
                    property.diph.dwHeaderSize = sizeof(property.diph);
                    property.diph.dwHow = DIPH_DEVICE;
                    property.dwData = DIPROPAXISMODE_ABS;

                    m_device->SetProperty(DIPROP_AXISMODE, &property.diph);

                    // Check if the axis mode has been set to absolute
                    std::memset(&property, 0, sizeof(property));
                    property.diph.dwSize = sizeof(property);
                    property.diph.dwHeaderSize = sizeof(property.diph);
                    property.diph.dwHow = DIPH_DEVICE;
                    property.diph.dwObj = 0;

                    result = m_device->GetProperty(DIPROP_AXISMODE, &property.diph);

                    if (FAILED(result))
                    {
                        std::string outstr;
                        std::transform(m_identification.name.begin(), m_identification.name.end(),
                                       std::back_inserter(outstr),
                                       [](wchar_t c) { return (char)c; });
                        err() << "Failed to verify DirectInput device axis mode for device "
                              << outstr << ": " << result << std::endl;

                        m_device->Release();
                        m_device = nullptr;

                        return false;
                    }

                    // If the axis mode hasn't been set to absolute fail here and blacklist the device
                    if (property.dwData != DIPROPAXISMODE_ABS)
                    {
                        if (m_identification.vendorId && m_identification.productId)
                        {
                            jsBlacklistEntry entry;

                            entry.vendorId = m_identification.vendorId;
                            entry.productId = m_identification.productId;

                            jsBlacklist.push_back(entry);
                            jsBlacklist.shrink_to_fit();
                        }

                        m_device->Release();
                        m_device = nullptr;

                        return false;
                    }

                    break;
                }
            }

            // Try to enable buffering by setting the buffer size
            std::memset(&property, 0, sizeof(property));
            property.diph.dwSize = sizeof(property);
            property.diph.dwHeaderSize = sizeof(property.diph);
            property.diph.dwHow = DIPH_DEVICE;
            property.dwData = directInputEventBufferSize;

            result = m_device->SetProperty(DIPROP_BUFFERSIZE, &property.diph);

            if (result == DI_OK)
            {
                // Buffering supported
                m_buffered = true;
            }
            else if (result == DI_POLLEDDEVICE)
            {
                // Only polling supported
                m_buffered = false;
            }
            else
            {
                std::string outstr;
                std::transform(m_identification.name.begin(), m_identification.name.end(),
                               std::back_inserter(outstr),
                               [](wchar_t c) { return (char)c; });
                err() << "Failed to set DirectInput device buffer size for device "
                      << outstr << ": " << result << std::endl;

                m_device->Release();
                m_device = nullptr;

                return false;
            }

            // std::cout << "buffered = " << m_buffered << std::endl;

            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////
void jsImpl::closeDInput()
{
    if (m_device)
    {
        // Release the device
        m_device->Release();
        m_device = nullptr;
    }
}

////////////////////////////////////////////////////////////
jsCaps jsImpl::getCapabilitiesDInput() const
{
    jsCaps caps;

    // Count how many buttons have valid offsets
    caps.nButton = 0;
    for (int button : m_buttons)
    {
        if (button != -1)
            ++caps.nButton;
    }

    // Count how many pov hats have valid offsets
    caps.nPOV = 0;
    for (int pov : m_povs)
    {
        if (pov != -1)
            ++caps.nPOV;
    }

    // Check which axes have valid offsets
    for (int i = 0; i < js::max_nAxis; ++i)
        caps.axes[i] = (m_axes[i] != -1);

    return caps;
}

////////////////////////////////////////////////////////////
jsState jsImpl::updateDInputBuffered()
{

    // If we don't make it to the end of this function, mark the device as disconnected
    m_state.connected = false;

    if (!m_device)
        return m_state;

    DIDEVICEOBJECTDATA events[directInputEventBufferSize];
    DWORD eventCount = directInputEventBufferSize;

    // Try to get the device data
    HRESULT result = m_device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), events, &eventCount, 0);

    // If we have not acquired or have lost the device, attempt to (re-)acquire it and get the device data again
    if ((result == DIERR_NOTACQUIRED) || (result == DIERR_INPUTLOST))
    {
        m_device->Acquire();
        result = m_device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), events, &eventCount, 0);
    }

    // If we still can't get the device data, assume it has been disconnected
    if ((result == DIERR_NOTACQUIRED) || (result == DIERR_INPUTLOST))
    {
        m_device->Release();
        m_device = nullptr;

        return m_state;
    }

    if (FAILED(result))
    {
        err() << "Failed to get DirectInput device data: " << result << std::endl;

        return m_state;
    }

    // Iterate through all buffered events
    for (DWORD i = 0; i < eventCount; ++i)
    {
        bool eventHandled = false;

        // Get the current state of each axis
        for (int j = 0; j < js::max_nAxis; ++j)
        {
            if (m_axes[j] == static_cast<int>(events[i].dwOfs))
            {
                // if ((j == Axis::PovX) || (j == Axis::PovY))
                // {
                //     unsigned short value = LOWORD(events[i].dwData);

                //     if (value != 0xFFFF)
                //     {
                //         float angle = (static_cast<float>(value)) * 3.141592654f / DI_DEGREES / 180.f;

                //         m_state.axes[Axis::PovX] = std::sin(angle) * 100.f;
                //         m_state.axes[Axis::PovY] = std::cos(angle) * 100.f;
                //     }
                //     else
                //     {
                //         m_state.axes[Axis::PovX] = 0.f;
                //         m_state.axes[Axis::PovY] = 0.f;
                //     }
                // }
                // else
                {
                    // map axis range to +/-100 (equivalent to 100% of full scale in each direction)
                    m_state.axes[j] = (static_cast<float>(static_cast<short>(events[i].dwData)) + 0.5f) * 100.f / 32767.5f;
                }

                eventHandled = true;
                break;
            }
        }

        if (eventHandled)
            continue;

        // Get the current state of each button
        for (int j = 0; j < js::max_nButton; ++j)
        {
            if (m_buttons[j] == static_cast<int>(events[i].dwOfs))
            {
                m_state.buttons[j] = (events[i].dwData != 0);

                eventHandled = true;
                break;
            }
        }

        if (eventHandled)
            continue;

        // Get the current state of each pov
        for (int j = 0; j < js::max_nPOV; ++j)
        {
            if (m_povs[j] == static_cast<int>(events[i].dwOfs))
            {
                unsigned short value = LOWORD(events[i].dwData);

                if (value != 0xFFFF)
                    m_state.povs[j] = static_cast<int>(value); // angles (in deg)
                else
                    m_state.povs[j] = -1; // -1 indicates center position

                eventHandled = true;
                break;
            }
        }
    }

    m_state.connected = true;

    return m_state;
}

////////////////////////////////////////////////////////////
jsState jsImpl::updateDInputPolled()
{
    jsState state;

    if (m_device)
    {
        // Poll the device
        m_device->Poll();

        DIJOYSTATE2 joystate;

        // Try to get the device state
        HRESULT result = m_device->GetDeviceState(sizeof(joystate), &joystate);

        // If we have not acquired or have lost the device, attempt to (re-)acquire it and get the device state again
        if ((result == DIERR_NOTACQUIRED) || (result == DIERR_INPUTLOST))
        {
            m_device->Acquire();
            m_device->Poll();
            result = m_device->GetDeviceState(sizeof(joystate), &joystate);
        }

        // If we still can't get the device state, assume it has been disconnected
        if ((result == DIERR_NOTACQUIRED) || (result == DIERR_INPUTLOST))
        {
            m_device->Release();
            m_device = nullptr;

            return state;
        }

        if (FAILED(result))
        {
            err() << "Failed to get DirectInput device state: " << result << std::endl;

            return state;
        }

        // Get the current state of each axis
        for (int i = 0; i < js::max_nAxis; ++i)
        {
            if (m_axes[i] != -1)
            {
                // if ((i == Axis::PovX) || (i == Axis::PovY))
                // {
                //     unsigned short value = LOWORD(*reinterpret_cast<const DWORD *>(reinterpret_cast<const char *>(&joystate) + m_axes[i]));

                //     if (value != 0xFFFF)
                //     {
                //         float angle = (static_cast<float>(value)) * 3.141592654f / DI_DEGREES / 180.f;

                //         state.axes[Axis::PovX] = std::sin(angle) * 100.f;
                //         state.axes[Axis::PovY] = std::cos(angle) * 100.f;
                //     }
                //     else
                //     {
                //         state.axes[Axis::PovX] = 0.f;
                //         state.axes[Axis::PovY] = 0.f;
                //     }
                // }
                // else
                {
                    // map axis range to +/-100 (equivalent to 100% of full scale in each direction)
                    state.axes[i] = (static_cast<float>(*reinterpret_cast<const LONG *>(reinterpret_cast<const char *>(&joystate) + m_axes[i])) + 0.5f) * 100.f / 32767.5f;
                }
            }
            else
            {
                state.axes[i] = 0.f;
            }
        }

        // Get the current state of each button
        for (int i = 0; i < js::max_nButton; ++i)
        {
            if (m_buttons[i] != -1)
            {
                BYTE value = *reinterpret_cast<const BYTE *>(reinterpret_cast<const char *>(&joystate) + m_buttons[i]);

                state.buttons[i] = ((value & 0x80) != 0);
            }
            else
            {
                state.buttons[i] = false;
            }
        }

        // Get the current state of each pov
        for (int i = 0; i < js::max_nPOV; ++i)
        {
            if (m_povs[i] != -1)
            {
                unsigned short value = LOWORD(*reinterpret_cast<const DWORD *>(reinterpret_cast<const char *>(&joystate) + m_povs[i]));

                if (value != 0xFFFF)
                    state.povs[i] = static_cast<int>(value); // angles (in deg)
                else
                    state.povs[i] = -1; // -1 indicates center position
            }
        }

        state.connected = true;
    }

    return state;
}

////////////////////////////////////////////////////////////
BOOL CALLBACK jsImpl::deviceEnumerationCallback(const DIDEVICEINSTANCE *deviceInstance, void *)
{
    for (jsRecord &record : jsList)
    {
        if (record.guid == deviceInstance->guidInstance)
        {
            record.plugged = true;

            return DIENUM_CONTINUE;
        }
    }

    jsRecord record = {deviceInstance->guidInstance, hd::js::max_nJoystick, true};
    jsList.push_back(record);

    return DIENUM_CONTINUE;
}

////////////////////////////////////////////////////////////
BOOL CALLBACK jsImpl::deviceObjectEnumerationCallback(const DIDEVICEOBJECTINSTANCE *deviceObjectInstance, void *userData)
{
    hd::priv::jsImpl &joystick = *reinterpret_cast<hd::priv::jsImpl *>(userData);

    if (DIDFT_GETTYPE(deviceObjectInstance->dwType) & DIDFT_AXIS)
    {
        // Axes
        if (deviceObjectInstance->guidType == guids::GUID_XAxis)
            joystick.m_axes[js::Axis::X] = DIJOFS_X;
        else if (deviceObjectInstance->guidType == guids::GUID_YAxis)
            joystick.m_axes[js::Axis::Y] = DIJOFS_Y;
        else if (deviceObjectInstance->guidType == guids::GUID_ZAxis)
            joystick.m_axes[js::Axis::Z] = DIJOFS_Z;
        else if (deviceObjectInstance->guidType == guids::GUID_RxAxis)
            joystick.m_axes[js::Axis::Rx] = DIJOFS_RX;
        else if (deviceObjectInstance->guidType == guids::GUID_RyAxis)
            joystick.m_axes[js::Axis::Ry] = DIJOFS_RY;
        else if (deviceObjectInstance->guidType == guids::GUID_RzAxis)
            joystick.m_axes[js::Axis::Rz] = DIJOFS_RZ;
        else if (deviceObjectInstance->guidType == guids::GUID_Slider)
        {

            for (int i = 0; i < 2; ++i) // max. two sliders
            {
                if (joystick.m_axes[js::Axis::S0 + i] == -1)
                {
                    joystick.m_axes[js::Axis::S0 + i] = DIJOFS_SLIDER(i);
                    break;
                }
            }
        }
        else
            return DIENUM_CONTINUE;

        // Set the axis' value range to that of a signed short: [-32768, 32767]
        DIPROPRANGE propertyRange;

        std::memset(&propertyRange, 0, sizeof(propertyRange));
        propertyRange.diph.dwSize = sizeof(propertyRange);
        propertyRange.diph.dwHeaderSize = sizeof(propertyRange.diph);
        propertyRange.diph.dwObj = deviceObjectInstance->dwType;
        propertyRange.diph.dwHow = DIPH_BYID;
        propertyRange.lMin = -32768;
        propertyRange.lMax = 32767;

        HRESULT result = joystick.m_device->SetProperty(DIPROP_RANGE, &propertyRange.diph);

        if (result != DI_OK)
            err() << "Failed to set DirectInput device axis property range: " << result << std::endl;

        return DIENUM_CONTINUE;
    }
    else if (DIDFT_GETTYPE(deviceObjectInstance->dwType) & DIDFT_POV)
    {
        // POVs
        if (deviceObjectInstance->guidType == guids::GUID_POV)
        {
            for (int i = 0; i < js::max_nPOV; ++i)
            {
                if (joystick.m_povs[i] == -1)
                {
                    joystick.m_povs[i] = DIJOFS_POV(i);
                    break;
                }
            }
        }

        return DIENUM_CONTINUE;
    }
    else if (DIDFT_GETTYPE(deviceObjectInstance->dwType) & DIDFT_BUTTON)
    {
        // Buttons
        for (int i = 0; i < js::max_nButton; ++i)
        {
            if (joystick.m_buttons[i] == -1)
            {
                joystick.m_buttons[i] = DIJOFS_BUTTON(i);
                break;
            }
        }

        return DIENUM_CONTINUE;
    }

    return DIENUM_CONTINUE;
}

} // namespace priv

} // namespace hd
