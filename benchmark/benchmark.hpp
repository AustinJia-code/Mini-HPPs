/**
 * @file benchmark.hpp
 * @author Austin Jia
 * @brief Small benchmarking framework.
 * @namespace bench
 * 
 * @features
 *      - Warm up iterations to mitigate cold start effects
 *      - Average, min, and max time reporting
 * 
 * @details
 *      - State reset left to user via optional setup and teardown functions
 *      - Measures in milliseconds, may be inaccurate for very fast functions
 * 
 * @todo
 *      - More accurate timing for fast functions (CPU cycle counters/ns)
 *      - Memory reporting
 */

#pragma once

#include <functional>
#include <cstddef>
#include <chrono>

namespace bench
{

using ms_t = int64_t;

/**
 * Internal details, not part of the public API.
 */
namespace detail
{
    using ns_t = int64_t;
    using steady_clock = std::chrono::steady_clock;

    inline ms_t ns_to_ms (ns_t ns)
    {
        return (ms_t) (ns / 1000000);
    }

    inline ns_t get_time_ns ()
    {
        auto now = steady_clock::now ();
        return ns_t {std::chrono::duration_cast<std::chrono::nanoseconds>
                     (now.time_since_epoch ()).count ()};
    }

    inline ms_t get_time_ms ()
    {
        return ns_to_ms (get_time_ns ());
    }
}
namespace d = detail;

/**
 * Benchmark Hyperparameters
 */
struct Params
{
    size_t warmup_iterations;
    size_t iterations;
};

/**
 * Benchmark Results
 */
struct Results
{
    ms_t average_time;
    ms_t min_time;
    ms_t max_time;
};

/**
 * Get benchmark results for a function with optional setup and teardown.
 * 
 * @param params Benchmark parameters
 * @param func The function to benchmark
 * @param setup Optional setup function to run before each iteration
 * @param teardown Optional teardown function to run after each iteration
 * @return Benchmark results
 */
Results benchmark (const Params& params,
                   const std::function<void()>& func,
                   const std::function<void()>& setup = [](){},
                   const std::function<void()>& teardown = [](){})
{
    // warmup
    for (size_t i = 0; i < params.warmup_iterations; ++i)
    {
        setup ();
        func ();
        teardown ();
    }

    Results results {};

    // actual
    for (size_t i = 0; i < params.iterations; ++i)
    {
        setup ();

        ms_t start = d::get_time_ms ();
        func ();
        ms_t end = d::get_time_ms ();

        ms_t elapsed = end - start;
        results.average_time += elapsed;
        results.min_time = (i == 0) ?
            elapsed : std::min (results.min_time, elapsed);
        results.max_time = (i == 0) ?
            elapsed : std::max (results.max_time, elapsed);

        teardown ();
    }

    results.average_time /= params.iterations;
    return results;
};

}