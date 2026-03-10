/**
 * @file testbench_basic.cpp
 * @author Austin Jia
 * @brief Basic testbench usage.
 */

#include "testbench.hpp"

bool test_fam_one ()
{
    std::cout << "Hello from fam one!" << std::endl;
    return true;
}

bool test_fam_two ()
{
    return true;
}

bool test_depends ()
{
    return true;
}

int main ()
{
    test::Testbench tb;

    tb.add_family
    (
        "Family One", 
        {
            {test_fam_one, "Fam One"},
            {test_fam_two, "Fam Two"}
        }
    );

    tb.add_test ({test_depends, "Test Depends"});

    tb.run_tests ();
    tb.print_results ();

    return 0;
}