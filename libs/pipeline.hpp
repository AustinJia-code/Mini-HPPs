/**
 * @file pipeline.hpp
 * @author Austin Jia
 * @brief Function pipeline implementation.
 */

#pragma once

/*
Goal: 
auto r =
    pipe (vec)
    | filter ([](int x) { return x % 2; })
    | map ([](int x) { return x * 2; })
    | collect<std::vector> ();
*/