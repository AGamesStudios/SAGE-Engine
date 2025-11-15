#include "QuickStartGuide.h"
#include "Core/Logger.h"
#include <fstream>

namespace SAGE {

bool QuickStartGuide::CreateFirstProject(const QuickStartOptions& options) {
    SAGE_INFO("=== 🚀 SAGE Engine Quick Start Guide ===");
    SAGE_INFO("Создаём ваш первый проект шаг за шагом...\n");
    
    // Шаг 1: Создаём проект
    LogStep("1. Создание проекта");
    
    ProjectInfo info;
    info.name = options.projectName;
    info.path = options.projectPath.empty() 
        ? ("./Projects/" + options.projectName) 
        : options.projectPath;
    info.type = options.useLogConOnly ? ProjectType::LogConOnly : ProjectType::CppWithLogCon;
    info.templateType = options.templateType;
    info.windowTitle = options.projectName;
    info.author = "Beginner Developer";
    
    if (!ProjectManager::Instance().CreateProject(info)) {
        LogStep("1. Создание проекта", false);
        return false;
    }
    
    // Шаг 2: Создаём стартовую сцену
    LogStep("2. Настройка стартовой сцены");
    
    // Сцена уже создана в ProjectManager::CreateStarterScene
    
    // Шаг 3: Создаём пример скрипта
    LogStep("3. Создание примера кода");
    
    std::string scriptPath = info.path + "/Scripts/player_controller.logcon";
    if (!CreateExampleScript(scriptPath, options.templateType)) {
        LogStep("3. Создание примера кода", false);
        return false;
    }
    
    // Шаг 4: Финал
    SAGE_INFO("\n=== ✅ Проект создан успешно! ===\n");
    SAGE_INFO("📂 Путь: {}", info.path);
    SAGE_INFO("📝 Главная сцена: Scenes/MainScene.scene");
    SAGE_INFO("🎮 Скрипт: Scripts/player_controller.logcon");
    SAGE_INFO("\n📚 Следующие шаги:");
    SAGE_INFO("   1. Откройте проект в редакторе SAGE");
    SAGE_INFO("   2. Нажмите Play (▶) чтобы запустить");
    SAGE_INFO("   3. Редактируйте Scripts/player_controller.logcon");
    SAGE_INFO("   4. Добавляйте объекты в сцену!\n");
    
    ShowBeginnerTutorial();
    
    return true;
}

bool QuickStartGuide::SetupDemoScene(ECS::ECSContext& ecsContext, ProjectTemplate templateType) {
    auto& registry = ecsContext.GetRegistry();
    
    SAGE_INFO("Настройка демо-сцены...");
    
    // ОБЯЗАТЕЛЬНО: Камера
    GameObjectTemplates::CreateCamera(registry, Vector2::Zero(), 1280, 720, true);
    
    switch (templateType) {
        case ProjectTemplate::Platformer2D: {
            // Игрок
            GameObjectTemplates::CreatePlayer(registry, Vector2(0, 50));
            
            // Земля
            GameObjectTemplates::CreatePlatform(registry, Vector2(0, -200), Vector2(800, 50));
            
            // Несколько платформ
            GameObjectTemplates::CreatePlatform(registry, Vector2(-300, -100), Vector2(200, 30));
            GameObjectTemplates::CreatePlatform(registry, Vector2(300, -100), Vector2(200, 30));
            GameObjectTemplates::CreatePlatform(registry, Vector2(0, 0), Vector2(150, 30));
            
            // Враг
            GameObjectTemplates::CreateEnemy(registry, Vector2(200, 50));
            
            // Монетки
            GameObjectTemplates::CreateCollectible(registry, Vector2(-100, 50));
            GameObjectTemplates::CreateCollectible(registry, Vector2(100, 100));
            
            SAGE_INFO("✓ Platformer demo scene setup complete");
            break;
        }
        
        case ProjectTemplate::TopDown2D: {
            // Игрок
            GameObjectTemplates::CreatePlayer(registry, Vector2::Zero());
            
            // Враги в разных позициях
            GameObjectTemplates::CreateEnemy(registry, Vector2(100, 100));
            GameObjectTemplates::CreateEnemy(registry, Vector2(-100, -100));
            GameObjectTemplates::CreateEnemy(registry, Vector2(100, -100));
            
            // Монетки
            for (int i = 0; i < 5; i++) {
                float angle = (i / 5.0f) * 2.0f * 3.14159f;
                Vector2 pos(std::cos(angle) * 150, std::sin(angle) * 150);
                GameObjectTemplates::CreateCollectible(registry, pos);
            }
            
            SAGE_INFO("✓ Top-down demo scene setup complete");
            break;
        }
        
        case ProjectTemplate::Empty:
        default:
            // Только камера - уже создана
            SAGE_INFO("✓ Empty scene setup complete");
            break;
    }
    
    return true;
}

void QuickStartGuide::ShowBeginnerTutorial() {
    SAGE_INFO("=== 📖 Туториал для начинающих ===\n");
    
    SAGE_INFO("🎮 Основы SAGE Engine:");
    SAGE_INFO("   • Сцены (Scenes) - уровни вашей игры");
    SAGE_INFO("   • Объекты (GameObjects) - персонажи, платформы, враги");
    SAGE_INFO("   • Компоненты - Transform (позиция), Sprite (картинка), Physics");
    SAGE_INFO("   • Скрипты - ваш код на LogCon или C++\n");
    
    SAGE_INFO("🖱️ Управление в редакторе:");
    SAGE_INFO("   • ▶ Play - запустить игру");
    SAGE_INFO("   • ⏸ Pause - пауза");
    SAGE_INFO("   • ⏹ Stop - остановить");
    SAGE_INFO("   • Перетаскивайте объекты мышью");
    SAGE_INFO("   • F2 - переименовать объект\n");
    
    SAGE_INFO("📝 LogCon - простой язык для игр:");
    SAGE_INFO("```logcon");
    SAGE_INFO("function Update(deltaTime) {");
    SAGE_INFO("    // Управление игроком");
    SAGE_INFO("    if (Input.IsKeyDown(\"A\")) {");
    SAGE_INFO("        player.MoveLeft();");
    SAGE_INFO("    }");
    SAGE_INFO("}");
    SAGE_INFO("```\n");
    
    SAGE_INFO("🔑 Горячие клавиши:");
    SAGE_INFO("   • Ctrl+S - сохранить сцену");
    SAGE_INFO("   • Ctrl+N - новая сцена");
    SAGE_INFO("   • Ctrl+O - открыть сцену");
    SAGE_INFO("   • Delete - удалить объект\n");
    
    SAGE_INFO("📚 Полезные ссылки:");
    SAGE_INFO("   • Документация: https://sage-engine.dev/docs");
    SAGE_INFO("   • Туториалы: https://sage-engine.dev/tutorials");
    SAGE_INFO("   • Примеры: https://sage-engine.dev/examples");
    SAGE_INFO("   • Discord: https://discord.gg/sage-engine\n");
}

bool QuickStartGuide::CreateExampleScript(const std::string& scriptPath, ProjectTemplate templateType) {
    std::ofstream script(scriptPath);
    if (!script.is_open()) {
        SAGE_ERROR("Failed to create example script: {}", scriptPath);
        return false;
    }
    
    script << "// 🎮 Player Controller\n";
    script << "// Этот скрипт управляет игроком\n\n";
    
    if (templateType == ProjectTemplate::Platformer2D) {
        script << "// === ПЛАТФОРМЕР ===\n\n";
        
        script << "// Вызывается при старте игры\n";
        script << "function Start() {\n";
        script << "    Log(\"Player spawned!\");\n";
        script << "}\n\n";
        
        script << "// Вызывается каждый кадр\n";
        script << "function Update(deltaTime) {\n";
        script << "    // Получаем игрока\n";
        script << "    var player = FindEntity(\"Player\");\n";
        script << "    if (player == null) return;\n\n";
        
        script << "    // Движение влево/вправо\n";
        script << "    if (Input.IsKeyDown(\"A\") || Input.IsKeyDown(\"Left\")) {\n";
        script << "        player.MoveLeft();\n";
        script << "    }\n";
        script << "    if (Input.IsKeyDown(\"D\") || Input.IsKeyDown(\"Right\")) {\n";
        script << "        player.MoveRight();\n";
        script << "    }\n\n";
        
        script << "    // Прыжок\n";
        script << "    if (Input.IsKeyPressed(\"Space\")) {\n";
        script << "        if (player.IsGrounded()) {\n";
        script << "            player.Jump();\n";
        script << "            Log(\"Jump!\");\n";
        script << "        }\n";
        script << "    }\n";
        script << "}\n\n";
        
        script << "// Когда игрок сталкивается с чем-то\n";
        script << "function OnCollision(player, other) {\n";
        script << "    if (other.HasTag(\"Enemy\")) {\n";
        script << "        Log(\"Hit enemy!\");\n";
        script << "        // player.TakeDamage(10);\n";
        script << "    }\n";
        script << "    if (other.HasTag(\"Collectible\")) {\n";
        script << "        Log(\"Collected coin!\");\n";
        script << "        other.Destroy();\n";
        script << "    }\n";
        script << "}\n";
        
    } else if (templateType == ProjectTemplate::TopDown2D) {
        script << "// === TOP-DOWN ===\n\n";
        
        script << "function Update(deltaTime) {\n";
        script << "    var player = FindEntity(\"Player\");\n";
        script << "    if (player == null) return;\n\n";
        
        script << "    // Движение в 4 направлениях\n";
        script << "    var moveX = 0;\n";
        script << "    var moveY = 0;\n\n";
        
        script << "    if (Input.IsKeyDown(\"W\")) moveY = 1;\n";
        script << "    if (Input.IsKeyDown(\"S\")) moveY = -1;\n";
        script << "    if (Input.IsKeyDown(\"A\")) moveX = -1;\n";
        script << "    if (Input.IsKeyDown(\"D\")) moveX = 1;\n\n";
        
        script << "    // Применяем движение\n";
        script << "    player.Move(moveX, moveY, deltaTime);\n";
        script << "}\n";
        
    } else {
        // Empty template
        script << "function Update(deltaTime) {\n";
        script << "    // Ваш код здесь!\n";
        script << "}\n";
    }
    
    script << "\n// 💡 СОВЕТЫ:\n";
    script << "// • Log(text) - вывести в консоль\n";
    script << "// • FindEntity(name) - найти объект по имени\n";
    script << "// • Input.IsKeyDown(key) - проверить нажатие клавиши\n";
    script << "// • deltaTime - время с прошлого кадра\n";
    
    script.close();
    
    SAGE_INFO("✓ Example script created: {}", scriptPath);
    return true;
}

void QuickStartGuide::LogStep(const std::string& step, bool success) {
    if (success) {
        SAGE_INFO("   ✓ {}", step);
    } else {
        SAGE_ERROR("   ✗ {}", step);
    }
}

} // namespace SAGE
