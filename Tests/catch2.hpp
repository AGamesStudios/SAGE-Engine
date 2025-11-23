// Catch2 v3 single-header (simplified for SAGE)
// https://github.com/catchorg/Catch2
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

namespace Catch {
    struct TestCase {
        std::string name;
        std::string tags;
        void (*func)();
    };

    inline std::vector<TestCase>& GetTests() {
        static std::vector<TestCase> tests;
        return tests;
    }

    struct TestRegistrar {
        TestRegistrar(const char* name, const char* tags, void (*func)()) {
            GetTests().push_back({name, tags, func});
        }
    };

    struct AssertionFailed {
        std::string message;
        const char* file;
        int line;
    };

    inline void ReportFailure(const std::string& msg, const char* file, int line) {
        throw AssertionFailed{msg, file, line};
    }

    template<typename T>
    struct Approx {
        T value;
        T epsilon = static_cast<T>(1e-5);

        Approx(T v) : value(v) {}
        Approx& margin(T m) { epsilon = m; return *this; }

        bool operator==(T other) const {
            return std::abs(value - other) <= epsilon;
        }

        friend bool operator==(T lhs, const Approx& rhs) {
            return rhs == lhs;
        }
    };

    inline int RunAllTests(int argc, char* argv[]) {
        (void)argc;  // Unused in basic implementation
        (void)argv;
        
        int passed = 0;
        int failed = 0;

        std::cout << "\n===============================================================================\n";
        std::cout << "Running SAGE Engine Tests\n";
        std::cout << "===============================================================================\n\n";

        for (const auto& test : GetTests()) {
            try {
                std::cout << "Running: " << test.name << " " << test.tags << "\n";
                test.func();
                std::cout << "  PASSED\n";
                passed++;
            } catch (const AssertionFailed& e) {
                std::cout << "  FAILED: " << e.message << "\n";
                std::cout << "    at " << e.file << ":" << e.line << "\n";
                failed++;
            } catch (const std::exception& e) {
                std::cout << "  EXCEPTION: " << e.what() << "\n";
                failed++;
            }
        }

        std::cout << "\n===============================================================================\n";
        std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
        std::cout << "===============================================================================\n";

        return failed > 0 ? 1 : 0;
    }
}

#define SAGE_STRINGIFY(x) #x
#define SAGE_CONCAT_IMPL(a, b) a##b
#define SAGE_CONCAT(a, b) SAGE_CONCAT_IMPL(a, b)

#define TEST_CASE(testname, tags) \
    static void SAGE_CONCAT(TestFunc_, __LINE__)(); \
    namespace { \
        static Catch::TestRegistrar SAGE_CONCAT(TestReg_, __LINE__)(testname, tags, SAGE_CONCAT(TestFunc_, __LINE__)); \
    } \
    static void SAGE_CONCAT(TestFunc_, __LINE__)()

#define REQUIRE(expr) \
    do { \
        if (!(expr)) { \
            std::ostringstream oss; \
            oss << "REQUIRE( " #expr " )"; \
            Catch::ReportFailure(oss.str(), __FILE__, __LINE__); \
        } \
    } while(0)

#define CHECK(expr) REQUIRE(expr)

#define REQUIRE_FALSE(expr) REQUIRE(!(expr))

#define SECTION(name) if (true)
