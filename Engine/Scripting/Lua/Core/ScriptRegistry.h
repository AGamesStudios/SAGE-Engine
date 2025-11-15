#pragma once

#include "ScriptModule.h"
#include "Core/Logger.h"
#include "Memory/Ref.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace SAGE {

    /**
     * @brief Script Registry - centralized script management system
     * 
     * Features:
     * - Module registration and lifecycle management
     * - Automatic dependency resolution
     * - Hot-reload monitoring
     * - Script search paths
     * - Event broadcasting to scripts
     */
    class ScriptRegistry {
    public:
        static ScriptRegistry& Instance() {
            static ScriptRegistry instance;
            return instance;
        }

        /**
         * @brief Register a script module
         */
        bool RegisterModule(const std::string& name, const std::string& filepath) {
            if (m_Modules.find(name) != m_Modules.end()) {
                SAGE_WARNING("ScriptRegistry: Module '{}' already registered", name);
                return false;
            }

            auto module = CreateRef<ScriptModule>(name, filepath);
            m_Modules[name] = module;
            SAGE_INFO("ScriptRegistry: Registered module '{}'", name);
            return true;
        }

        /**
         * @brief Load a module (with dependency resolution)
         */
        bool LoadModule(const std::string& name) {
            auto it = m_Modules.find(name);
            if (it == m_Modules.end()) {
                SAGE_ERROR("ScriptRegistry: Module '{}' not registered", name);
                return false;
            }

            auto& module = it->second;

            // Load dependencies first
            for (const auto& dep : module->GetDependencies()) {
                if (!IsModuleLoaded(dep)) {
                    SAGE_INFO("ScriptRegistry: Loading dependency '{}'", dep);
                    if (!LoadModule(dep)) {
                        SAGE_ERROR("ScriptRegistry: Failed to load dependency '{}'", dep);
                        return false;
                    }
                }
            }

            return module->Load();
        }

        /**
         * @brief Unload a module
         */
        void UnloadModule(const std::string& name) {
            auto it = m_Modules.find(name);
            if (it == m_Modules.end()) return;
            it->second->Unload();
        }

        /**
         * @brief Reload a module (hot-reload)
         */
        bool ReloadModule(const std::string& name) {
            auto it = m_Modules.find(name);
            if (it == m_Modules.end()) {
                SAGE_ERROR("ScriptRegistry: Module '{}' not found", name);
                return false;
            }
            return it->second->Reload();
        }

        /**
         * @brief Reload all modules
         */
        void ReloadAllModules() {
            SAGE_INFO("ScriptRegistry: Reloading all modules...");
            for (auto& [name, module] : m_Modules) {
                module->Reload();
            }
        }

        /**
         * @brief Update all loaded modules
         */
        void UpdateModules(float deltaTime) {
            for (auto& [name, module] : m_Modules) {
                if (module->IsLoaded()) {
                    module->Update(deltaTime);
                }
            }
        }

        /**
         * @brief Get a module by name
         */
        Ref<ScriptModule> GetModule(const std::string& name) {
            auto it = m_Modules.find(name);
            return (it != m_Modules.end()) ? it->second : nullptr;
        }

        /**
         * @brief Check if module is loaded
         */
        bool IsModuleLoaded(const std::string& name) const {
            auto it = m_Modules.find(name);
            return (it != m_Modules.end()) && it->second->IsLoaded();
        }

        /**
         * @brief Add script search path
         */
        void AddSearchPath(const std::string& path) {
            if (std::find(m_SearchPaths.begin(), m_SearchPaths.end(), path) == m_SearchPaths.end()) {
                m_SearchPaths.push_back(path);
                SAGE_INFO("ScriptRegistry: Added search path '{}'", path);
            }
        }

        /**
         * @brief Auto-discover and register scripts in search paths
         */
        void AutoDiscoverScripts(const std::string& extension = ".lua") {
            for (const auto& searchPath : m_SearchPaths) {
                if (!std::filesystem::exists(searchPath)) continue;

                for (const auto& entry : std::filesystem::recursive_directory_iterator(searchPath)) {
                    if (entry.is_regular_file() && entry.path().extension() == extension) {
                        std::string name = entry.path().stem().string();
                        std::string filepath = entry.path().string();
                        
                        // Auto-register if not already registered
                        if (m_Modules.find(name) == m_Modules.end()) {
                            RegisterModule(name, filepath);
                        }
                    }
                }
            }
        }

        /**
         * @brief Broadcast event to all loaded modules
         */
        template<typename... Args>
        void BroadcastEvent(const std::string& eventName, Args&&... args) {
            for (auto& [name, module] : m_Modules) {
                if (module->IsLoaded() && module->HasFunction(eventName)) {
                    module->CallFunction(eventName, std::forward<Args>(args)...);
                }
            }
        }

        /**
         * @brief Get all registered modules
         */
        const std::unordered_map<std::string, Ref<ScriptModule>>& GetModules() const {
            return m_Modules;
        }

        /**
         * @brief Clear all modules
         */
        void Clear() {
            for (auto& [name, module] : m_Modules) {
                module->Unload();
            }
            m_Modules.clear();
            SAGE_INFO("ScriptRegistry: Cleared all modules");
        }

    private:
        ScriptRegistry() = default;

        std::unordered_map<std::string, Ref<ScriptModule>> m_Modules;
        std::vector<std::string> m_SearchPaths;
    };

} // namespace SAGE
