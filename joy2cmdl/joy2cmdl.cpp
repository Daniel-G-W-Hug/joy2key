#include "di8joy/di8joy.hpp"
#include "string_trim.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <windows.h>

using namespace std::chrono;

int main()
{

    //get information about the joystick
    std::cout << "\nGet info on connected joysticks:\n";

    // update must be called once to initialize the joystick library ldi8joy
    hd::js::update();

    int num_sticks{0};
    hd::js::Id id[hd::js::max_nJoystick]; // storage for hd::Joystick::Count joysticks

    for (int i = 0; i < hd::js::max_nJoystick; ++i)
    {
        //std::cout << "\n\ni = " << i << ":\n";
        id[i] = hd::js::getId(i);
        if (id[i].vendorId != 0)
        {
            ++num_sticks;
            std::wstring controller(L"\nDevice name: " + id[i].name);
            std::string outstr;
            std::transform(controller.begin(), controller.end(),
                           std::back_inserter(outstr),
                           [](wchar_t c) { return (char)c; });
            std::cout << outstr << std::endl;
            std::cout << "Vendor ID: " << id[i].vendorId << std::endl;
            std::cout << "Product ID: " << id[i].productId << std::endl;

            //query joystick for settings if it's plugged in...
            if (hd::js::isConnected(i))
            {
                // check how many buttons joystick number 0 has
                unsigned int nButton = hd::js::getButtonCount(i);
                unsigned int nPOV = hd::js::getPovCount(i);

                // check if joystick number 0 has a Z axis
                bool hasZ = hd::js::hasAxis(i, hd::js::Z);

                std::cout << "Button count: " << nButton << std::endl;
                std::cout << "Pov count: " << nPOV << std::endl;

                int numAxes{0};
                std::string nameAxes{};
                if (hd::js::hasAxis(i, hd::js::X))
                {
                    ++numAxes;
                    nameAxes += " X";
                }
                if (hd::js::hasAxis(i, hd::js::Y))
                {
                    ++numAxes;
                    nameAxes += " Y";
                }
                if (hd::js::hasAxis(i, hd::js::Z))
                {
                    ++numAxes;
                    nameAxes += " Z";
                }
                if (hd::js::hasAxis(i, hd::js::Rx))
                {
                    ++numAxes;
                    nameAxes += " Rx";
                }
                if (hd::js::hasAxis(i, hd::js::Ry))
                {
                    ++numAxes;
                    nameAxes += " Ry";
                }
                if (hd::js::hasAxis(i, hd::js::Rz))
                {
                    ++numAxes;
                    nameAxes += " Rz";
                }
                if (hd::js::hasAxis(i, hd::js::S0))
                {
                    ++numAxes;
                    nameAxes += " S0";
                }
                if (hd::js::hasAxis(i, hd::js::S1))
                {
                    ++numAxes;
                    nameAxes += " S1";
                }

                hd::ltrim(nameAxes);

                std::cout << "Number of axes: " << numAxes << std::endl;
                std::cout << "Names of axes: " << nameAxes << std::endl;
            }
        }
    }

    std::cout << "\nTotal number of connected joysticks: " << num_sticks << "\n\n";

    std::cout << "Press ESC to stop...\n"
              << std::endl;

    // get console information
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &coninfo); // get current console position

    while (true)
    {

        for (unsigned int i = 0; i < hd::js::max_nJoystick; ++i)
        {
            if (hd::js::isConnected(i))
            {
                hd::js::update();
                std::cout << "Joystick " << i << ": ";
                std::string outstr{};
                for (unsigned int j = 0; j < hd::js::getButtonCount(i); ++j)
                {
                    outstr += (hd::js::isButtonPressed(i, j)) ? "1" : "0";
                }
                std::cout << outstr << std::endl;
            }
        }

        // move cusor back to initial position
        // GetConsoleScreenBufferInfo(hConsole, &coninfo);
        // coninfo.dwCursorPosition.Y -= num_sticks; // move up to first line
        //coninfo.dwCursorPosition.X += 5; // move to the right the length of the word
        SetConsoleCursorPosition(hConsole, coninfo.dwCursorPosition);

        // quit on ESC keypress
        if (GetAsyncKeyState(VK_ESCAPE))
            break;

        std::this_thread::sleep_for(100ms);
    }

    // do
    // {
    //     std::cout << '\n'
    //               << "Press a key to continue...";
    // } while (std::cin.get() != '\n');

    return 0;
}