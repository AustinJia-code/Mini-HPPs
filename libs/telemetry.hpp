/**
 * @file telemetry.hpp
 * @author Austin Jia
 * @brief Small console telemetry solution.
 * @namespace telem
 * 
 * @features
 *      - Telemetry class for printing dynamic console output
 *      - Helper functions for tracking variables and printing strings
 *      - Optional output to logging stream
 * 
 * @details
 *      - Sizes based on initial console sizes and will not adjust
 */

#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <sys/ioctl.h>
#include <unistd.h>
#include <optional>
#include <sstream>

namespace telem
{

/**
 * Returns a function that allows telemetry to track a variable
 */
template<typename T>
std::function<std::string()> telem_var (const std::string& label, const T& var)
{
    return [&label, &var]
    {
        std::ostringstream ss;
        ss << label << ": " << value;
        return ss.str ();
    };
}

/**
 * Returns a function that tells telemetry to print a string
 */
std::function<std::string()> telem_str (const std::string& data)
{
    return [&data]
    {
        return data;
    };
}

/**
 * Telemetry class
 */
class Telemetry
{
private:
    std::vector<std::function<std::string ()>> lines;
    int start_line;

    /**
     * System helper for terminal dimensions
     */
    int get_terminal_rows ()
    {
        struct winsize w;
        ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }

public:
    /**
     * Init with list of line-generating functions.
     */
    Telemetry (std::vector<std::function<std::string ()>> lines)
        : lines (std::move (lines))
    {
        start_line = get_terminal_rows () - lines.size () + 1;

        // restrict scroll area
        std::cout << "\033[1;" << (start_line - 1) << "r";
    }

    /**
     * Refresh console output, outputs to an additional optional ostream for
     * logging
     */
    void refresh (std::optional<std::ostream&> log_stream = std::nullopt)
    {
        for (size_t i = 0; i < lines.size(); i++)
        {
            std::cout << "\033[" << (start_line + i) << ";1H";
            std::cout << "\033[2K";
            std::cout << lines[i] ();

            // Optional logging
            if (log_stream)
            {
                (*log_stream) << lines[i] () << "\n";
                if (i == lines.size () - 1)
                    (*log_stream).flush ();
            }
        }

        std::cout.flush ();
    }
};

}