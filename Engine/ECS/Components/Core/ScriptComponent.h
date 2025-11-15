#pragma once

#include "Scripting/Lua/Core/LuaVM.h"

namespace SAGE {

    /**
     * @brief ScriptComponent - ECS component for Lua scripts
     */
    struct ScriptComponent {
        std::string scriptName;        // Script identifier (e.g., "player_controller")
        std::string scriptPath;        // File path (e.g., "assets/scripts/player.lua")
        bool enabled = true;
        bool autoReload = false;       // Hot-reload on file change
        
        // Script lifecycle callbacks (Lua function names)
        std::string onStartFunction = "OnStart";
        std::string onUpdateFunction = "OnUpdate";
        std::string onDestroyFunction = "OnDestroy";
        
        ScriptComponent() = default;
        
        ScriptComponent(const std::string& name, const std::string& path)
            : scriptName(name), scriptPath(path) {}
    };

} // namespace SAGE
