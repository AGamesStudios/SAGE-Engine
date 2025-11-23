# SAGE Engine Unit Tests

## Overview
This directory contains the unit test suite for the SAGE Engine, using a lightweight Catch2-style testing framework.

## Running Tests

### Via VS Code Tasks
- Press `Ctrl+Shift+P` → `Tasks: Run Task`
- Select "Run Unit Tests (Debug)" or "Run Unit Tests (Release)"

### Via Command Line
```powershell
# Build and run Debug tests
cmake --build build --config Debug --target SAGE_Tests
.\build\bin\Debug\SAGE_Tests.exe

# Build and run Release tests
cmake --build build --config Release --target SAGE_Tests
.\build\bin\Release\SAGE_Tests.exe
```

## Test Structure

### Test Framework (`catch2.hpp`)
Lightweight single-header framework with Catch2-compatible API:
- `TEST_CASE(name, tags)` - Define test case
- `SECTION(name)` - Group related assertions
- `REQUIRE(expr)` - Assert that must pass
- `CHECK(expr)` - Assertion that logs but continues
- `Catch::Approx(value).margin(epsilon)` - Floating-point comparison

### Test Suites

#### MathTests.cpp
Tests core math library functionality:
- Vector2: construction, arithmetic, normalization, dot product
- Matrix3: transformations, multiplication, inverse, projection
- Color: RGBA conversion, predefined colors
- Rect: contains, intersection, bounds

#### Camera2DTests.cpp
Tests camera coordinate transformations:
- ScreenToWorld conversions
- WorldToScreen conversions
- Round-trip transformation consistency
- Zoom, rotation, and viewport effects

#### ResourceManagerTests.cpp
Tests resource caching and lifecycle:
- Load/unload resources
- Weak pointer caching
- Cache hit behavior
- Cleanup of unused resources

#### PluginManagerTests.cpp
Tests plugin system behavior:
- Plugin loading/unloading
- Version compatibility
- Error handling for invalid plugins
- Singleton access pattern

## Writing New Tests

### Basic Test Example
```cpp
#include "catch2.hpp"
#include "SAGE/YourModule/YourClass.h"

using namespace SAGE;

TEST_CASE("Your feature description", "[category][tag]") {
    SECTION("Specific behavior") {
        YourClass obj;
        REQUIRE(obj.GetValue() == 42);
    }
    
    SECTION("Another behavior") {
        YourClass obj(100);
        CHECK(obj.GetValue() > 0);
        CHECK(obj.GetValue() <= 100);
    }
}
```

### Floating-Point Comparisons
```cpp
// Use Approx for floating-point values
float result = CalculateSomething();
REQUIRE(result == Catch::Approx(3.14159f).margin(0.0001f));

// For vectors/points
Vector2 pos = GetPosition();
REQUIRE(pos.x == Catch::Approx(100.0f).margin(1.0f));
REQUIRE(pos.y == Catch::Approx(200.0f).margin(1.0f));
```

### Adding New Test File
1. Create `YourTests.cpp` in `Tests/` directory
2. Include `catch2.hpp` and necessary headers
3. Write test cases using TEST_CASE macro
4. Add file to `Tests/CMakeLists.txt`:
   ```cmake
   set(TEST_SOURCES
       TestMain.cpp
       MathTests.cpp
       Camera2DTests.cpp
       ResourceManagerTests.cpp
       PluginManagerTests.cpp
       YourTests.cpp  # <-- Add here
   )
   ```
5. Rebuild with `cmake --build build --config Debug --target SAGE_Tests`

## Test Tags

Tests are organized with tags for selective execution (future enhancement):
- `[math]` - Math library tests
- `[vector2]` - Vector2-specific tests
- `[matrix3]` - Matrix3-specific tests
- `[camera]` - Camera system tests
- `[graphics]` - Graphics-related tests
- `[resource]` - Resource management tests
- `[core]` - Core engine systems
- `[plugin]` - Plugin system tests

## Current Test Coverage

```
✅ Vector2 operations - PASSED
✅ Matrix3 operations - PASSED
✅ Color operations - PASSED
✅ Rect operations - PASSED
✅ ResourceManager caching - PASSED
✅ Camera2D coordinate transformations - PASSED
✅ Camera2D properties - PASSED
✅ PluginManager lifecycle - PASSED
✅ PluginManager version compatibility - PASSED

Test Results: 9 passed, 0 failed (100%)
```

## Test Philosophy

1. **Fast:** Tests should run in <100ms total
2. **Isolated:** Each test should be independent
3. **Readable:** Test names describe behavior, not implementation
4. **Focused:** One concept per test case
5. **Comprehensive:** Cover normal cases, edge cases, and error conditions

## Future Enhancements

- [ ] Tag-based test filtering (`SAGE_Tests.exe [math]`)
- [ ] Test fixtures for complex setup/teardown
- [ ] Parameterized tests for data-driven testing
- [ ] Mocking framework for dependencies
- [ ] Code coverage reporting
- [ ] Performance benchmarks
- [ ] Integration tests for full engine initialization

## Troubleshooting

### Tests Won't Build
- Ensure you've run `cmake --build build --config Debug` at least once
- Check that `SAGE_Engine` library is built successfully
- Verify all test files are listed in `Tests/CMakeLists.txt`

### Tests Crash on Startup
- Check that all engine DLLs are in the correct output directory
- Verify resource paths are correct (tests run from workspace root)
- Look for missing dependencies (GLFW, GLAD, etc.)

### Floating-Point Test Failures
- Increase margin in `Approx().margin()` if precision errors occur
- Check for platform-specific floating-point behavior
- Consider using epsilon comparisons for very small values

## Contributing

When adding new engine features:
1. Write tests FIRST (test-driven development)
2. Ensure all existing tests still pass
3. Add tests for edge cases and error conditions
4. Update this README if adding new test categories

---

**Last Updated:** 2025-11-17  
**Test Count:** 9 test cases  
**Pass Rate:** 100%
