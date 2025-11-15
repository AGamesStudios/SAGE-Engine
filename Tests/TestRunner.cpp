#include "TestFramework.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

    struct CommandLineOptions {
        bool showHelp = false;
        bool listTests = false;
        bool verbose = false;
        bool useColor = true;
        std::string filter;
    };

    struct Colors {
        const char* reset = "";
        const char* green = "";
        const char* red = "";
        const char* yellow = "";
        const char* cyan = "";
        const char* bold = "";
    };

    void EnableVirtualTerminalProcessing() {
    #ifdef _WIN32
        HANDLE stdoutHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            return;
        }

        DWORD mode = 0;
        if (!::GetConsoleMode(stdoutHandle, &mode)) {
            return;
        }

        if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
            ::SetConsoleMode(stdoutHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    #endif
    }

    Colors MakeColors(bool enabled) {
        if (!enabled) {
            return Colors{};
        }

        return Colors{
            "\x1b[0m",
            "\x1b[32m",
            "\x1b[31m",
            "\x1b[33m",
            "\x1b[36m",
            "\x1b[1m"
        };
    }

    void PrintUsage(const char* exeName) {
        std::cout << "SAGE Engine Test Runner\n\n"
                  << "Usage:\n"
                  << "  " << exeName << " [options]\n\n"
                  << "Options:\n"
                  << "  --help                 Show this help message\n"
                  << "  --list-tests           List registered tests (respects --filter)\n"
                  << "  --filter=PATTERN       Run only tests matching the glob pattern\n"
                  << "  --verbose              Enable verbose output\n"
                  << "  --no-color             Disable ANSI colored output\n"
                  << "  --color                Force enable ANSI colored output\n";
    }

    CommandLineOptions ParseCommandLine(int argc, char** argv) {
        CommandLineOptions options;

        for (int i = 1; i < argc; ++i) {
            const std::string_view arg(argv[i]);

            if (arg == "--help" || arg == "-h") {
                options.showHelp = true;
                continue;
            }

            if (arg == "--list-tests") {
                options.listTests = true;
                continue;
            }

            if (arg == "--verbose" || arg == "-v") {
                options.verbose = true;
                continue;
            }

            if (arg == "--no-color") {
                options.useColor = false;
                continue;
            }

            if (arg == "--color") {
                options.useColor = true;
                continue;
            }

            constexpr std::string_view kFilterEq = "--filter=";
            if (arg.substr(0, kFilterEq.size()) == kFilterEq) {
                options.filter.assign(arg.substr(kFilterEq.size()));
                continue;
            }

            if (arg == "--filter" && i + 1 < argc) {
                options.filter.assign(argv[++i]);
                continue;
            }

            std::cerr << "Unknown argument: " << arg << "\n";
            options.showHelp = true;
            return options;
        }

        return options;
    }

    std::string ExtractModule(const std::string& testName) {
        const std::size_t underscore = testName.find('_');
        const std::size_t dot = testName.find('.');
        const std::size_t separator = (underscore != std::string::npos) ? underscore : dot;
        if (separator == std::string::npos) {
            return "General";
        }
        return testName.substr(0, separator);
    }

    std::string FormatDuration(double valueMs) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(3) << valueMs;
        return stream.str();
    }

    struct TestResult {
        const TestFramework::TestCase* test = nullptr;
        double durationMs = 0.0;
        bool passed = false;
        std::vector<TestFramework::FailureRecord> failures;
    };

} // namespace

int main(int argc, char** argv) {
    const CommandLineOptions options = ParseCommandLine(argc, argv);

    if (options.showHelp) {
        PrintUsage(argc > 0 ? argv[0] : "SAGETests");
        return 0;
    }

    if (options.useColor) {
        EnableVirtualTerminalProcessing();
    }
    const Colors colors = MakeColors(options.useColor);

    bool glAvailable = false;
    // Minimal GL harness: attempt to initialize GLFW + create hidden context
    if (glfwInit()) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(64, 64, "TestGL", nullptr, nullptr);
        if (window) {
            glfwMakeContextCurrent(window);
            if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                glAvailable = true;
            } else {
                glfwDestroyWindow(window);
                glfwTerminate();
            }
        } else {
            glfwTerminate();
        }
    }

    if (!glAvailable) {
        std::cout << "[INFO] GL context unavailable; GL-dependent tests will be skipped" << std::endl;
    }

    const auto& registeredTests = TestFramework::GetTests();
    std::vector<const TestFramework::TestCase*> selected;
    selected.reserve(registeredTests.size());
    for (const auto& test : registeredTests) {
        if (TestFramework::MatchesFilter(test.Name, options.filter)) {
            selected.push_back(&test);
        }
    }

    if (selected.empty()) {
        std::cout << "No tests matched the given criteria." << std::endl;
        return 0;
    }

    if (options.listTests) {
        std::map<std::string, std::vector<std::string>> grouped;
        for (const auto* test : selected) {
            grouped[ExtractModule(test->Name)].push_back(test->Name);
        }

        for (auto& [module, tests] : grouped) {
            std::cout << module << ":\n";
            std::sort(tests.begin(), tests.end());
            for (const auto& name : tests) {
                std::cout << "  - " << name << '\n';
            }
        }

        return 0;
    }

    std::cout << colors.bold << "[==========] " << colors.reset
              << "Running " << selected.size() << " test(s)";
    if (!options.filter.empty()) {
        std::cout << " (filter=\"" << options.filter << "\")";
    }
    std::cout << std::endl;

    struct GroupSummary {
        int total = 0;
        int passed = 0;
        int failed = 0;
        double durationMs = 0.0;
    };

    std::map<std::string, GroupSummary> moduleSummaries;
    std::vector<TestResult> results;
    results.reserve(selected.size());

    auto suiteStart = std::chrono::steady_clock::now();

    for (const auto* test : selected) {
        if (!glAvailable && test->Name == "TextureResourceManager_LoadTextureViaRM") {
            std::cout << colors.yellow << "[ SKIP     ]" << colors.reset << ' ' << test->Name
                      << " (OpenGL not initialized)" << std::endl;
            continue;
        }
        const std::string module = ExtractModule(test->Name);
        GroupSummary& summary = moduleSummaries[module];
        ++summary.total;

    std::cout << colors.cyan << "[ RUN      ]" << colors.reset << ' ' << test->Name << std::endl;

        TestFramework::TestContext context;
        context.SetImmediateFailureOutput(false);
        context.Failures = 0;
        context.FailureDetails.clear();

        const auto start = std::chrono::steady_clock::now();
        try {
            test->Function(context);
        }
        catch (const std::exception& ex) {
            const std::string message = std::string("Unhandled exception: ") + ex.what();
            context.Fail(message.c_str(), test->Name.c_str(), 0);
        }
        catch (...) {
            context.Fail("Unhandled unknown exception", test->Name.c_str(), 0);
        }
        const auto end = std::chrono::steady_clock::now();

        TestResult result;
        result.test = test;
        result.durationMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
        result.passed = (context.Failures == 0);
        result.failures = context.FailureDetails;
        results.push_back(result);

        summary.durationMs += result.durationMs;
        if (result.passed) {
            ++summary.passed;
            std::cout << colors.green << "[       OK ]" << colors.reset << ' ' << test->Name
                      << " (" << FormatDuration(result.durationMs) << " ms)" << std::endl;
            if (options.verbose) {
                std::cout << "           " << "Module: " << module << std::endl;
            }
        }
        else {
            ++summary.failed;
            std::cout << colors.red << "[  FAILED  ]" << colors.reset << ' ' << test->Name
                      << " (" << FormatDuration(result.durationMs) << " ms)" << std::endl;

            for (const auto& failure : result.failures) {
                std::cout << "           " << colors.red << "Failure" << colors.reset << ": " << failure.Expression;
                if (!failure.File.empty()) {
                    std::cout << " (" << failure.File;
                    if (failure.Line > 0) {
                        std::cout << ':' << failure.Line;
                    }
                    std::cout << ')';
                }
                std::cout << std::endl;
            }
        }
    }

    const auto suiteEnd = std::chrono::steady_clock::now();
    const double totalDuration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(suiteEnd - suiteStart).count();

    int passedCount = 0;
    int failedCount = 0;
    for (const auto& result : results) {
        if (result.passed) {
            ++passedCount;
        }
        else {
            ++failedCount;
        }
    }

    std::cout << colors.bold << "[==========] " << colors.reset
              << "Finished running " << selected.size() << " test(s) in "
              << FormatDuration(totalDuration) << " ms" << std::endl;

    if (passedCount > 0) {
        std::cout << colors.green << "[  PASSED  ]" << colors.reset
                  << ' ' << passedCount << " test(s)" << std::endl;
    }
    if (failedCount > 0) {
        std::cout << colors.red << "[  FAILED  ]" << colors.reset
                  << ' ' << failedCount << " test(s)" << std::endl;
    }

    std::cout << "\nModule breakdown:" << std::endl;
    for (const auto& [module, summary] : moduleSummaries) {
    std::cout << "  " << module << ": " << summary.passed << '/' << summary.total
          << " passed (" << FormatDuration(summary.durationMs) << " ms)" << std::endl;
    }

    return failedCount == 0 ? 0 : 1;
}
