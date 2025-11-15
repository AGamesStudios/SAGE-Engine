#pragma once

#include "Core/Core.h"
#include "Core/Logger.h"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace SAGE {

/// @brief Типы проектов для разного уровня сложности
enum class ProjectType {
    LogConOnly,     ///< Только LogCon скрипты (для новичков)
    CppWithLogCon,  ///< C++ + LogCon (средний уровень)
    CppOnly         ///< Только C++ (продвинутый)
};

/// @brief Шаблоны стартовых проектов
enum class ProjectTemplate {
    Empty,          ///< Пустой проект (только камера)
    Platformer2D,   ///< 2D платформер
    TopDown2D,      ///< 2D вид сверху
    Puzzle,         ///< Головоломка
    RPG,            ///< RPG игра
    Custom          ///< Пользовательский
};

/// @brief Информация о проекте
struct ProjectInfo {
    std::string name;               ///< Название проекта
    std::string path;               ///< Путь к проекту
    ProjectType type;               ///< Тип проекта
    ProjectTemplate templateType;   ///< Шаблон проекта
    std::string mainScene;          ///< Главная сцена
    std::string version;            ///< Версия проекта
    std::string author;             ///< Автор
    
    // Настройки окна
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string windowTitle;
    bool fullscreen = false;
    
    ProjectInfo() : type(ProjectType::LogConOnly), 
                   templateType(ProjectTemplate::Empty),
                   version("1.0.0") {}
};

/// @brief Менеджер проектов - создание и управление игровыми проектами
class ProjectManager {
public:
    static ProjectManager& Instance() {
        static ProjectManager instance;
        return instance;
    }

    /// @brief Создать новый проект с автоматической настройкой
    /// @param info Информация о проекте
    /// @return true если успешно
    bool CreateProject(const ProjectInfo& info);
    
    /// @brief Загрузить существующий проект
    /// @param projectPath Путь к файлу .sageproject
    /// @return true если успешно
    bool LoadProject(const std::string& projectPath);
    
    /// @brief Сохранить текущий проект
    bool SaveProject();
    
    /// @brief Получить информацию о текущем проекте
    const ProjectInfo& GetCurrentProject() const { return m_CurrentProject; }
    
    /// @brief Проверка - загружен ли проект
    bool HasProject() const { return m_ProjectLoaded; }
    
    /// @brief Получить путь к ресурсам проекта
    std::string GetAssetsPath() const;
    
    /// @brief Получить путь к сценам проекта
    std::string GetScenesPath() const;
    
    /// @brief Получить путь к скриптам проекта
    std::string GetScriptsPath() const;
    
    /// @brief Закрыть текущий проект
    void CloseProject();

private:
    ProjectManager() = default;
    ~ProjectManager() = default;
    
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;
    
    /// @brief Создать структуру папок проекта
    bool CreateProjectStructure(const ProjectInfo& info);
    
    /// @brief Создать стартовую сцену по шаблону
    bool CreateStarterScene(const ProjectInfo& info);
    
    /// @brief Создать стартовые скрипты
    bool CreateStarterScripts(const ProjectInfo& info);
    
    /// @brief Создать конфигурационные файлы
    bool CreateConfigFiles(const ProjectInfo& info);
    
    /// @brief Сохранить .sageproject файл
    bool SaveProjectFile(const ProjectInfo& info);
    
    /// @brief Загрузить .sageproject файл
    bool LoadProjectFile(const std::string& path);

private:
    ProjectInfo m_CurrentProject;
    bool m_ProjectLoaded = false;
};

} // namespace SAGE
