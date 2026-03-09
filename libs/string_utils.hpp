/**
 * @file string_utils.hpp
 * @author Austin Jia
 * @brief String utility functions.
 */

#pragma once

#include <string>

namespace sutils
{

std::string trim (const std::string& s)
{
    std::string out = s;
    out.erase (out.find_last_not_of (" \n\r\t") + 1);
    out.erase (0, out.find_first_not_of (" \n\r\t"));

    return out;
}

}