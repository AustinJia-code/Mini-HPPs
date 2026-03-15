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
 * @return an std::function that allows telemetry to track a variable
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
 * @return an std::function that allows telemetry to print a static string
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
    std::vector<std::function<std::string ()>> lines_;
    int telem_rows_;         // number of telemetry lines
    int scroll_start_;       // first row of scroll region (1-indexed)
    int scroll_end_;         // last row of scroll region (1-indexed)
    int total_rows_;

    /**
     * Get number of temrinal rows
     */
    int get_terminal_rows_ ()
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
        for (int i = 0; i < telem_rows_; i++)
        {
            // Save cursor, move to telem row, clear, write, restore
            std::cout << "\033[" << (i + 1) << ";1H"
                      << "\033[2K"
                      << lines_[i] ();
        }
    }

public:
    /**
     * Function list constructor
     * 
     * @param lines Vector of functions that return strings to print
     * 
     */
    Telemetry (std::vector<std::function<std::string ()>> lines)
        : lines_ (std::move (lines))
    {
        telem_rows_   = this->lines_.size ();
        total_rows_   = get_terminal_rows_ ();
        scroll_start_ = telem_rows_ + 1;
        scroll_end_   = total_rows_;

        // Clear screen
        std::cout << "\033[2J";

        // Set scroll region to below telemetry
        std::cout << "\033[" << scroll_start_ << ";" << scroll_end_ << "r";

        // Draw initial telemetry
        draw_telem ();

        // Park cursor at start of scroll region
        std::cout << "\033[" << scroll_start_ << ";1H" << std::flush;
    }

    /**
     * Destructor restores full scroll region and moves cursor to bottom
     */
    ~Telemetry ()
    {
        // Restore full scroll region on exit
        std::cout << "\033[1;" << total_rows_ << "r";
        std::cout << "\033[" << total_rows_ << ";1H\n" << std::flush;
    }

    /**
     * Refresh telemetry lines at top, keeps cursor in scroll region
     * 
     * @param log_stream Optional stream to also output telemetry lines to
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
            for (int i = 0; i < telem_rows_; i++)
                log_stream << lines_[i] () << "\n";
            log_stream.flush ();
        }
    }
};

}