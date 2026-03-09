/**
 * @file testbench.hpp
 * @author Austin Jia
 * @brief Simple testbench framework.
 */

#pragma once

#include <string>
#include <iostream>
#include <ostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>
#include <chrono>

namespace testbench
{

using ms_t      = int64_t;
using steady_clock = std::chrono::steady_clock;

/**
 * Get current time in milliseconds
 */
inline ms_t get_time_ms ()
{
    auto now = steady_clock::now ();
    return ms_t {std::chrono::duration_cast<std::chrono::nanoseconds>
                    (now.time_since_epoch ()).count () / 1000000};
}

/**
 * Tests status
 */
enum TestStatus
{
    NONE,
    STARTED,
    PASS,
    FAIL,
    TIMEOUT,
    ERROR
};

/**
 * Result struct
 */
struct Test
{
    std::string name;
    std::function<bool ()> func;
    TestStatus status;
    ms_t timeout = 0;  // per-test override (0 = use testbench default)

    Test (std::function<bool ()> func, const std::string& name,
          ms_t timeout = 0)
        : func (func), name (name), timeout (timeout) {};
};

/**
 * Overload Test stream insertion
 */
std::ostream& operator << (std::ostream& os, const Test& obj)
{
    const char* status_str;
    switch (obj.status)
    {
        case STARTED:
            status_str = "STARTED";
            break;
        case PASS:
            status_str = "\033[32mPASS";
            break;
        case FAIL:
            status_str = "\033[31mFAIL";
            break;
        case TIMEOUT:
            status_str = "\033[31mTIMEOUT";
            break;
        case ERROR:
            status_str = "\033[31mERROR";
            break;
        default:
            status_str = "NONE";
            break;

    }

    os << status_str << " --- " << obj.name << "\033[0m";

    return os;
}

/**
 * A named group of tests with optional dependencies on other families
 */
struct TestFamily
{
    std::string name;
    std::vector <Test> tests {};
    std::vector <std::string> depends_on {};

    bool evaluated = false;
    bool all_passed = true;
};

/**
 * Unified test harness
 */
class Testbench
{
private:
    std::vector <TestFamily> families {};
    bool dependency_cycle = true;
    ms_t default_timeout = 5000;  // default 5s per test

    /**
     * Find family index by name, or -1
     */
    int find_family (const std::string& name) const
    {
        for (int i = 0; i < families.size (); ++i)
            if (families[i].name == name)
                return i;
        return -1;
    }

    /**
     * Run tests for a single family
     */
    void run_family (TestFamily& family)
    {
        if (family.evaluated) return;

        for (auto& test : family.tests)
            test.status = TestStatus::NONE;

        family.all_passed = true;
        std::vector<TestFamily> failed_deps {};

        // Check dependencies
        for (const auto& dep : family.depends_on)
        {
            int idx = find_family (dep);
            if (idx < 0) continue;

            TestFamily& dep_family = families[idx];
            if (!dep_family.evaluated)
                run_family (dep_family);

            if (!dep_family.all_passed)
                failed_deps.push_back (dep_family);
        }

        if (!failed_deps.empty ())
        {
            std::cerr << "\033[33mWARN --- family \""
                        << family.name
                        << "\" depends on failed: ";
            for (size_t i = 0; i < failed_deps.size (); ++i)
            {
                if (i > 0) std::cerr << ", ";
                std::cerr << "\"" << failed_deps[i].name << "\"";
            }
            std::cerr << "\033[0m" << std::endl;
        }

        // Run tests sequentially with timeout check
        for (auto& test : family.tests)
        {
            test.status = TestStatus::STARTED;

            ms_t limit = test.timeout > 0
                       ? test.timeout
                       : default_timeout;

            bool result = false;
            ms_t start = get_time_ms ();

            try
            {
                result = test.func ();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Test " << test.name
                          << " Error: " << e.what () << std::endl;
                test.status = TestStatus::ERROR;
            }
            catch (...)
            {
                std::cerr << "Test " << test.name
                          << " threw unknown exception" << std::endl;
                test.status = TestStatus::ERROR;
            }

            ms_t elapsed = get_time_ms () - start;

            if (elapsed > limit)
            {
                test.status = TestStatus::TIMEOUT;
                family.all_passed = false;
            }
            else if (!result)
            {
                family.all_passed = false;
                if (test.status != TestStatus::ERROR)
                    test.status = TestStatus::FAIL;
            }
            else
            {
                test.status = TestStatus::PASS;
            }
        }

        family.evaluated = true;
    }

    /**
     * Validate dependency graph before running tests.
     * Warns about missing dependencies and circular dependencies.
     * Returns true if valid dependency chain
     */
    bool validate_dependencies ()
    {
        std::unordered_set <std::string> known;
        for (const auto& family : families)
            if (!family.name.empty ())
                known.insert (family.name);

        // Check for missing dependencies
        for (const auto& family : families)
            for (const auto& dep : family.depends_on)
                if (!known.count (dep))
                    std::cerr << "\033[33mWARN --- family \""
                              << family.name
                              << "\" depends on unknown family \""
                              << dep << "\"\033[0m" << std::endl;

        // Check for circular dependencies via DFS
        // Build adjacency: family -> its dependencies
        std::unordered_map <std::string, std::vector <std::string>> adj;
        for (const auto& family : families)
            if (!family.name.empty ())
                adj[family.name] = family.depends_on;

        // 0 = unvisited, 1 = in-stack, 2 = done
        std::unordered_map <std::string, int> state;
        std::vector <std::string> path;

        this->dependency_cycle = false;

        std::function <void (const std::string&)> check_cycle =
            [&] (const std::string& node)
        {
            state[node] = 1;
            path.push_back (node);

            for (const auto& dep : adj[node])
            {
                if (!known.count (dep)) continue;
                if (state[dep] == 1)
                {
                    // Found cycle — build the cycle string
                    std::string cycle;
                    auto it = std::find (path.begin (), path.end (), dep);
                    for (; it != path.end (); ++it)
                    {
                        if (!cycle.empty ()) cycle += " -> ";
                        cycle += "\"" + *it + "\"";
                    }
                    cycle += " -> \"" + dep + "\"";
                    std::cerr << "\033[31mERROR --- circular dependency: "
                              << cycle << "\033[0m" << std::endl;

                    dependency_cycle = true;
                }
                else if (state[dep] == 0)
                {
                    check_cycle (dep);
                }
            }

            path.pop_back ();
            state[node] = 2;
        };

        for (const auto& name : known)
            if (state[name] == 0)
                check_cycle (name);

        return !dependency_cycle;
    }

public:
    /**
     * Set default timeout in milliseconds for all tests
     */
    void set_timeout (int ms)
    {
        default_timeout = ms;
    }

    /**
     * Add a test to the default (unnamed) family
     */
    void add_test (Test test)
    {
        int idx = find_family ("");
        if (idx < 0)
        {
            families.push_back (TestFamily {.name = ""});
            idx = families.size () - 1;
        }
        families[idx].tests.push_back (test);
    }

    /**
     * Add a named test family with optional dependencies
     */
    void add_family (const std::string& name,
                     std::vector <Test> tests,
                     std::vector <std::string> depends_on = {})
    {
        families.push_back (TestFamily
        {
            .name       = name,
            .tests      = std::move (tests),
            .depends_on = std::move (depends_on),
        });
    }

    /**
     * Run all families in order, checking dependencies
     * Return true if run success
     * 
     * @note Intended to be called once.
     */
    bool run_tests ()
    {
        std::cout << "===== TEST OUTPUT =====" << std::endl;

        if (!validate_dependencies ())
            return false;

        for (auto& family : families)
            run_family (family);
        
        return true;
    }

    /**
     * Print results grouped by family
     */
    void print_results ()
    {
        std::cout << "\n======= RESULTS =======" << std::endl;
        if (this->dependency_cycle)
        {    
            std::cerr << "\033[31mERROR --- Dependencies not validated"
                      << "\033[0m" << std::endl;
            return;
        }

        for (size_t f = 0; f < families.size (); ++f)
        {
            const auto& family = families[f];

            if (!family.name.empty ())
            {
                if (f > 0)
                    std::cout << "\n";

                std::cout << "--- " << family.name << " ---"
                          << std::endl;
            }

            for (size_t i = 0; i < family.tests.size (); ++i)
            {
                std::cout << family.tests[i];
                if (i < family.tests.size () - 1)
                    std::cout << std::endl;
            }

            if (f < families.size () - 1)
                std::cout << std::endl;
        }
        
        std::cout << "\n=======================" << std::endl;
    }
};

}