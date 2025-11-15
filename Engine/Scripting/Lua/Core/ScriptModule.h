#pragma once

#include "LuaVM.h"
#include "Core/Logger.h"
#include "Memory/Ref.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace SAGE {

    /**
     * @brief Script Module - isolated Lua environment for modular scripting
     * 
     * Features:
     * - Isolated lua_State per module (sandboxing)
     * - Module dependencies
     * - Hot-reload support
     * - Event callbacks
     * - Lifecycle hooks (OnLoad, OnUpdate, OnUnload)
     */
    class ScriptModule {
    public:
        ScriptModule(const std::string& name, const std::string& filepath)
            : m_Name(name), m_FilePath(filepath), m_Loaded(false) {}

        bool Load() {
            if (m_Loaded) {
                SAGE_WARNING("ScriptModule '{}' already loaded", m_Name);
                return true;
            }

            if (!m_VM.LoadScript(m_Name, m_FilePath)) {
                SAGE_ERROR("ScriptModule: Failed to load script '{}'", m_FilePath);
                return false;
            }

            // Call OnLoad callback if exists
            if (HasFunction("OnLoad")) {
                CallFunction("OnLoad");
            }

            m_Loaded = true;
            SAGE_INFO("ScriptModule '{}' loaded successfully", m_Name);
            return true;
        }

        void Unload() {
            if (!m_Loaded) return;

            // Call OnUnload callback if exists
            if (HasFunction("OnUnload")) {
                CallFunction("OnUnload");
            }

            m_Loaded = false;
            SAGE_INFO("ScriptModule '{}' unloaded", m_Name);
        }

        bool Reload() {
            SAGE_INFO("ScriptModule '{}' reloading...", m_Name);
            Unload();
            return Load();
        }

        void Update(float deltaTime) {
            if (!m_Loaded) return;
            if (HasFunction("OnUpdate")) {
                CallFunction("OnUpdate", deltaTime);
            }
        }

        // Function calling
        template<typename... Args>
        bool CallFunction(const std::string& functionName, Args&&... args) {
            return m_VM.CallFunction(functionName, std::forward<Args>(args)...);
        }

        template<typename R, typename... Args>
        std::optional<R> CallFunctionWithReturn(const std::string& functionName, Args&&... args) {
            return m_VM.CallFunctionWithReturn<R>(std::forward<Args>(args)...);
        }

        bool HasFunction(const std::string& functionName) const {
            return m_VM.HasGlobal(functionName);
        }

        // Global variables
        template<typename T>
        void SetVariable(const std::string& name, T value) {
            m_VM.SetGlobal(name, value);
        }

        template<typename T>
        T GetVariable(const std::string& name, T defaultValue = T()) {
            return m_VM.GetGlobal(name, defaultValue);
        }

        // Dependencies
        void AddDependency(const std::string& moduleName) {
            m_Dependencies.push_back(moduleName);
        }

        const std::vector<std::string>& GetDependencies() const {
            return m_Dependencies;
        }

        // Queries
        const std::string& GetName() const { return m_Name; }
        const std::string& GetFilePath() const { return m_FilePath; }
        bool IsLoaded() const { return m_Loaded; }

        // Direct VM access for advanced use
        LuaVM& GetVM() { return m_VM; }
        const LuaVM& GetVM() const { return m_VM; }

    private:
        std::string m_Name;
        std::string m_FilePath;
        bool m_Loaded;
        LuaVM m_VM;
        std::vector<std::string> m_Dependencies;
    };

    /**
     * @brief Script Context - shared data between C++ and Lua
     * 
     * Allows passing arbitrary data to scripts without tight coupling
     */
    class ScriptContext {
    public:
        // Generic value storage
        template<typename T>
        void Set(const std::string& key, const T& value) {
            m_Data[key] = std::any(value);
        }

        template<typename T>
        std::optional<T> Get(const std::string& key) const {
            auto it = m_Data.find(key);
            if (it == m_Data.end()) return std::nullopt;
            
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                SAGE_ERROR("ScriptContext: Type mismatch for key '{}'", key);
                return std::nullopt;
            }
        }

        bool Has(const std::string& key) const {
            return m_Data.find(key) != m_Data.end();
        }

        void Remove(const std::string& key) {
            m_Data.erase(key);
        }

        void Clear() {
            m_Data.clear();
        }

    private:
        std::unordered_map<std::string, std::any> m_Data;
    };

} // namespace SAGE
