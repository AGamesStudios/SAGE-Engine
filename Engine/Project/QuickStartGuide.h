#pragma once

#include "Project/ProjectManager.h"
#include "Project/GameObjectTemplates.h"
#include "ECS/ECSContext.h"
#include <string>
#include <memory>

namespace SAGE {

/// @brief Quick Start Guide - автоматизированная система для новичков
/// Помогает создать первый проект за несколько кликов
class QuickStartGuide {
public:
    
    /// @brief Пошаговое создание первого проекта
    struct QuickStartOptions {
        std::string projectName = "MyFirstGame";
        std::string projectPath = "";
        ProjectTemplate templateType = ProjectTemplate::Platformer2D;
        bool useLogConOnly = true; // true = только LogCon (для новичков)
        
        // Автоматические настройки
        bool autoCreateCamera = true;
        bool autoCreatePlayer = true;
        bool autoCreateGround = true;
    };
    
    /// @brief Создать первый проект автоматически
    /// @param options Опции создания
    /// @return true если успешно
    static bool CreateFirstProject(const QuickStartOptions& options);
    
    /// @brief Создать и запустить простую демо-сцену
    /// @param ecsContext ECS контекст для создания объектов
    /// @param templateType Тип шаблона (Platformer, TopDown и т.д.)
    /// @return true если успешно
    static bool SetupDemoScene(ECS::ECSContext& ecsContext, ProjectTemplate templateType);
    
    /// @brief Показать туториал для новичков (в консоли)
    static void ShowBeginnerTutorial();
    
    /// @brief Создать пример LogCon скрипта с комментариями
    /// @param scriptPath Путь для сохранения скрипта
    /// @param templateType Тип шаблона
    /// @return true если успешно
    static bool CreateExampleScript(const std::string& scriptPath, ProjectTemplate templateType);

private:
    static void LogStep(const std::string& step, bool success = true);
};

} // namespace SAGE
