#pragma once

/**
 * @file SAGE.h
 * @brief Main header for SAGE Engine public API
 * 
 * Include this file to access all engine functionality.
 * 
 * Example usage:
 * @code
 * #include <SAGE/SAGE.h>
 * 
 * int main() {
 *     SAGE::EngineConfig config;
 *     config.windowTitle = "My Game";
 *     
 *     SAGE::IEngine* engine = SAGE::CreateEngine();
 *     engine->Initialize(config);
 *     engine->Run();
 *     SAGE::DestroyEngine(engine);
 *     
 *     return 0;
 * }
 * @endcode
 */

// Core types
#include "Types.h"

// Main interfaces
#include "IEngine.h"
#include "IScene.h"

// Editor API (optional, only for editor tools)
#include "Editor/EditorAPI.h"

namespace SAGE {

/**
 * @brief Engine version information
 */
struct Version {
    static constexpr int Major = 1;
    static constexpr int Minor = 0;
    static constexpr int Patch = 0;
    
    static const char* GetString() { return "1.0.0"; }
};

} // namespace SAGE
