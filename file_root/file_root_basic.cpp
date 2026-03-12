/**
 * @file file_root_basic.cpp
 * @author Austin Jia
 * @brief Basic usage of file_root.
 */

#include "file_root.hpp"
#include <iostream>

int main ()
{
    // Use the current directory as project root
    froot::FileRoot f_root (".");

    std::string test_file = "froot_test.txt";
    std::string content = "Hello, FileRoot!";

    // Write a string to a file (relative to root)
    std::cout << "Writing to " << test_file << "..." << std::endl;
    auto write_res = f_root.string_to_file (content, test_file);
    if (!write_res)
    {
        std::cerr << "Error writing to file: " << write_res.error () << std::endl;
        return 1;
    }

    // Resolve an absolute path
    auto abs_path = f_root.get_abs_from_rel (test_file);
    if (abs_path)
    {
        std::cout << "Absolute path: " << *abs_path << std::endl;
    }

    // Read the file content back
    std::cout << "Reading from " << test_file << "..." << std::endl;
    auto read_res = f_root.file_to_string (test_file);
    if (read_res)
    {
        std::cout << "Content read: " << *read_res << std::endl;
    }
    else
    {
        std::cerr << "Error reading file: " << read_res.error () << std::endl;
    }

    return 0;
}
