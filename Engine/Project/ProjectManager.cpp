#include "ProjectManager.h"
#include <fstream>
#include <sstream>

namespace SAGE {

bool ProjectManager::CreateProject(const ProjectInfo& info) {
    SAGE_INFO("Creating new project: {}", info.name);
    
    // 1. Создаём структуру папок
    if (!CreateProjectStructure(info)) {
        SAGE_ERROR("Failed to create project structure");
        return false;
    }
    
    // 2. Создаём конфигурационные файлы
    if (!CreateConfigFiles(info)) {
        SAGE_ERROR("Failed to create config files");
        return false;
    }
    
    // 3. Создаём стартовую сцену
    if (!CreateStarterScene(info)) {
        SAGE_ERROR("Failed to create starter scene");
        return false;
    }
    
    // 4. Создаём стартовые скрипты (если нужно)
    if (info.type != ProjectType::CppOnly) {
        if (!CreateStarterScripts(info)) {
            SAGE_ERROR("Failed to create starter scripts");
            return false;
        }
    }
    
    // 5. Сохраняём .sageproject файл
    if (!SaveProjectFile(info)) {
        SAGE_ERROR("Failed to save project file");
        return false;
    }
    
    m_CurrentProject = info;
    m_ProjectLoaded = true;
    
    SAGE_INFO("✓ Project '{}' created successfully!", info.name);
    SAGE_INFO("  Path: {}", info.path);
    SAGE_INFO("  Type: {}", info.type == ProjectType::LogConOnly ? "LogCon Only" :
                              info.type == ProjectType::CppWithLogCon ? "C++ + LogCon" : "C++ Only");
    
    return true;
}

bool ProjectManager::CreateProjectStructure(const ProjectInfo& info) {
    namespace fs = std::filesystem;
    
    try {
        // Создаём корневую папку проекта
        fs::path projectPath(info.path);
        fs::create_directories(projectPath);
        
        // Создаём подпапки
        fs::create_directories(projectPath / "Assets");
        fs::create_directories(projectPath / "Assets" / "Sprites");
        fs::create_directories(projectPath / "Assets" / "Sounds");
        fs::create_directories(projectPath / "Assets" / "Music");
        fs::create_directories(projectPath / "Assets" / "Fonts");
        
        fs::create_directories(projectPath / "Scenes");
        fs::create_directories(projectPath / "Scripts");
        
        if (info.type != ProjectType::LogConOnly) {
            fs::create_directories(projectPath / "Source");
            fs::create_directories(projectPath / "Include");
        }
        
        fs::create_directories(projectPath / "Build");
        
        SAGE_INFO("✓ Project structure created");
        return true;
        
    } catch (const std::exception& e) {
        SAGE_ERROR("Failed to create project structure: {}", e.what());
        return false;
    }
}

bool ProjectManager::CreateStarterScene(const ProjectInfo& info) {
    namespace fs = std::filesystem;
    
    std::string sceneName = "MainScene.scene";
    fs::path scenePath = fs::path(info.path) / "Scenes" / sceneName;
    
    std::ofstream sceneFile(scenePath);
    if (!sceneFile.is_open()) {
        SAGE_ERROR("Failed to create scene file: {}", scenePath.string());
        return false;
    }
    
    // Создаём JSON сцены с базовой камерой
    sceneFile << "{\n";
    sceneFile << "  \"scene\": {\n";
    sceneFile << "    \"name\": \"Main Scene\",\n";
    sceneFile << "    \"entities\": [\n";
    
    // ОБЯЗАТЕЛЬНО: Камера (чтобы всё отображалось!)
    sceneFile << "      {\n";
    sceneFile << "        \"name\": \"MainCamera\",\n";
    sceneFile << "        \"components\": {\n";
    sceneFile << "          \"Transform\": {\n";
    sceneFile << "            \"position\": [0, 0],\n";
    sceneFile << "            \"rotation\": 0,\n";
    sceneFile << "            \"scale\": [1, 1]\n";
    sceneFile << "          },\n";
    sceneFile << "          \"Camera\": {\n";
    sceneFile << "            \"width\": " << info.windowWidth << ",\n";
    sceneFile << "            \"height\": " << info.windowHeight << ",\n";
    sceneFile << "            \"zoom\": 1.0,\n";
    sceneFile << "            \"isMain\": true\n";
    sceneFile << "          }\n";
    sceneFile << "        }\n";
    sceneFile << "      }";
    
    // Добавляем объекты в зависимости от шаблона
    switch (info.templateType) {
        case ProjectTemplate::Platformer2D:
            // Player
            sceneFile << ",\n      {\n";
            sceneFile << "        \"name\": \"Player\",\n";
            sceneFile << "        \"components\": {\n";
            sceneFile << "          \"Transform\": { \"position\": [0, 0], \"size\": [32, 32] },\n";
            sceneFile << "          \"Sprite\": { \"color\": [0, 128, 255, 255] },\n";
            sceneFile << "          \"Physics\": { \"type\": \"dynamic\", \"fixedRotation\": true },\n";
            sceneFile << "          \"PlayerMovement\": { \"speed\": 200, \"jumpForce\": 400 }\n";
            sceneFile << "        }\n";
            sceneFile << "      },\n";
            // Ground
            sceneFile << "      {\n";
            sceneFile << "        \"name\": \"Ground\",\n";
            sceneFile << "        \"components\": {\n";
            sceneFile << "          \"Transform\": { \"position\": [0, -200], \"size\": [800, 50] },\n";
            sceneFile << "          \"Sprite\": { \"color\": [100, 100, 100, 255] },\n";
            sceneFile << "          \"Physics\": { \"type\": \"static\" }\n";
            sceneFile << "        }\n";
            sceneFile << "      }";
            break;
            
        case ProjectTemplate::TopDown2D:
            // Player
            sceneFile << ",\n      {\n";
            sceneFile << "        \"name\": \"Player\",\n";
            sceneFile << "        \"components\": {\n";
            sceneFile << "          \"Transform\": { \"position\": [0, 0], \"size\": [32, 32] },\n";
            sceneFile << "          \"Sprite\": { \"color\": [0, 255, 0, 255] },\n";
            sceneFile << "          \"PlayerMovement\": { \"speed\": 150, \"mode\": \"topdown\" }\n";
            sceneFile << "        }\n";
            sceneFile << "      }";
            break;
            
        case ProjectTemplate::Empty:
        default:
            // Только камера - уже добавлена выше
            break;
    }
    
    sceneFile << "\n    ]\n";
    sceneFile << "  }\n";
    sceneFile << "}\n";
    
    sceneFile.close();
    
    SAGE_INFO("✓ Starter scene created: {}", sceneName);
    return true;
}

bool ProjectManager::CreateStarterScripts(const ProjectInfo& info) {
    namespace fs = std::filesystem;
    
    // Создаём main.logcon скрипт для новичков
    fs::path mainScriptPath = fs::path(info.path) / "Scripts" / "main.logcon";
    
    std::ofstream scriptFile(mainScriptPath);
    if (!scriptFile.is_open()) {
        SAGE_ERROR("Failed to create main script");
        return false;
    }
    
    // Простой стартовый скрипт на LogCon
    scriptFile << "// " << info.name << " - Main Script\n";
    scriptFile << "// Этот скрипт запускается при старте игры\n\n";
    
    scriptFile << "function Start() {\n";
    scriptFile << "    // Вызывается один раз при запуске\n";
    scriptFile << "    Log(\"Game Started: " << info.name << "\");\n";
    scriptFile << "}\n\n";
    
    scriptFile << "function Update(deltaTime) {\n";
    scriptFile << "    // Вызывается каждый кадр\n";
    scriptFile << "    // deltaTime - время с прошлого кадра\n";
    scriptFile << "}\n\n";
    
    // Добавляем примеры в зависимости от шаблона
    if (info.templateType == ProjectTemplate::Platformer2D) {
        scriptFile << "// Пример: Управление игроком\n";
        scriptFile << "function PlayerUpdate(player, deltaTime) {\n";
        scriptFile << "    if (Input.IsKeyDown(\"A\") || Input.IsKeyDown(\"Left\")) {\n";
        scriptFile << "        player.MoveLeft();\n";
        scriptFile << "    }\n";
        scriptFile << "    if (Input.IsKeyDown(\"D\") || Input.IsKeyDown(\"Right\")) {\n";
        scriptFile << "        player.MoveRight();\n";
        scriptFile << "    }\n";
        scriptFile << "    if (Input.IsKeyPressed(\"Space\")) {\n";
        scriptFile << "        player.Jump();\n";
        scriptFile << "    }\n";
        scriptFile << "}\n";
    }
    
    scriptFile.close();
    
    // Создаём README для новичков
    fs::path readmePath = fs::path(info.path) / "README.md";
    std::ofstream readme(readmePath);
    if (readme.is_open()) {
        readme << "# " << info.name << "\n\n";
        readme << "## 🎮 Как начать разработку\n\n";
        readme << "### Шаг 1: Откройте проект в редакторе\n";
        readme << "Запустите редактор SAGE Engine и откройте этот проект.\n\n";
        readme << "### Шаг 2: Редактируйте сцену\n";
        readme << "Откройте `Scenes/MainScene.scene` и добавляйте объекты.\n\n";
        readme << "### Шаг 3: Пишите код\n";
        
        if (info.type == ProjectType::LogConOnly) {
            readme << "Редактируйте `Scripts/main.logcon` - простой язык для новичков!\n\n";
            readme << "```logcon\n";
            readme << "function Update(deltaTime) {\n";
            readme << "    // Ваш код здесь\n";
            readme << "}\n";
            readme << "```\n\n";
        } else {
            readme << "Выбирайте: LogCon (простой) или C++ (мощный).\n\n";
        }
        
        readme << "### Шаг 4: Запустите игру\n";
        readme << "Нажмите Play (▶) в редакторе!\n\n";
        readme << "## 📚 Документация\n";
        readme << "- [LogCon Reference](https://sage-engine.dev/docs/logcon)\n";
        readme << "- [API Reference](https://sage-engine.dev/docs/api)\n";
        readme << "- [Tutorials](https://sage-engine.dev/tutorials)\n";
        
        readme.close();
    }
    
    SAGE_INFO("✓ Starter scripts created");
    return true;
}

bool ProjectManager::CreateConfigFiles(const ProjectInfo& info) {
    namespace fs = std::filesystem;
    
    // engine_config.json
    fs::path configPath = fs::path(info.path) / "engine_config.json";
    std::ofstream config(configPath);
    if (!config.is_open()) {
        SAGE_ERROR("Failed to create config file");
        return false;
    }
    
    config << "{\n";
    config << "  \"window\": {\n";
    config << "    \"title\": \"" << (info.windowTitle.empty() ? info.name : info.windowTitle) << "\",\n";
    config << "    \"width\": " << info.windowWidth << ",\n";
    config << "    \"height\": " << info.windowHeight << ",\n";
    config << "    \"fullscreen\": " << (info.fullscreen ? "true" : "false") << ",\n";
    config << "    \"vsync\": true\n";
    config << "  },\n";
    config << "  \"physics\": {\n";
    config << "    \"gravity\": [0, -9.81],\n";
    config << "    \"timestep\": 0.016666\n";
    config << "  },\n";
    config << "  \"audio\": {\n";
    config << "    \"masterVolume\": 1.0,\n";
    config << "    \"musicVolume\": 0.7,\n";
    config << "    \"sfxVolume\": 1.0\n";
    config << "  },\n";
    config << "  \"rendering\": {\n";
    config << "    \"targetFPS\": 60,\n";
    config << "    \"enablePostProcessing\": true\n";
    config << "  }\n";
    config << "}\n";
    
    config.close();
    
    SAGE_INFO("✓ Config files created");
    return true;
}

bool ProjectManager::SaveProjectFile(const ProjectInfo& info) {
    namespace fs = std::filesystem;
    
    fs::path projectFilePath = fs::path(info.path) / (info.name + ".sageproject");
    std::ofstream file(projectFilePath);
    if (!file.is_open()) {
        SAGE_ERROR("Failed to save .sageproject file");
        return false;
    }
    
    file << "{\n";
    file << "  \"name\": \"" << info.name << "\",\n";
    file << "  \"version\": \"" << info.version << "\",\n";
    file << "  \"author\": \"" << info.author << "\",\n";
    file << "  \"type\": \"" << (info.type == ProjectType::LogConOnly ? "LogConOnly" :
                                   info.type == ProjectType::CppWithLogCon ? "CppWithLogCon" : "CppOnly") << "\",\n";
    file << "  \"template\": \"" << (info.templateType == ProjectTemplate::Platformer2D ? "Platformer2D" :
                                      info.templateType == ProjectTemplate::TopDown2D ? "TopDown2D" : "Empty") << "\",\n";
    file << "  \"mainScene\": \"Scenes/MainScene.scene\",\n";
    file << "  \"engineVersion\": \"1.0.0\"\n";
    file << "}\n";
    
    file.close();
    
    SAGE_INFO("✓ Project file saved: {}", projectFilePath.string());
    return true;
}

bool ProjectManager::LoadProject(const std::string& projectPath) {
    if (!LoadProjectFile(projectPath)) {
        return false;
    }
    
    m_ProjectLoaded = true;
    SAGE_INFO("✓ Project '{}' loaded successfully", m_CurrentProject.name);
    
    return true;
}

bool ProjectManager::LoadProjectFile(const std::string& path) {
    // TODO: Implement JSON parsing
    SAGE_WARNING("LoadProjectFile not fully implemented yet");
    return false;
}

bool ProjectManager::SaveProject() {
    if (!m_ProjectLoaded) {
        SAGE_WARNING("No project loaded to save");
        return false;
    }
    
    return SaveProjectFile(m_CurrentProject);
}

std::string ProjectManager::GetAssetsPath() const {
    namespace fs = std::filesystem;
    return (fs::path(m_CurrentProject.path) / "Assets").string();
}

std::string ProjectManager::GetScenesPath() const {
    namespace fs = std::filesystem;
    return (fs::path(m_CurrentProject.path) / "Scenes").string();
}

std::string ProjectManager::GetScriptsPath() const {
    namespace fs = std::filesystem;
    return (fs::path(m_CurrentProject.path) / "Scripts").string();
}

void ProjectManager::CloseProject() {
    m_ProjectLoaded = false;
    SAGE_INFO("Project closed");
}

} // namespace SAGE
