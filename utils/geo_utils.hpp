/**
 * @file geo_utils.hpp
 * @author Austin Jia
 * @brief Basic geometric utilities and types.
 * @namespace gutils
 */

#pragma once

#include <cmath>
#include <algorithm>

namespace gutils
{

using dist_t = double;   // contextual distance type, can be mm, cm, m, etc
using deg_t  = double;

struct vec3_t
{
    dist_t x, y, z;

    /**
     * Default constructor
     */
    vec3_t () : x (0), y (0), z (0) {}

    /**
     * Constructor with explicit values
     */
    vec3_t (dist_t x_, dist_t y_, dist_t z_) : x (x_), y (y_), z (z_) {}

    /**
     * Single value constructor
     */
    vec3_t (dist_t v) : x (v), y (v), z (v) {}

    /**
     * Add another vector to this vector in place
     */
    vec3_t& operator+= (const vec3_t& other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;

        return *this;
    }

    /**
     * Add another vector to this vector
     */
    vec3_t operator+ (const vec3_t& other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    /**
     * Subtract another vector from this vector
     */
    vec3_t operator- (const vec3_t& other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    /**
     * Scalar multiplication
     */
    vec3_t operator* (dist_t scalar) const
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    /**
     * Scalar division
     */
    vec3_t operator/ (dist_t scalar) const
    {
        return {x / scalar, y / scalar, z / scalar};
    }

    /**
     * Equality operator
     */
    bool operator== (const vec3_t& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

/**
 * Get the magnitude of a vector
 */
dist_t mag (const vec3_t& v)
{
    return std::sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * Get the norm of a vector
 */
vec3_t norm (const vec3_t& v)
{
    dist_t m = mag (v);
    return {v.x / m, v.y / m, v.z / m};
}

/**
 * Get the dot product of two vectors
 */
dist_t dot (const vec3_t& a, const vec3_t& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * Get the cross product with another vector
 */
vec3_t cross (const vec3_t& a, const vec3_t& b)
{
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

/**
 * Get the minimum of two vectors
 */
vec3_t min (const vec3_t& a, const vec3_t& b)
{
    return {std::min (a.x, b.x),
            std::min (a.y, b.y),
            std::min (a.z, b.z)};
}  

/**
 * Get the max of two vectors
 */
vec3_t max (const vec3_t& a, const vec3_t& b)
{
    return {std::max (a.x, b.x),
            std::max (a.y, b.y),
            std::max (a.z, b.z)};
}  

/**
 * Get the absolute value of a vector
 */
vec3_t abs (const vec3_t& v)
{
    return {std::abs (v.x), std::abs (v.y), std::abs (v.z)};
}

/**
 * Rotate a vector by axis-angle
 */
vec3_t rotate (const vec3_t& v, const vec3_t& axis, deg_t angle)
{
    vec3_t axis_norm = norm (axis);
    double cos_a = std::cos (angle * M_PI / 180.0);
    double sin_a = std::sin (angle * M_PI / 180.0);

    return v * cos_a
         + cross (axis_norm, v) * sin_a
         + axis_norm * dot (axis, v) * (1 - cos_a);
}

}