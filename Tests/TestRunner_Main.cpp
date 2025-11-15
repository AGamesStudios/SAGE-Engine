// Main Test Runner for SAGE Engine
// Запускает все тесты и выводит отчёт

#include "TestFramework.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "SAGE ENGINE - COMPREHENSIVE TEST SUITE\n";
    std::cout << "========================================\n\n";

    std::string filter;
    if (argc > 1) {
        filter = argv[1];
        std::cout << "Running tests matching filter: \"" << filter << "\"\n\n";
    }

    // Запускаем все тесты
    int failedTests = TestFramework::RunAllTests(filter);

    std::cout << "\n";

    if (failedTests == 0) {
        std::cout << "🎉 ALL TESTS PASSED! 🎉\n";
        std::cout << "SAGE Engine is stable and ready!\n";
        return 0;
    } else {
        std::cout << "❌ " << failedTests << " TEST(S) FAILED\n";
        std::cout << "Please fix the issues before deployment.\n";
        return 1;
    }
}
