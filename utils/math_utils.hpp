/**
 * @file math_utils.hpp
 * @author Austin Jia
 * @brief Math/geometry utility functions.
 * @namespace mutils
 * 
 * @features
 *      - Radian and degree type aliases and conversion functions
 *      - Distance type aliases
 */

#pragma once

#include <cmath>

namespace mutils
{

using deg_t = double;
using rad_t = double;
using mm_t  = double;

/**
 * Convert radians to degrees
 */
inline deg_t rad_to_deg (rad_t rad)
{
    return rad * 180.0 / M_PI;
}

/**
 * Convert degrees to radians
 */
inline rad_t deg_to_rad (deg_t deg)
{
    return deg * M_PI / 180.0;
}

}