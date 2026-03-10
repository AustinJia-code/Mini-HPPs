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
 *      - Old cout is lost after the console is filled
 *      - Turn of sticky scroll for vscode terminal
 */

#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sstream>

namespace telem
{

/**
 * Returns a function that allows telemetry to track a variable
 */
template<typename T>
std::function<std::string()> telem_var (const std::string& label, const T& var)
{
    return [label, &var]
    {
        std::ostringstream ss;
        ss << label << ": " << var;
        return ss.str ();
    };
}

/**
 * Returns a function that tells telemetry to print a string
 */
std::function<std::string()> telem_str (const std::string& data)
{
    return [data]
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
    int telem_rows;   // number of telemetry lines
    int scroll_start; // first row of scroll region (1-indexed)
    int scroll_end;   // last row of scroll region (1-indexed)
    int total_rows;

    int get_terminal_rows ()
    {
        struct winsize w;
        ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }

    /**
     * Draw all telemetry lines at the top
     */
    void draw_telem ()
    {
        for (int i = 0; i < telem_rows; i++)
        {
            // Save cursor, move to telem row, clear, write, restore
            std::cout << "\033[" << (i + 1) << ";1H"
                      << "\033[2K"
                      << lines[i] ();
        }
    }

public:
    Telemetry (std::vector<std::function<std::string ()>> lines)
        : lines (std::move (lines))
    {
        telem_rows   = this->lines.size ();
        total_rows   = get_terminal_rows ();
        scroll_start = telem_rows + 1;
        scroll_end   = total_rows;

        // Clear screen
        std::cout << "\033[2J";

        // Set scroll region to below telemetry
        std::cout << "\033[" << scroll_start << ";" << scroll_end << "r";

        // Draw initial telemetry
        draw_telem ();

        // Park cursor at start of scroll region
        std::cout << "\033[" << scroll_start << ";1H" << std::flush;
    }

    ~Telemetry ()
    {
        // Restore full scroll region on exit
        std::cout << "\033[1;" << total_rows << "r";
        std::cout << "\033[" << total_rows << ";1H\n" << std::flush;
    }

    /**
     * Refresh telemetry lines at top, keeps cursor in scroll region
     */
    void refresh (std::ostream& log_stream = std::cout)
    {
        // Save cursor position (in scroll region)
        std::cout << "\033[s";

        draw_telem ();

        // Restore cursor back into scroll region
        std::cout << "\033[u" << std::flush;

        if (log_stream.rdbuf () != std::cout.rdbuf ())
        {
            for (int i = 0; i < telem_rows; i++)
                log_stream << lines[i] () << "\n";
            log_stream.flush ();
        }
    }
};

}