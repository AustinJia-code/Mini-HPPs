/**
 * @file time_utils.hpp
 * @author Austin Jia
 * @brief Chrono wrapper for basic types and clock utility functions.
 */

#pragma once
#include <chrono>

namespace tutils
{

using ns_t          = int64_t;
using us_t          = int64_t;
using ms_t          = int64_t;
using sec_t         = double;

using steady_clock  = std::chrono::steady_clock;

/*
 * ns_t to ms_t conversion
 */
inline ms_t ns_to_ms (ns_t ns)
{
    return (ms_t) (ns / 1000000);
}

/*
 * sec_t to us_t conversion
 */
inline us_t sec_to_us (sec_t ts)
{
    return (us_t) (ts * 1000000);
}

/**
 * Get current time in nanoseconds
 */
inline ns_t get_time_ns ()
{
    auto now = steady_clock::now ();
    return ns_t {std::chrono::duration_cast<std::chrono::nanoseconds> (
                    now.time_since_epoch ()).count ()};
}

/**
 * Get current time in milliseconds
 */
inline ms_t get_time_ms ()
{
    return ns_to_ms (get_time_ns ());
}

/**
 * Convert absolute milliseconds since epoch to timepoint
 */
inline std::chrono::time_point<steady_clock> ms_to_tp (ms_t ms)
{
    return std::chrono::time_point<std::chrono::steady_clock,
           std::chrono::milliseconds> (std::chrono::milliseconds (ms));
}

}