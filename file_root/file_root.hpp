/**
 * @file file_root.hpp
 * @author Austin Jia
 * @brief Project file utility functions.
 * @namespace froot
 * 
 * @features
 *      - FileRoot class for handling files with a known project root
 *      - File to string and string to file functions
 * 
 * @todo
 *      - file append
 *      - file scanner
 */

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <iterator>

namespace froot
{

/**
 * Handles files for a project with a known root directory.
 */
class FileRoot
{
private: 
    std::string root;

    /**
     * Trim leading and trailing whitespace and slashes from a string
     */
    std::string trim (const std::string& s) const
    {
        std::string out = s;
        out.erase (out.find_last_not_of (" \n\r\t/") + 1);
        out.erase (0, out.find_first_not_of (" \n\r\t/"));

        return out;
    }

public:
    /**
     * Root constructor
     */
    FileRoot (const std::string& root)
        : root (root) {}

    /**
     * Get absolute path from root-relative path
     * @warning output removes any trailing slashes for directories
     */
    std::string get_abs_from_rel (const std::string& extension) const
    {
        if (extension.size () < 1)
            return root;

        return std::string {root} + "/" + trim (extension);
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