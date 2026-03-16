/**
 * @file arg_parser_basic.cpp
 * @author Austin Jia
 * @brief Basic usage of arg_parser.
 * 
 * @example ./arg_parser_basic im_flagless --name=Austin -c 5 --verbose --item=a --item=b
 */

#include "arg_parser.hpp"
#include <iostream>

int main (int argc, char* argv[])
{
    // Initialize parser with some defaults
    argp::arg_parser parser (argc, argv,
    {
        {"name", {"Evil Austin"}},
        {"count", {"1"}}
    });

    // Get flagless args (e.g., im_flagless)
    auto flagless_res = parser.get ("argless_");
    if (flagless_res)
    {
        std::cout << "Flagless args:";
        for (const auto& arg : *flagless_res)
        {
            std::cout << " " << arg;
        }
        std::cout << std::endl;
    }

    // Get a boolean flag (e.g., --verbose or -v)
    auto verbose = parser.get_bool ("verbose");
    if (verbose && *verbose)
    {
        std::cout << "Verbose mode enabled" << std::endl;
    }

    // Get a string value (e.g., --name=Alice or -n Alice)
    auto name_res = parser.get ("name");
    if (name_res)
    {
        std::cout << "Hello, " << name_res->front () << "!" << std::endl;
    }

    // Get a size_t value (e.g., --c=5 or -c 5)
    auto count_res = parser.get_size_t ("c");
    if (count_res)
    {
        std::cout << "Count is: " << *count_res << std::endl;
    }
    else
    {
        std::cerr << "Error parsing count: " << count_res.error () << std::endl;
    }

    // Get multiple values (e.g., --item=a --item=b)
    auto items_res = parser.get ("item");
    if (items_res)
    {
        std::cout << "Items:";
        for (const auto& item : *items_res)
        {
            std::cout << " " << item;
        }
        std::cout << std::endl;
    }

    return 0;
}
