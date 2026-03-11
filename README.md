### Mini HPPs
A collection of small C++23 header libraries for personal use:

| Header                | Description                   |
|-----------------------|-------------------------------|
| `arg_parser.hpp`      | argument handler              |
| `benchmark.hpp`       | speed and memory benchmarker  |
| `dashboard.hpp`       | console telemetry             |
| `testbench.hpp`       | unit test framework           |
| `math_utils.hpp`      | math helpers                  |
| `file_utils.hpp`      | file helpers                  |
| `string_utils.hpp`    | string helpers                |
| `time_utils.hpp`      | time helpers                  |

### Design
**Intention:**
* Readability preferred over clever template metaprogramming
* Implementations kept small and practical

**Per-File:**
* Header-only libraries, no compilation or linking required
* Designed to be easily dropped into small projects
* Standard library only (no external dependencies)
* No dependencies between headers
* Each header defines a small dedicated namespace

**Throwing Exceptions:**
* Constructors may throw if objects cannot be created in a valid state
* Runtime operations return `std::expected` for recoverable errors
* Exceptions avoided in normal execution paths

**Behavior:**
* Functions never modify input parameters
* Methods may modify the object unless marked `const`
* Return values preferred over output parameters