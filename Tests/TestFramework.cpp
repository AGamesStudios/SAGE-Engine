#include "TestFramework.h"

#include <exception>
#include <iostream>
#include <vector>

namespace TestFramework {

    struct TestCase {
        std::string Name;
        TestFunc Function;
    };

    static std::vector<TestCase>& GetTests() {
        static std::vector<TestCase> tests;
        return tests;
    }

    void Register(const std::string& name, TestFunc func) {
        GetTests().push_back(TestCase{ name, std::move(func) });
    }

    void TestContext::Check(bool condition, const char* expr, const char* file, int line) {
        if (condition) {
            return;
        }

        ++Failures;
        std::cerr << "    Assertion failed: " << expr << " (" << file << ":" << line << ")\n";
    }

    int RunAllTests() {
        auto& tests = GetTests();
        std::cout << "[SAGE Tests] Running " << tests.size() << " test(s)" << std::endl;

        int totalFailures = 0;
        int passed = 0;

        for (auto& test : tests) {
            TestContext ctx;
            try {
                test.Function(ctx);
            }
            catch (const std::exception& ex) {
                ctx.Check(false, ex.what(), __FILE__, __LINE__);
            }
            catch (...) {
                ctx.Check(false, "Unknown exception", __FILE__, __LINE__);
            }

            if (ctx.Failures == 0) {
                ++passed;
                std::cout << "  [PASS] " << test.Name << std::endl;
            }
            else {
                std::cout << "  [FAIL] " << test.Name << " (" << ctx.Failures << " failure(s))" << std::endl;
            }

            totalFailures += ctx.Failures;
        }

        std::cout << "[SAGE Tests] " << passed << "/" << tests.size() << " test(s) passed" << std::endl;
        return totalFailures == 0 ? 0 : 1;
    }

} // namespace TestFramework
