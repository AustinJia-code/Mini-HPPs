/**
 * @file arg_parser.hpp
 * @author Austin Jia
 * @brief Small argument handler.
 * @namespace argp
 * 
 * @features
 *      - Flagless boolean args
 *      - Short flags (-f [opt_val]) and long flags (--flag[=opt_val])
 *      - Short flag groups (-abc == -a -b -c) 
 *      - Short flags with values for last in group (-abc v1 == -a -b -c v1)
 *      - Short flags with multiple values (-f v1 v2 == -f v1 -f v2)
 *      - Terminal flag '--' to stop flag parsing
 *      - Flags with single expected types via varied get functions
 * 
 * @details
 *      - Flagless args must be before any flagged args, and are treated
 *        as a flag with boolean "true"
 *      - Repeat flags will accumulate values into a vector
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <expected>
#include <stdexcept>

namespace argp
{

namespace details
{
    using arg_map_t = std::unordered_map<std::string, std::vector<std::string>>;

    const std::string ExpMissingFlag      = "Flag not found";
    const std::string ExpMissingValue     = "Flag has no value";
    const std::string ExpWrongValueCount  = "Flag has wrong number of values";
    const std::string ExpWrongValueType   = "Flag has wrong value type";
}
namespace d = details;

class arg_parser
{
private:
    d::arg_map_t args;

    /**
     * Helper for typed gets
     */
    std::expected<std::vector<std::string>, std::string> get_expect_single (
        const std::string& flag) const 
    {
        auto values = get (flag);

        if (!values)
            return std::unexpected (d::ExpMissingFlag);

        if (values->empty ())
            return std::unexpected (d::ExpMissingValue);

        if (values->size () != 1)
            return std::unexpected (d::ExpWrongValueType);

        return values;
    }

public:
    /*
     * Construct arg_parser from command line arguments
     * Expects arguments in the form --flag=value or --flag
     */
    arg_parser (int argc, char* argv[], d::arg_map_t defaults = {})
    {
        // First parse flagless boolean args until we hit a flag
        int i = 1;
        while (i < argc && argv[i][0] != '-')
        {
            args[argv[i]].push_back ("true");
            ++i;
        }

        // Flagged args
        for (; i < argc; ++i)
        {
            std::string arg = argv[i];
            // Terminal flag
            if (arg == "--")
                break;

            // Long flag    
            if (arg.rfind ("--", 0) == 0)
            {
                size_t eq_pos = arg.find ('=');
                if (eq_pos != std::string::npos)
                {
                    std::string flag = arg.substr (2, eq_pos - 2);
                    std::string value = arg.substr (eq_pos + 1);
                    args[flag].push_back (value);
                }
                // No value, treat as boolean
                else
                {
                    std::string flag = arg.substr (2);
                    args[flag].push_back ("true");
                }
            }
            // Short flag
            else if (arg.rfind ("-", 0) == 0)
            {
                // Non-final combined short flags are boolean
                for (size_t j = 1; j < arg.size() - 1; ++j)
                {
                    std::string flag (1, arg[j]);
                    args[flag].push_back ("true");
                }

                // Check last flag for args
                std::string flag (1, arg.back ());
                while (i + 1 < argc && argv[i + 1][0] != '-')
                    args[flag].push_back (argv[++i]);
               
                // If no args, treat as boolean
                if (args[flag].empty ())
                    args[flag].push_back ("true");
            }
        }

        // Set defaults if not in args
        for (const auto& [flag, value] : defaults)
            if (args.find (flag) == args.end ())
                args[flag] = value;
    }

    /**
     * Get values for a flag, or MissingFlag if not present.
     */
    std::expected<std::vector<std::string>, std::string> get (
        const std::string& flag) const
    {
        auto it = args.find (flag);
        if (it != args.end ())
            return it->second;

        return std::unexpected (d::ExpMissingFlag);
    }

    /**
     * Get single value for a flag, or error if not present or multiple values
     * If flag has no values, presence of flag is treated as boolean "true"
     * If flag is not present, treated as boolean "false"
     * {"true", "1", "false", "0"} are accepted boolean flags
     */  
    std::expected<bool, std::string> get_bool (const std::string& flag) const
    {
        auto values = get (flag);

        // If flag not present, treat as false
        if (!values)
            return false;

        // If flag present but no values, treat as true
        if (values->empty ())
            return std::unexpected (d::ExpMissingValue);

        // Too many values, error
        if (values->size () != 1)
            return std::unexpected (d::ExpWrongValueCount);

        // Parse boolean value
        const std::string& first = values->front ();
        if (first == "true" || first == "1")
            return true;
        else if (first == "false" || first == "0")
            return false;
        else
            return std::unexpected (d::ExpWrongValueType);
    }

    /**
     * Get single size_t for a flag via std::stoi, or error if not present or
     * multiple values
     */
    std::expected<std::size_t, std::string> get_size_t (const std::string& flag) const
    {
        auto values = get_expect_single (flag);
        
        if (!values)
            return std::unexpected (values.error ());

        try
        {
            return std::size_t {std::stoi (values->front ())};
        }
        catch (...)
        {
            return std::unexpected (d::ExpWrongValueType);
        }
    }
};

}