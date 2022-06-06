#include "di8joy/di8joy.hpp"
#include "string_trim.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

int main()
{

    //get information about the joystick
    std::cout << "\nGet info on connected joysticks:\n";

    // update must be called once to initialize the joystick library ldi8joy
    hd::Joystick::update();

    int num_sticks{0};
    hd::Joystick::Identification id[hd::Joystick::Count]; // storage for hd::Joystick::Count joysticks

    for (int i = 0; i < hd::Joystick::Count; ++i)
    {
        id[i] = hd::Joystick::getIdentification(i);
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
            if (hd::Joystick::isConnected(i))
            {
                // check how many buttons joystick number 0 has
                unsigned int buttonCount = hd::Joystick::getButtonCount(i);
                unsigned int povCount = hd::Joystick::getPovCount(i);

                // check if joystick number 0 has a Z axis
                bool hasZ = hd::Joystick::hasAxis(i, hd::Joystick::Z);

                std::cout << "Button count: " << buttonCount << std::endl;
                std::cout << "Pov count: " << povCount << std::endl;

                int numAxes{0};
                std::string nameAxes{};
                if (hd::Joystick::hasAxis(i, hd::Joystick::X))
                {
                    ++numAxes;
                    nameAxes += " X";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::Y))
                {
                    ++numAxes;
                    nameAxes += " Y";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::Z))
                {
                    ++numAxes;
                    nameAxes += " Z";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::Rx))
                {
                    ++numAxes;
                    nameAxes += " Rx";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::Ry))
                {
                    ++numAxes;
                    nameAxes += " Ry";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::Rz))
                {
                    ++numAxes;
                    nameAxes += " Rz";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::S0))
                {
                    ++numAxes;
                    nameAxes += " S0";
                }
                if (hd::Joystick::hasAxis(i, hd::Joystick::S1))
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

    for (unsigned int i = 0; i < hd::Joystick::Count; ++i)
    {
        if (hd::Joystick::isConnected(i))
        {
            hd::Joystick::update();
            std::cout << "Joystick " << i << ": ";
            std::string outstr{};
            for (unsigned int j = 0; j < hd::Joystick::getButtonCount(i); ++j)
            {
                outstr += (hd::Joystick::isButtonPressed(i, j)) ? "1" : "0";
            }
            std::cout << outstr << std::endl;
        }
    }

    return 0;
}