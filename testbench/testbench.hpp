/**
 * @file testbench.hpp
 * @author Austin Jia
 * @brief Small testbench framework.
 * @namespace test
 * 
 * @features
 *      - Test and TestFamily structs for defining tests and grouping them
 *      - Testbench class for running tests with dependency handling and timeouts
 *      - Validation of dependency graph with cycle detection
 * 
 * @details
 *      - Tests are run sequentially within families, families are run in
 *        dependency order
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

namespace test
{
    
using ms_t = int64_t;

/**
 * Internal details, not part of the public API.
 */
namespace detail
{
    using steady_clock = std::chrono::steady_clock;

    /**
     * Get current time in milliseconds
     */
    static ms_t get_time_ms ()
    {
        auto now = d::steady_clock::now ();
        return ms_t {std::chrono::duration_cast<std::chrono::nanoseconds>
                        (now.time_since_epoch ()).count () / 1000000};
    }
}
namespace d = detail;

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
    TestStatus status = TestStatus::NONE;
    ms_t timeout = 0;  // per-test override (0 = use testbench default)

    /**
     * Test constructor
     * 
     * @param func The test function, should boolean result of test
     * @param name The name of the test, for reporting
     * @param timeout Optional timeout in milliseconds for this test
     *                (0 = use testbench default)
     */
    Test (std::function<bool ()> func, const std::string& name, ms_t timeout = 0)
        : func (func), name (name), timeout (timeout) {};
};

/**
 * Overload Test stream insertion
 * 
 * @param os The output stream
 * @param obj The Test object to print
 * @return The output stream
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
    std::vector <TestFamily> families_ {};
    bool dependency_cycle_ = true;
    ms_t default_timeout_ = 5000;  // default 5s per test

    /**
     * Find family index by name, or -1
     */
    int find_family (const std::string& name) const
    {
        for (int i = 0; i < families_.size (); ++i)
            if (families_[i].name == name)
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
            if (idx < 0)
                continue;

            TestFamily& dep_family = families_[idx];
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
                       : default_timeout_;

            bool result = false;
            ms_t start = d::get_time_ms ();

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

            ms_t elapsed = d::get_time_ms () - start;

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
        for (const auto& family : families_)
            if (!family.name.empty ())
                known.insert (family.name);

        // Check for missing dependencies
        for (const auto& family : families_)
            for (const auto& dep : family.depends_on)
                if (!known.count (dep))
                    std::cerr << "\033[33mWARN --- family \""
                              << family.name
                              << "\" depends on unknown family \""
                              << dep << "\"\033[0m" << std::endl;

        // Check for circular dependencies via DFS
        // Build adjacency: family -> its dependencies
        std::unordered_map <std::string, std::vector <std::string>> adj;
        for (const auto& family : families_)
            if (!family.name.empty ())
                adj[family.name] = family.depends_on;

        // 0 = unvisited, 1 = in-stack, 2 = done
        std::unordered_map <std::string, int> state;
        std::vector <std::string> path;

        this->dependency_cycle_ = false;

        std::function <void (const std::string&)> check_cycle =
            [&] (const std::string& node)
        {
            state[node] = 1;
            path.push_back (node);

            for (const auto& dep : adj[node])
            {
                if (!known.count (dep))
                    continue;
                    
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

                    dependency_cycle_ = true;
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

        return !dependency_cycle_;
    }

public:
    /**
     * Set default timeout in milliseconds for all tests
     * 
     * @param ms Timeout in milliseconds (0 = no timeout)
     */
    void set_timeout (int ms)
    {
        default_timeout_ = ms;
    }

    /**
     * Add a test to the default (unnamed) family
     * 
     * @param test The test to add
     */
    void add_test (Test test)
    {
        int idx = find_family ("");
        if (idx < 0)
        {
            families_.push_back (TestFamily {.name = ""});
            idx = families_.size () - 1;
        }
        families_[idx].tests.push_back (test);
    }

    /**
     * Add a named test family with optional dependencies
     * 
     * @param name The name of the family (for reporting and dependencies)
     * @param tests The tests in the family
     * @param depends_on Optional list of family names that this family depends on
     */
    void add_family (const std::string& name,
                     std::vector <Test> tests,
                     std::vector <std::string> depends_on = {})
    {
        families_.push_back (TestFamily
        {
            .name       = name,
            .tests      = std::move (tests),
            .depends_on = std::move (depends_on),
        });
    }

    /**
     * Run all families in order, checking dependencies.
     * Intended to be called once.
     * 
     * @return true if tests ran (dependency graph valid)
     */
    bool run_tests ()
    {
        std::cout << "===== TEST OUTPUT =====" << std::endl;

        if (!validate_dependencies ())
            return false;

        for (auto& family : families_)
            run_family (family);
        
        return true;
    }

    /**
     * Print results grouped by family
     */
    void print_results ()
    {
        std::cout << "\n======= RESULTS =======" << std::endl;
        if (this->dependency_cycle_)
        {    
            std::cerr << "\033[31mERROR --- Dependencies not validated"
                      << "\033[0m" << std::endl;
            return;
        }

        for (size_t f = 0; f < families_.size (); ++f)
        {
            const auto& family = families_[f];

            std::string name_print = family.name.empty ()
                                        ? "Ungrouped Tests" : family.name;
            if (f > 0)
                std::cout << "\n";

            std::cout << "--- " << name_print << " ---"
                    << std::endl;

            for (size_t i = 0; i < family.tests.size (); ++i)
            {
                std::cout << family.tests[i];
                if (i < family.tests.size () - 1)
                    std::cout << std::endl;
            }

            if (f < families_.size () - 1)
                std::cout << std::endl;
        }
        
        std::cout << "\n=======================" << std::endl;
    }
};

}