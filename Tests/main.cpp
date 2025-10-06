#include "TestFramework.h"

// Объявление функции регистрации интеграционных тестов
namespace SAGE {
    void RegisterSystemIntegrationTests();
}

int main() {
    SAGE::RegisterSystemIntegrationTests();
    return TestFramework::RunAllTests();
}
