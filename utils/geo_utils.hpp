/**
 * @file geo_utils.hpp
 * @author Austin Jia
 * @brief Geometry utility functions.
 * @namespace gu
 * 
 * @features
 *      - Radian and degree type aliases and conversion functions
 *      - Distance type aliases
 */

#pragma once

#include <cmath>

namespace gutils
{

using deg_t     = double;
using rad_t     = double;
using dist_t    = double;   // contextual distance type, can be mm, cm, m, etc

struct vec3_t
{
    dist_t x, y, z;

    /**
     * Get the magnitude of the vector
     */
    dist_t mag () const
    {
        return std::sqrt (x * x + y * y + z * z);
    }
};

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