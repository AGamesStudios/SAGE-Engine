#include "SAGE.h"
#include "Core/GameObject.h"
#include "Scripting/LogCon/ScriptCompiler.h"
#include "Scripting/LogCon/Runtime/Interpreter.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace SAGE;
using namespace SAGE::Scripting::LogCon;

// Простое демо для тестирования LogCon скриптов
int main() {
#ifdef _WIN32
    // Включаем UTF-8 для консоли Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Включаем поддержку ANSI escape sequences для цветов
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, mode);
#endif

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LogCon Script Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed\n";
        return 1;
    }

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    std::cout << "=== LogCon Demo Started ===\n";
    std::cout << "Controls: WASD to move player\n";
    std::cout << "Press ESC to quit\n\n";

    // Компиляция LogCon скрипта
    ScriptCompiler compiler;
    const std::string scriptPath = "assets/scripts/player.ru.logcon";
    
    if (!compiler.CompileScript(scriptPath)) {
        std::cerr << "Failed to compile script: " << scriptPath << "\n";
        glfwTerminate();
        return 1;
    }

    auto scriptPtr = compiler.GetScriptShared();
    if (!scriptPtr) {
        std::cerr << "Script compilation succeeded but returned null\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "Script compiled successfully!\n";
    std::cout << "Entities found: " << scriptPtr->entities.size() << "\n";
    
    // Отладка: выводим AST первой сущности
    if (!scriptPtr->entities.empty()) {
        const auto& entity = scriptPtr->entities.front();
        std::cout << "\nEntity: " << entity.name << "\n";
        std::cout << "  Properties: " << entity.properties.size() << "\n";
        std::cout << "  Events: " << entity.events.size() << "\n";
        for (const auto& event : entity.events) {
            const char* typeName = "Unknown";
            switch (event.type) {
                case AST::EventBlock::Type::OnCreate: typeName = "OnCreate"; break;
                case AST::EventBlock::Type::OnUpdate: typeName = "OnUpdate"; break;
                case AST::EventBlock::Type::OnDestroy: typeName = "OnDestroy"; break;
                default: break;
            }
            std::cout << "    Event " << typeName << ": " << event.statements.size() << " statements\n";
        }
        std::cout << "  Functions: " << entity.functions.size() << "\n\n";
    }

    // Создание Runtime Interpreter
    Runtime::Interpreter interpreter;
    if (!interpreter.Instantiate(scriptPtr)) {
        std::cerr << "Failed to instantiate script runtime\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "Script runtime initialized!\n";
    
    // Поиск созданного игрока
    GameObject* player = GameObject::Find("Игрок");
    if (player) {
        std::cout << "Player entity created: '" << player->name << "'\n";
        std::cout << "Initial position: (" << player->x << ", " << player->y << ")\n";
        
        // Получение свойств из скрипта
        auto health = interpreter.GetProperty(player, "здоровье");
        auto speed = interpreter.GetProperty(player, "скорость");
        auto coins = interpreter.GetProperty(player, "монеты");
        
        if (health) std::cout << "Health: " << health->AsNumber() << "\n";
        if (speed) std::cout << "Speed: " << speed->AsNumber() << "\n";
        if (coins) std::cout << "Coins: " << coins->AsNumber() << "\n";
        std::cout << "\n";
    } else {
        std::cout << "Warning: Player entity 'Игрок' not found\n";
    }

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    double lastTime = glfwGetTime();
    double lastPrintTime = lastTime;
    int frameCount = 0;

    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Обработка событий
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Обновление всех GameObject (вызовет OnUpdate скриптов)
        GameObject::UpdateAll(deltaTime);

        // Отрисовка
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        frameCount++;

        // Вывод позиции игрока каждую секунду
        if (player && (currentTime - lastPrintTime) >= 1.0) {
            std::cout << "[" << frameCount << "] Player position: (" 
                      << player->x << ", " << player->y << ")";
            
            auto health = interpreter.GetProperty(player, "здоровье");
            if (health) {
                std::cout << " | Health: " << health->AsNumber();
            }
            
            std::cout << "\n";
            lastPrintTime = currentTime;
        }
    }

    std::cout << "\n=== LogCon Demo Finished ===\n";

    // Очистка
    interpreter.Clear();
    GameObject::DestroyAll();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
