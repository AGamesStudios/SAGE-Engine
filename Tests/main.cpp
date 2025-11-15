#include "TestFramework.h"
#include "../Engine/Core/Logger.h"

#include <iostream>
#include <string_view>

// Объявление функции регистрации интеграционных тестов
namespace SAGE {
    void RegisterSystemIntegrationTests();
}

// Forward declare SoA test registration
void RegisterSpriteBatchSoATests();

int main(int argc, char** argv) {
    std::string filter;
    bool listTests = false;

    constexpr std::string_view kGTestFilterPrefix = "--gtest_filter=";
    constexpr std::string_view kFilterPrefix = "--filter=";

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--gtest_list_tests" || arg == "--list-tests") {
            listTests = true;
            continue;
        }

        if (arg.rfind(kGTestFilterPrefix, 0) == 0) {
            filter = std::string(arg.substr(kGTestFilterPrefix.size()));
            continue;
        }

        if (arg.rfind(kFilterPrefix, 0) == 0) {
            filter = std::string(arg.substr(kFilterPrefix.size()));
            continue;
        }
    }

    SAGE::Logger::Init();
    SAGE::RegisterSystemIntegrationTests();
    RegisterSpriteBatchSoATests();

    if (listTests) {
        TestFramework::ListTests(std::cout);
        return 0;
    }

    return TestFramework::RunAllTests(filter);
}
