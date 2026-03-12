/**
 * @file fileroot.hpp
 * @author Austin Jia
 * @brief Project file utility functions.
 * @namespace froot
 * 
 * @features
 *      - FileRoot class for handling files with a known project root directory
 *      - File to string and string to file functions
 * 
 * @details
 *      - If constructor fails, all methods will return an invalid std::expect
 *        or do nothing.
 * 
 * @todo
 *      - file append
 *      - file scanner
 *      - path joining operator
 *      - malformed path checks
 */

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <expected>

namespace froot
{

namespace details
{
    const std::string ExpNotInitialized = "FileRoot not initialized";
}
namespace d = details;

/**
 * Handles files for a project with a known root directory.
 */
class FileRoot
{

private: 
    std::string root;
    bool initialized = false;

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
     * Root directory constructor
     */
    FileRoot (const std::string& root)
        : root (root)
    {
        std::ifstream dir (root);

        if (std::filesystem::exists (root) &&
            std::filesystem::is_directory (root))
            initialized = true;
    }

    /**
     * Get absolute path from root-relative path
     * @warning output removes any trailing slashes for directories
     */
    std::expected<std::string, std::string>
    get_abs_from_rel (const std::string& extension) const
    {
        if (!initialized)
            return std::unexpected (d::ExpNotInitialized);

        if (extension.empty ())
            return root;

        return root + "/" + trim(extension);
    }

    /**
     * Read file contents into a string
     * Returns empty string on failure
     */
    std::expected<std::string, std::string>
    file_to_string (const std::string& path, bool abs_path = false) const
    {
        if (!initialized)
            return std::unexpected (d::ExpNotInitialized);

        if (path.size () < 1)
            return "";

        std::string file_path = abs_path ? path : *get_abs_from_rel (path);

        std::ifstream file (file_path);

        if (!file.is_open ())
            return std::unexpected ("Could not open file: " + file_path);


        return std::string {std::istreambuf_iterator<char> {file},
                            std::istreambuf_iterator<char> {}};
    }

    /**
     * Overwrites file with string
     */
    std::expected<void, std::string>
    string_to_file (const std::string& str, const std::string& path,
                    bool abs_path = false) const
    {
        if (!initialized)
            return std::unexpected (d::ExpNotInitialized);

        std::string file_path = abs_path ? path : *get_abs_from_rel (path);

        std::ofstream file (file_path);

        if (!file.is_open ())
            return std::unexpected ("Could not open file: " + file_path);

        file << str;

        return {};
    }
};

}