#pragma once

#include "ECS/System.h"
// Temporarily disabled - Lua scripting not available
// #include "ECS/Components/Core/ScriptComponent.h"

namespace SAGE::ECS {

/// @brief Система выполнения скриптов (DISABLED - Lua not available)
/// Обновляет все ScriptComponent каждый кадр
class ScriptSystem : public ISystem {
public:
    ScriptSystem() {
        SetPriority(10); // Скрипты выполняются рано
    }

    void Update(Registry&, float) override {
        // Disabled - Lua scripting not available
        // auto scripts = registry.GetComponentView<ScriptComponent>();
        // 
        // for (auto& [entity, script] : scripts) {
        //     if (script.updateCallback) {
        //         script.updateCallback(registry, entity, deltaTime);
        //     }
        // }
    }

    std::string GetName() const override {
        return "ScriptSystem";
    }
};

} // namespace SAGE::ECS
