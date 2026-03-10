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
        telem::telem_var ("Counter", counter),
        telem::telem_str ("===== FOOTER =====")
    }};

    while (counter++ < 100)
    {
        telem.refresh ();

        std::cout << "Hello from normal cout at count " << counter << std::endl;
        
        usleep (100000);
    }

    return 0;
}