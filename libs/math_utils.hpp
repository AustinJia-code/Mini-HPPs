/**
 * @file math_utils.hpp
 * @author Austin Jia
 * @brief Math utility functions.
 */

#pragma once

#include <cmath>

namespace mutils
{

using deg_t = double;
using rad_t = double;

inline deg_t rad_to_deg (rad_t rad)
{
    return rad * 180.0 / M_PI;
}

inline rad_t deg_to_rad (deg_t deg)
{
    return deg * M_PI / 180.0;

}

}