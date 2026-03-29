/**
 * @file math_utils.hpp
 * @author Austin Jia
 * @brief Math/conversion utility functions.
 * @namespace mutils
 * 
 * @features
 *      - Radian and degree type aliases and conversion functions
 */

#pragma once

#include <cmath>

namespace mutils
{

using deg_t     = double;
using rad_t     = double;

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

/**
 * Round
 */
template <typename T>
inline T round (T value, int decimals)
{
    T factor = std::pow (static_cast<T> (10), decimals);
    return std::round (value * factor) / factor;
}

/**
 * Approx
 */
template <typename T>
inline bool approx (T a, T b, T eps = static_cast<T> (1e-5))
{
    return std::abs (a - b) < eps;
}

/**
 * Quantize a floating point number into discrete steps
 */
template <typename T>
inline int64_t quantize (T x, T eps = static_cast<T> (1e-4))
{
    return static_cast<int64_t> (std::round (x / eps));
}

/**
 * Linearly interpolate between two values
 */
template <typename T>
inline T lerp (T a, T b, T t)
{
    return a + (b - a) * t;
}

}