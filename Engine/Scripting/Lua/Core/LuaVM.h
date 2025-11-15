#pragma once

#include "Core/Logger.h"
#include "Memory/Ref.h"
#include "LuaForward.h"
#include "ScriptVariables.h"
#include "ScriptLifecycle.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>

#if SAGE_ENABLE_LUA

namespace SAGE {

    /**
     * @brief LuaVM - Lua virtual machine wrapper using sol2
     * 
     * Features:
     * - Script loading and execution
     * - Hot-reload support
     * - Global variable management
     * - Function calling from C++
     * - Error handling
     * 
     * Usage:
     *   LuaVM vm;
     *   vm.LoadScript("player_controller", "assets/scripts/player.lua");
     *   vm.CallFunction("OnUpdate", deltaTime);
     *   int health = vm.GetGlobal<int>("playerHealth");
     */
    class LuaVM {
    public:
        LuaVM() {
            m_Lua.open_libraries(
                sol::lib::base,
                sol::lib::package,
                sol::lib::math,
                sol::lib::string,
                sol::lib::table
            );
            
            // Create shared variable storage
            m_Variables = std::make_shared<Scripting::ScriptVariables>();
            
            // Create lifecycle managers
            m_ScriptLifecycle = std::make_shared<Scripting::ScriptLifecycle>(m_Lua);
            m_GameStateManager = std::make_shared<Scripting::GameStateManager>(m_Lua);
            m_SceneManager = std::make_shared<Scripting::SceneManager>(m_Lua);
            
            SAGE_INFO("Lua VM initialized (Lua {0})", LUA_VERSION_MAJOR << "." << LUA_VERSION_MINOR);
        }
        
        // Destructor - sol::state automatically calls lua_close() on destruction
        // No manual cleanup needed (sol2 handles it via RAII)
        ~LuaVM() = default;
        
        // Script loading
        bool LoadScript(const std::string& name, const std::string& filepath) {
            try {
                auto result = m_Lua.script_file(filepath);
                m_LoadedScripts[name] = filepath;
                SAGE_INFO("Loaded Lua script: {0} ({1})", name, filepath);
                return true;
            }
            catch (const sol::error& e) {
                SAGE_ERROR("Lua script error in {0}: {1}", filepath, e.what());
                return false;
            }
        }
        
        bool ExecuteString(const std::string& code) {
            try {
                m_Lua.script(code);
                return true;
            }
            catch (const sol::error& e) {
                SAGE_ERROR("Lua execution error: {0}", e.what());
                return false;
            }
        }
        
        bool ReloadScript(const std::string& name) {
            auto it = m_LoadedScripts.find(name);
            if (it == m_LoadedScripts.end()) {
                SAGE_WARN("Script not found for reload: {0}", name);
                return false;
            }
            
            SAGE_INFO("Hot-reloading Lua script: {0}", name);
            return LoadScript(name, it->second);
        }
        
        void ReloadAllScripts() {
            for (const auto& [name, filepath] : m_LoadedScripts) {
                LoadScript(name, filepath);
            }
        }
        
        // Function calling
        template<typename... Args>
        bool CallFunction(const std::string& functionName, Args&&... args) {
            try {
                sol::protected_function func = m_Lua[functionName];
                if (!func.valid()) {
                    SAGE_WARN("Lua function not found: {0}", functionName);
                    return false;
                }
                
                auto result = func(std::forward<Args>(args)...);
                if (!result.valid()) {
                    sol::error err = result;
                    SAGE_ERROR("Lua function call error ({0}): {1}", functionName, err.what());
                    return false;
                }
                
                return true;
            }
            catch (const sol::error& e) {
                SAGE_ERROR("Lua function call exception ({0}): {1}", functionName, e.what());
                return false;
            }
        }
        
        template<typename R, typename... Args>
        std::optional<R> CallFunctionWithReturn(const std::string& functionName, Args&&... args) {
            try {
                sol::protected_function func = m_Lua[functionName];
                if (!func.valid()) {
                    SAGE_WARN("Lua function not found: {0}", functionName);
                    return std::nullopt;
                }
                
                sol::protected_function_result result = func(std::forward<Args>(args)...);
                if (!result.valid()) {
                    sol::error err = result;
                    SAGE_ERROR("Lua function call error ({0}): {1}", functionName, err.what());
                    return std::nullopt;
                }
                
                return result.get<R>();
            }
            catch (const sol::error& e) {
                SAGE_ERROR("Lua function call exception ({0}): {1}", functionName, e.what());
                return std::nullopt;
            }
        }
        
        // Global variables
        template<typename T>
        void SetGlobal(const std::string& name, T value) {
            m_Lua[name] = value;
        }
        
        template<typename T>
        T GetGlobal(const std::string& name, T defaultValue = T()) {
            sol::optional<T> value = m_Lua[name];
            return value.value_or(defaultValue);
        }
        
        bool HasGlobal(const std::string& name) {
            return m_Lua[name] != sol::lua_nil;
        }
        
        // Table management
        sol::table CreateTable() {
            return m_Lua.create_table();
        }
        
        sol::table GetTable(const std::string& name) {
            return m_Lua[name];
        }
        
        void SetTable(const std::string& name, sol::table table) {
            m_Lua[name] = table;
        }
        
        // Direct access to sol::state for advanced binding
        sol::state& GetState() {
            return m_Lua;
        }
        
        const sol::state& GetState() const {
            return m_Lua;
        }
        
        // Access to variable system
        std::shared_ptr<Scripting::ScriptVariables> GetVariables() { 
            return m_Variables; 
        }
        
        // Access to lifecycle managers
        std::shared_ptr<Scripting::ScriptLifecycle> GetScriptLifecycle() {
            return m_ScriptLifecycle;
        }
        
        std::shared_ptr<Scripting::GameStateManager> GetGameStateManager() {
            return m_GameStateManager;
        }
        
        std::shared_ptr<Scripting::SceneManager> GetSceneManager() {
            return m_SceneManager;
        }
        
        // Lifecycle updates (call from engine loop)
        void UpdateScripts(float deltaTime) {
            if (m_ScriptLifecycle) {
                m_ScriptLifecycle->UpdateAll(deltaTime);
                m_ScriptLifecycle->UpdateCoroutines(deltaTime);
            }
            if (m_GameStateManager) {
                m_GameStateManager->UpdateCurrentState(deltaTime);
            }
        }
        
        void FixedUpdateScripts(float fixedDeltaTime) {
            if (m_ScriptLifecycle) {
                m_ScriptLifecycle->FixedUpdateAll(fixedDeltaTime);
            }
        }
        
        void StartAllScripts() {
            if (m_ScriptLifecycle) {
                m_ScriptLifecycle->StartAll();
            }
        }
        
        void DestroyAllScripts() {
            if (m_ScriptLifecycle) {
                m_ScriptLifecycle->DestroyAll();
            }
        }
        
        // Error handling
        void SetErrorHandler(std::function<void(const std::string&)> handler) {
            m_ErrorHandler = handler;
        }
        
        // Queries
        bool IsScriptLoaded(const std::string& name) const {
            return m_LoadedScripts.find(name) != m_LoadedScripts.end();
        }
        
        const std::unordered_map<std::string, std::string>& GetLoadedScripts() const {
            return m_LoadedScripts;
        }
        
    private:
        sol::state m_Lua;
        std::unordered_map<std::string, std::string> m_LoadedScripts; // name -> filepath
        std::function<void(const std::string&)> m_ErrorHandler;
        std::shared_ptr<Scripting::ScriptVariables> m_Variables;
        
        // Lifecycle managers
        std::shared_ptr<Scripting::ScriptLifecycle> m_ScriptLifecycle;
        std::shared_ptr<Scripting::GameStateManager> m_GameStateManager;
        std::shared_ptr<Scripting::SceneManager> m_SceneManager;
    };

} // namespace SAGE

#else

namespace SAGE {

    class LuaVM {
    public:
        LuaVM() = default;
        ~LuaVM() = default;

        bool LoadScript(const std::string&, const std::string&) { return false; }
        bool ExecuteString(const std::string&) { return false; }
        bool ReloadScript(const std::string&) { return false; }
        void ReloadAllScripts() {}

        template<typename... Args>
        bool CallFunction(const std::string&, Args&&...) { return false; }

        template<typename R, typename... Args>
        std::optional<R> CallFunctionWithReturn(const std::string&, Args&&...) { return std::nullopt; }

        template<typename T>
        void SetGlobal(const std::string&, T) {}

        template<typename T>
        T GetGlobal(const std::string&, T defaultValue = T()) { return defaultValue; }

        bool HasGlobal(const std::string&) { return false; }

        sol::table CreateTable() { return {}; }
        sol::table GetTable(const std::string&) { return {}; }
        void SetTable(const std::string&, sol::table) {}

        sol::state& GetState() { return m_DummyState; }
        const sol::state& GetState() const { return m_DummyState; }

        std::shared_ptr<Scripting::ScriptVariables> GetVariables() { return m_Variables; }
        std::shared_ptr<Scripting::ScriptLifecycle> GetScriptLifecycle() { return m_ScriptLifecycle; }
        std::shared_ptr<Scripting::GameStateManager> GetGameStateManager() { return m_GameStateManager; }
        std::shared_ptr<Scripting::SceneManager> GetSceneManager() { return m_SceneManager; }

        void RegisterUsertype(const std::string&, std::function<void(sol::state&)>) {}
        void UnregisterUsertype(const std::string&) {}
        bool HasScript(const std::string&) const { return false; }
        void Clear() {}

    private:
        sol::state m_DummyState;
        std::shared_ptr<Scripting::ScriptVariables> m_Variables = std::make_shared<Scripting::ScriptVariables>();
        std::shared_ptr<Scripting::ScriptLifecycle> m_ScriptLifecycle = std::make_shared<Scripting::ScriptLifecycle>(m_DummyState);
        std::shared_ptr<Scripting::GameStateManager> m_GameStateManager = std::make_shared<Scripting::GameStateManager>(m_DummyState);
        std::shared_ptr<Scripting::SceneManager> m_SceneManager = std::make_shared<Scripting::SceneManager>(m_DummyState);
    };

} // namespace SAGE

#endif
