#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "SAGE/Core/Core.h"

namespace SAGE {

class IScriptModule {
public:
    virtual ~IScriptModule() = default;
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void OnUpdate(float dt) = 0;
    virtual void ReloadScripts() {}
};

class ScriptEngine {
public:
    static void Init() {
        Get().InternalInit();
    }

    static void Shutdown() {
        Get().InternalShutdown();
    }

    static void Update(float dt) {
        Get().InternalUpdate(dt);
    }

    static void RegisterModule(const std::string& languageName, std::shared_ptr<IScriptModule> module) {
        Get().m_Modules[languageName] = module;
        module->Init();
    }

    static std::shared_ptr<IScriptModule> GetModule(const std::string& languageName) {
        auto it = Get().m_Modules.find(languageName);
        if (it != Get().m_Modules.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    static ScriptEngine& Get() {
        static ScriptEngine instance;
        return instance;
    }

    void InternalInit() {
        // Initialize core scripting systems if any
    }

    void InternalShutdown() {
        for (auto& [name, module] : m_Modules) {
            module->Shutdown();
        }
        m_Modules.clear();
    }

    void InternalUpdate(float dt) {
        for (auto& [name, module] : m_Modules) {
            module->OnUpdate(dt);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<IScriptModule>> m_Modules;
};

} // namespace SAGE
