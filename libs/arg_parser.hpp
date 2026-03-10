/**
 * @file arg_parser.hpp
 * @author Austin Jia
 * @brief Small argument handler.
 * 
 * Supports:
 *      - flagless boolean args, must be before any flags
 *      - Short flags (-f [some_val]) and long flags (--flag[=some_val])
 *      - Combined short flags (-abc == -a -b -c) 
 *      - Short flags with multiple values (-f v1 v2 == -f=v1 -f=v2)
 *          - Short flags with values must be last in the combined group
 *      - Terminal flag "--" to stop flag parsing
 *      - All flags default to boolean false unless specified otherwise
 * 
 * - Repeat flags will accumulate values into a vector
 * 
 * @todo: test
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace argp
{

namespace details
{
    using arg_map_t = std::unordered_map<std::string, std::vector<std::string>>;
}
namespace d = details;

class arg_parser
{
private:
    d::arg_map_t args;

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
     * Get values for a flag, or empty vector if not present
     */
    std::optional<std::vector<std::string>> get (const std::string& flag) const
    {
        auto it = args.find (flag);
        if (it != args.end ())
            return it->second;

        return std::nullopt;
    }

    /**
     * Get true/false, determined by first value of flag if present
     * "true" and "1" are the only accepted true values
     */  
    bool get_bool (const std::string& flag) const
    {
        auto values = get (flag);

        if (values)
        {
            const std::string& first = values->front ();
            return first == "true" || first == "1";
        }

        return false;
    }
};

}