#include "TestFramework.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Core/ResourceManager.h"

namespace TestFramework {

namespace {

    bool GlobMatch(std::string_view pattern, std::string_view value) {
        std::size_t patternIndex = 0;
        std::size_t valueIndex = 0;
        std::size_t starIndex = std::string_view::npos;
        std::size_t matchIndex = 0;

        while (valueIndex < value.size()) {
            if (patternIndex < pattern.size()
                && (pattern[patternIndex] == '?' || pattern[patternIndex] == value[valueIndex])) {
                ++patternIndex;
                ++valueIndex;
                continue;
            }

            if (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
                starIndex = patternIndex++;
                matchIndex = valueIndex;
                continue;
            }

            if (starIndex != std::string_view::npos) {
                patternIndex = starIndex + 1;
                valueIndex = ++matchIndex;
                continue;
            }

            return false;
        }

        while (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
            ++patternIndex;
        }

        return patternIndex == pattern.size();
    }

    std::vector<std::string_view> SplitPatterns(std::string_view input) {
        std::vector<std::string_view> patterns;
        std::size_t start = 0;
        while (start <= input.size()) {
            const std::size_t end = input.find(':', start);
            const std::size_t length = (end == std::string_view::npos) ? input.size() - start : end - start;
            if (length > 0) {
                patterns.emplace_back(input.substr(start, length));
            }
            if (end == std::string_view::npos) {
                break;
            }
            start = end + 1;
        }
        return patterns;
    }

    bool MatchesFilterImpl(const std::string& name, const std::string& filter) {
        if (filter.empty() || filter == "*") {
            return true;
        }

        const std::string_view filterView(filter);
        const std::size_t dashPos = filterView.find('-');
        const std::string_view positiveView = filterView.substr(0, dashPos);
        const std::string_view negativeView = (dashPos == std::string_view::npos)
            ? std::string_view{}
            : filterView.substr(dashPos + 1);

        const auto positivePatterns = SplitPatterns(positiveView);
        bool include = positivePatterns.empty();
        for (const std::string_view pattern : positivePatterns) {
            if (GlobMatch(pattern, name)) {
                include = true;
                break;
            }
        }
        if (!include) {
            return false;
        }

        const auto negativePatterns = SplitPatterns(negativeView);
        for (const std::string_view pattern : negativePatterns) {
            if (GlobMatch(pattern, name)) {
                return false;
            }
        }

        return true;
    }

    std::vector<TestCase>& MutableTests() {
        static std::vector<TestCase> tests;
        return tests;
    }

} // namespace

    void Register(const std::string& name, TestFunc func) {
        MutableTests().push_back(TestCase{ name, std::move(func) });
    }

    void TestContext::Check(bool condition, const char* expr, const char* file, int line) {
        if (condition) {
            return;
        }

        Fail(expr, file, line);
    }

    void TestContext::Fail(const char* message, const char* file, int line) {
        ++Failures;

        FailureRecord record;
        if (message != nullptr && message[0] != '\0') {
            record.Expression = message;
        }
        else {
            record.Expression = "<no expression>";
        }

        if (file != nullptr && file[0] != '\0') {
            record.File = file;
        }
        else {
            record.File = "<unknown>";
        }

        record.Line = line;
        FailureDetails.push_back(record);

        if (m_ImmediateFailureOutput) {
            std::cerr << "    Assertion failed: " << record.Expression << " (" << record.File << ":" << record.Line << ")\n";
        }
    }

    void TestContext::SetImmediateFailureOutput(bool enabled) {
        m_ImmediateFailureOutput = enabled;
    }

    const std::vector<TestCase>& GetTests() {
        return MutableTests();
    }

    void ListTests(std::ostream& stream) {
        const auto& tests = MutableTests();
        for (const auto& test : tests) {
            stream << test.Name << '\n';
        }
    }

    bool MatchesFilter(const std::string& name, const std::string& filter) {
        return MatchesFilterImpl(name, filter);
    }

    int RunAllTests(const std::string& filter) {
        SAGE::ResourceManager::Get().SetGpuLoadingEnabled(false);

        auto& allTests = MutableTests();
        std::vector<TestCase*> selected;
        selected.reserve(allTests.size());
        for (auto& test : allTests) {
            if (MatchesFilterImpl(test.Name, filter)) {
                selected.push_back(&test);
            }
        }

        std::cout << "[SAGE Tests] Running " << selected.size() << " test(s)";
        if (!filter.empty()) {
            std::cout << " (filter=\"" << filter << "\")";
        }
        std::cout << std::endl;

        int totalFailures = 0;
        int passed = 0;

        for (auto* test : selected) {
            TestContext ctx;
            ctx.SetImmediateFailureOutput(true);
            ctx.Failures = 0;
            ctx.FailureDetails.clear();
            try {
                test->Function(ctx);
            }
            catch (const std::exception& ex) {
                const std::string message = std::string("Unhandled exception: ") + ex.what();
                ctx.Fail(message.c_str(), test->Name.c_str(), 0);
            }
            catch (...) {
                ctx.Fail("Unhandled unknown exception", test->Name.c_str(), 0);
            }

            if (ctx.Failures == 0) {
                ++passed;
                std::cout << "  [PASS] " << test->Name << std::endl;
            }
            else {
                std::cout << "  [FAIL] " << test->Name << " (" << ctx.Failures << " failure(s))" << std::endl;
            }

            totalFailures += ctx.Failures;
        }

        std::cout << "[SAGE Tests] " << passed << "/" << selected.size() << " test(s) passed" << std::endl;
        return totalFailures == 0 ? 0 : 1;
    }

} // namespace TestFramework
