#include <stdio.h>
#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <string>

class DiJoyStick
{
  public:
    struct Entry
    {
        DIDEVICEINSTANCE diDeviceInstance;
        DIDEVCAPS diDevCaps;
        LPDIRECTINPUTDEVICE8 diDevice;
        DIJOYSTATE2 joystate;
    };

    DiJoyStick() : entry(0), maxEntry(0), nEntry(0), di(0)
    {
    }

    ~DiJoyStick()
    {
        clear();
    }

    void clear()
    {
        if (entry)
        {
            delete[] entry;
            entry = 0;
        }
        maxEntry = 0;
        nEntry = 0;
        di = 0;
    }

    void enumerate(LPDIRECTINPUT di, DWORD dwDevType = DI8DEVTYPE_JOYSTICK, LPCDIDATAFORMAT lpdf = &c_dfDIJoystick2, DWORD dwFlags = DIEDFL_ATTACHEDONLY, int maxEntry = 16)
    {
        clear();

        entry = new Entry[maxEntry];
        this->di = di;
        this->maxEntry = maxEntry;
        nEntry = 0;
        this->lpdf = lpdf;

        di->EnumDevices(dwDevType, DIEnumDevicesCallback_static, this, dwFlags);

        this->di = 0;
    }

    int getEntryCount() const
    {
        return nEntry;
    }

    const Entry *getEntry(int index) const
    {
        const Entry *e = 0;
        if (index >= 0 && index < nEntry)
        {
            e = &entry[index];
        }
        return e;
    }

    void update()
    {
        for (int iEntry = 0; iEntry < nEntry; ++iEntry)
        {
            Entry &e = entry[iEntry];
            LPDIRECTINPUTDEVICE8 d = e.diDevice;

            if (FAILED(d->Poll()))
            {
                HRESULT hr = d->Acquire();
                while (hr == DIERR_INPUTLOST)
                {
                    hr = d->Acquire();
                }
            }
            else
            {
                d->GetDeviceState(sizeof(DIJOYSTATE2), &e.joystate);
            }
        }
    }

  protected:
    static BOOL CALLBACK DIEnumDevicesCallback_static(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
    {
        return reinterpret_cast<DiJoyStick *>(pvRef)->DIEnumDevicesCallback(lpddi, pvRef);
    }

    BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
    {
        if (nEntry < maxEntry)
        {
            Entry e = {0};

            memcpy(&e.diDeviceInstance, lpddi, sizeof(e.diDeviceInstance));
            e.diDevCaps.dwSize = sizeof(e.diDevCaps);

            LPDIRECTINPUTDEVICE8 did = 0;

            if (SUCCEEDED(di->CreateDevice(lpddi->guidInstance, (LPDIRECTINPUTDEVICE *)&did, 0)))
            {
                if (SUCCEEDED(did->SetDataFormat(lpdf)))
                {
                    if (SUCCEEDED(did->GetCapabilities(&e.diDevCaps)))
                    {
                        e.diDevice = did;
                        entry[nEntry++] = e;
                    }
                }
            }
        }
        return DIENUM_CONTINUE;
    }

    //
    Entry *entry;
    int maxEntry;
    int nEntry;
    LPDIRECTINPUT di;
    LPCDIDATAFORMAT lpdf;
};

int main(int, const char *)
{
    DiJoyStick djs;
    LPDIRECTINPUT lpDi = 0;

    // get console information
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (SUCCEEDED(DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&lpDi, 0)))
    {
        djs.enumerate(lpDi);


        for (;;)
        {
            djs.update();

            for (int i = 0; i < djs.getEntryCount(); ++i)
            {

                const DiJoyStick::Entry *e = djs.getEntry(i);
                if (e)
                {
                    const DIJOYSTATE2 *js = &e->joystate;

                    int pov = js->rgdwPOV[0];
                    if (pov < 0)
                    {
                        pov = -1;
                    }
                    else
                    {
                        pov /= 100;
                    }
                    std::string outstr{};
                    for (int i = 0; i < 64; ++i)
                    {
                        outstr += js->rgbButtons[i] ? "1" : "0";
                    }

                    printf("i: %2d, pov: %3d, buttons: %s\n", i, pov, outstr.c_str());
                }

                // move cusor back to initial position
                GetConsoleScreenBufferInfo(hConsole, &coninfo);
                coninfo.dwCursorPosition.Y -= djs.getEntryCount(); // move up to first line
                //coninfo.dwCursorPosition.X += 5; // move to the right the length of the word
                SetConsoleCursorPosition(hConsole, coninfo.dwCursorPosition);
            }
        }

        
    }

    return 0;
}