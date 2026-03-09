/**
 * @file file_utils.hpp
 * @author Austin Jia
 * @brief Project file utility functions.
 * 
 * @todo: add safety for trailing, leading /, quotes, etc.
 *        file append
 *        scanner
 *        file existence check
 */

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <iterator>

namespace futils
{

class RootedProject
{
private: 
    std::string root;

public:
    RootedProject (const std::string& root) : root (root) {}

    /**
     * Get absolute path from root-relative path
     */
    std::string get_abs_from_rel (const std::string& extension) const
    {
        return std::string {root} + extension;
    }

    /**
     * Read file contents into a string
     * Returns empty string on failure
     */
    inline std::string file_to_string (const std::string& path,
                                       bool abs_path = false) const
    {
        if (path.size () < 1)
            return "";

        std::string file_path = abs_path ? path : get_abs_from_rel (path);

        std::ifstream file (file_path);

        if (!file.is_open ())
        {
            std::cerr << "Error: could not open " << path << std::endl;
            return {};
        }

        return {std::istreambuf_iterator<char> {file},
                std::istreambuf_iterator<char> {}};
    }

    /**
     * Overwrites file with string
     */
    void string_to_file (const std::string& str, const std::string& path,
                        bool abs_path = false) const
    {
        std::string file_path = abs_path ? path : get_abs_from_rel (path);

        std::ofstream file (file_path);

        if (!file.is_open ())
        {
            std::cerr << "Error: could not open " << path << std::endl;
            return;
        }

        file << str;
    }

};

}