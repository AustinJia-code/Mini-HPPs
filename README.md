### Mini HPPs
A collection of small **C++23** header libraries for personal use:

| Header                | Description          | Namespace    |
|-----------------------|----------------------|--------------|
| `arg_parser.hpp`      | argument handler     | ```argp```   |
| `benchmark.hpp`       | function benchmarker | ```bench```  |
| `file_root.hpp`       | project file helpers | ```froot```  |
| `dashboard.hpp`       | console telemetry    | ```dash```   |
| `testbench.hpp`       | unit test framework  | ```test```   |
| `math_utils.hpp`      | math helpers         | ```mutils``` |
| `string_utils.hpp`    | string helpers       | ```sutils``` |
| `time_utils.hpp`      | time helpers         | ```tutils``` |

### Design
**Intention:**
* Readability preferred over convoluted template metaprogramming
* Implementations kept small and practical

**Per-File:**
* Header-only libraries, no compilation or linking required
* Designed to be easily dropped into small projects
* Standard library only (no external dependencies)
* Each header defines a small dedicated namespace
* Headers have no dependencies
    * causes many redundant aliases for common types such as
      ```ms_t```... working on a more elegant solution 

**Throwing Exceptions:**
* Constructors may throw if objects cannot be created in a valid state
* Runtime operations return `std::expected` for recoverable errors
* Exceptions otherwise avoided in normal execution paths

**Behavior:**
* Functions never modify input parameters
* Methods may modify the object unless marked `const`
* Return values preferred over output parameters
