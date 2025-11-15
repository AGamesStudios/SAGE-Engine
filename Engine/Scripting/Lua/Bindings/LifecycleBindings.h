#pragma once

#include "Scripting/Lua/Core/ScriptLifecycle.h"
#include "Core/Logger.h"
#include "Scripting/Lua/Core/LuaForward.h"

namespace SAGE {

    /**
     * @brief Lifecycle and advanced scripting bindings
     */
    #if SAGE_ENABLE_LUA

    class LifecycleBindings {
    public:
        static void BindAll(sol::state& lua, 
                           Scripting::ScriptLifecycle* lifecycle = nullptr,
                           Scripting::GameStateManager* stateManager = nullptr,
                           Scripting::SceneManager* sceneManager = nullptr) {
            
            // Script lifecycle
            if (lifecycle) {
                BindScriptLifecycle(lua, lifecycle);
            }
            
            // Game state management
            if (stateManager) {
                BindGameStateManager(lua, stateManager);
            }
            
            // Scene management
            if (sceneManager) {
                BindSceneManager(lua, sceneManager);
            }
            
            // Utility bindings
            BindCoroutineHelpers(lua);
            BindEventHelpers(lua);
            
            SAGE_INFO("Lifecycle bindings registered");
        }

    private:
        static void BindScriptLifecycle(sol::state& lua, Scripting::ScriptLifecycle* lifecycle) {
            auto scripts = lua.create_table();
            
            // Load script template
            scripts["LoadScript"] = [lifecycle](const std::string& name, const std::string& path) {
                return lifecycle->LoadScript(name, path);
            };

            #else

            class LifecycleBindings {
            public:
                static void BindAll(sol::state&, Scripting::ScriptLifecycle* = nullptr,
                                     Scripting::GameStateManager* = nullptr,
                                     Scripting::SceneManager* = nullptr) {}
            };

            #endif
            
            // Create instance from template
            scripts["CreateInstance"] = [lifecycle](const std::string& scriptName, const std::string& instanceName) {
                return lifecycle->CreateInstance(scriptName, instanceName) != nullptr;
            };
            
            // Destroy instance
            scripts["DestroyInstance"] = [lifecycle](const std::string& instanceName) {
                lifecycle->DestroyInstance(instanceName);
            };
            
            // Get instance table
            scripts["GetInstance"] = [lifecycle](const std::string& name) {
                return lifecycle->GetInstanceTable(name);
            };
            
            // Broadcast event to all scripts
            scripts["BroadcastEvent"] = [lifecycle](const std::string& eventName, sol::object data) {
                lifecycle->BroadcastEvent(eventName, data);
            };
            
            // Send event to specific script
            scripts["SendEvent"] = [lifecycle](const std::string& instanceName, const std::string& eventName, sol::object data) {
                lifecycle->SendEventTo(instanceName, eventName, data);
            };
            
            // Start coroutine
            scripts["StartCoroutine"] = [lifecycle](const std::string& instanceName, sol::protected_function coro) {
                lifecycle->StartCoroutine(instanceName, coro);
            };
            
            lua["Scripts"] = scripts;
        }

        static void BindGameStateManager(sol::state& lua, Scripting::GameStateManager* stateManager) {
            auto gameState = lua.create_table();
            
            // Register state
            gameState["Register"] = [stateManager](const std::string& name, sol::table stateTable) {
                stateManager->RegisterState(name, stateTable);
            };
            
            // Push state (adds to stack)
            gameState["Push"] = [stateManager](const std::string& name) {
                stateManager->PushState(name);
            };
            
            // Pop state (removes from stack)
            gameState["Pop"] = [stateManager]() {
                stateManager->PopState();
            };
            
            // Change state (replaces current)
            gameState["Change"] = [stateManager](const std::string& name) {
                stateManager->ChangeState(name);
            };
            
            // Get current state
            gameState["GetCurrent"] = [stateManager]() {
                return stateManager->GetCurrentState();
            };
            
            // Check if in state
            gameState["IsInState"] = [stateManager](const std::string& name) {
                return stateManager->IsInState(name);
            };
            
            lua["GameState"] = gameState;
        }

        static void BindSceneManager(sol::state& lua, Scripting::SceneManager* sceneManager) {
            auto scene = lua.create_table();
            
            // Register scene
            scene["Register"] = [sceneManager](const std::string& name, sol::table sceneTable) {
                sceneManager->RegisterScene(name, sceneTable);
            };
            
            // Load scene
            scene["Load"] = [sceneManager](const std::string& name) {
                sceneManager->LoadScene(name);
            };
            
            // Unload scene
            scene["Unload"] = [sceneManager](const std::string& name) {
                sceneManager->UnloadScene(name);
            };
            
            // Activate scene
            scene["Activate"] = [sceneManager](const std::string& name) {
                sceneManager->ActivateScene(name);
            };
            
            // Get current scene name
            scene["GetCurrent"] = [sceneManager]() {
                return sceneManager->GetCurrentSceneName();
            };
            
            lua["Scene"] = scene;
        }

        static void BindCoroutineHelpers(sol::state& lua) {
            // Create coroutine helper table
            auto coro = lua.create_table();
            
            // Wait for seconds
            coro["WaitForSeconds"] = [](float seconds) {
                return [seconds, elapsed = 0.0f](float dt) mutable {
                    elapsed += dt;
                    return elapsed < seconds;
                };
            };
            
            // Wait until condition
            coro["WaitUntil"] = [](sol::function condition) {
                return [condition]() {
                    return !condition();
                };
            };
            
            // Wait while condition
            coro["WaitWhile"] = [](sol::function condition) {
                return [condition]() {
                    return condition();
                };
            };
            
            lua["Coroutine"] = coro;
        }

        static void BindEventHelpers(sol::state& lua) {
            // Event data wrapper
            lua.new_usertype<sol::table>("EventData",
                sol::constructors<sol::table()>()
            );
        }
    };

} // namespace SAGE
