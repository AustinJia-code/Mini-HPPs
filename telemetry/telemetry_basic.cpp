/**
 * @file telemetry_basic.hpp
 * @author Austin Jia
 * @brief Basic usage of telemetry.
 */

#include "telemetry.hpp"

int main ()
{
    int counter = 0;
    telem::Telemetry telem
    {{
        telem::telem_str ("===== HEADER ====="),
        telem::telem_var ("Counter", counter)
    }};

    while (counter++ < 10)
    {
        telem.refresh ();

        std::cout << "Hello from normal cout!" << std::endl;
        
        usleep (500000);
    }

    return 0;
}