/**
 * @file dashboard.hpp
 * @author Austin Jia
 * @brief Console telemetry implementation.
 * 
 * @warning NOT THREAD SAFE.
 */

#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sstream>

namespace dashboard
{

static constexpr const char* CLEAR_LINE = "\033[2K";

template<typename T>
std::function<std::string()> track_var (const std::string& label, const T& var)
{
    return [&label, &var]
    {
        std::ostringstream ss;
        ss << label << ": " << value;
        return ss.str ();
    };
}

class Telemetry
{
private:
    std::vector<std::function<std::string ()>> lines;
    int start_line;

    int get_terminal_rows()
    {
        struct winsize w;
        ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }

public:
    Telemetry (std::vector<std::function<std::string ()>> lines)
        : lines (std::move (lines))
    {
        start_line = get_terminal_rows() - lines.size() + 1;

        // restrict scroll area
        std::cout << "\033[1;" << (start_line - 1) << "r";
    }

    void refresh()
    {
        for (size_t i = 0; i < lines.size(); i++)
        {
            std::cout << "\033[" << (start_line + i) << ";1H";
            std::cout << CLEAR_LINE;
            std::cout << lines[i] ();
        }

        std::cout.flush ();
    }
};

}