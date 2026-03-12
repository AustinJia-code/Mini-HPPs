/**
 * @file benchmark_basic.cpp
 * @author Austin Jia
 * @brief Basic usage of benchmark.
 */

#include "benchmark.hpp"
#include <iostream>

int fib_recursive (int n)
{
    if (n <= 1)
        return n;
    
        return fib_recursive (n - 1) + fib_recursive (n - 2);
}

int fib_iterative (int n)
{
    if (n <= 1)
        return n;

    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i)
    {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

int main ()
{
    bench::Params params {warmup_iterations: 5,
                          iterations: 1000};

    auto results = bench::benchmark (params, []()
    {
        fib_recursive (30);
    });

    std::cout << "Recursive Fibonacci (n = 30)\n"
              << "   Average Time: " << results.average_time << " ms\n"
              << "   Min Time: " << results.min_time << " ms\n"
              << "   Max Time: " << results.max_time << " ms\n"
              << std::endl;

    results = bench::benchmark (params, []()
    {
        fib_iterative (30);
    });

    std::cout << "Iterative Fibonacci (n = 30)\n"
              << "   Average Time: " << results.average_time << " ms\n"
              << "   Min Time: " << results.min_time << " ms\n"
              << "   Max Time: " << results.max_time << " ms"
              << std::endl;

    return 0;
}