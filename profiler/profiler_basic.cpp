/**
 * @file profiler_basic.cpp
 * @author Austin Jia
 * @brief Basic usage of profiler.
 */

#include "profiler.hpp"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

// Busy-wait to simulate real work
void busy_wait_us (int us)
{
    auto end = std::chrono::steady_clock::now () + std::chrono::microseconds (us);
    while (std::chrono::steady_clock::now () < end);
}

int main ()
{
    prof::Profiler p;

    p.start ("render");
        p.start ("frustum_cull");
            busy_wait_us (150);
        p.stop ();
        p.start ("shadow_pass");
            busy_wait_us (1800);
        p.stop ();
        p.start ("geometry_pass");
            busy_wait_us (2400);
        p.stop ();
        p.start ("lighting_pass");
            busy_wait_us (900);
        p.stop ();
        p.start ("post_process");
            p.start ("bloom");
                busy_wait_us (600);
            p.stop ();
            p.start ("tonemap");
                busy_wait_us (200);
            p.stop ();
        p.stop ();
    p.stop ();

    p.save_html ("flame_graph.html");

    return 0;
}
