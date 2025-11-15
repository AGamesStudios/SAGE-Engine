#include "ScriptLifecycle.h"
#include "Core/Logger.h"

namespace SAGE {
namespace Scripting {

    // ============================================================================
    // ScriptLifecycle Implementation
    // ============================================================================

    bool ScriptLifecycle::LoadScript(const std::string& name, const std::string& filepath) {
        try {
            auto result = m_Lua.script_file(filepath);
            
            // Expect script to return a table (the script template)
            if (result.valid() && result.get_type() == sol::type::table) {
                m_ScriptTemplates[name] = result.get<sol::table>();
                SAGE_INFO("Loaded Lua script template: {0}", name);
                return true;
            } else {
                SAGE_ERROR("Script {0} must return a table", filepath);
                return false;
            }
        }
        catch (const sol::error& e) {
            SAGE_ERROR("Failed to load script {0}: {1}", filepath, e.what());
            return false;
        }
    }

    ScriptLifecycle::ScriptInstance* ScriptLifecycle::CreateInstance(
        const std::string& scriptName, 
        const std::string& instanceName) {
        
        auto it = m_ScriptTemplates.find(scriptName);
        if (it == m_ScriptTemplates.end()) {
            SAGE_ERROR("Script template not found: {0}", scriptName);
            return nullptr;
        }

        // Create new instance by copying template
        ScriptInstance instance;
        instance.name = instanceName;
        instance.instance = m_Lua.create_table_with();
        
        // Copy all fields from template to instance
        sol::table& templateTable = it->second;
        for (auto& pair : templateTable) {
            instance.instance[pair.first] = pair.second;
        }
        
        // Set up lifecycle callbacks
        instance.onLoad = instance.instance["OnLoad"];
        instance.onStart = instance.instance["OnStart"];
        instance.onUpdate = instance.instance["OnUpdate"];
        instance.onFixedUpdate = instance.instance["OnFixedUpdate"];
        instance.onDestroy = instance.instance["OnDestroy"];
        
        // Call OnLoad immediately
        if (instance.onLoad.valid()) {
            CallScriptFunction(instance, instance.onLoad);
        }
        
        m_Instances[instanceName] = instance;
        SAGE_INFO("Created script instance: {0} (from {1})", instanceName, scriptName);
        
        return &m_Instances[instanceName];
    }

    void ScriptLifecycle::DestroyInstance(const std::string& instanceName) {
        auto it = m_Instances.find(instanceName);
        if (it == m_Instances.end()) return;
        
        auto& instance = it->second;
        
        // Call OnDestroy
        if (instance.onDestroy.valid()) {
            CallScriptFunction(instance, instance.onDestroy);
        }
        
        m_Instances.erase(it);
        SAGE_INFO("Destroyed script instance: {0}", instanceName);
    }

    void ScriptLifecycle::StartAll() {
        for (auto& [name, instance] : m_Instances) {
            if (!instance.started && instance.enabled && instance.onStart.valid()) {
                CallScriptFunction(instance, instance.onStart);
                instance.started = true;
            }
        }
    }

    void ScriptLifecycle::UpdateAll(float deltaTime) {
        for (auto& [name, instance] : m_Instances) {
            if (instance.enabled && instance.onUpdate.valid()) {
                CallScriptFunction(instance, instance.onUpdate, deltaTime);
            }
        }
    }

    void ScriptLifecycle::FixedUpdateAll(float fixedDeltaTime) {
        for (auto& [name, instance] : m_Instances) {
            if (instance.enabled && instance.onFixedUpdate.valid()) {
                CallScriptFunction(instance, instance.onFixedUpdate, fixedDeltaTime);
            }
        }
    }

    void ScriptLifecycle::DestroyAll() {
        std::vector<std::string> instanceNames;
        for (auto& [name, instance] : m_Instances) {
            instanceNames.push_back(name);
        }
        
        for (const auto& name : instanceNames) {
            DestroyInstance(name);
        }
    }

    void ScriptLifecycle::BroadcastEvent(const std::string& eventName, sol::object data) {
        for (auto& [name, instance] : m_Instances) {
            auto it = instance.eventHandlers.find(eventName);
            if (it != instance.eventHandlers.end() && it->second.valid()) {
                CallScriptFunction(instance, it->second, data);
            }
        }
    }

    void ScriptLifecycle::SendEventTo(const std::string& instanceName, 
                                      const std::string& eventName, 
                                      sol::object data) {
        auto it = m_Instances.find(instanceName);
        if (it == m_Instances.end()) return;
        
        auto& instance = it->second;
        auto handler = instance.eventHandlers.find(eventName);
        if (handler != instance.eventHandlers.end() && handler->second.valid()) {
            CallScriptFunction(instance, handler->second, data);
        }
    }

    void ScriptLifecycle::StartCoroutine(const std::string& instanceName, 
                                         sol::protected_function coroutine) {
        auto instance = GetInstance(instanceName);
        if (!instance) {
            SAGE_ERROR("Cannot start coroutine: instance {0} not found", instanceName);
            return;
        }
        
        sol::thread thread = sol::thread::create(m_Lua);
        sol::coroutine coro(thread.state(), coroutine);
        
        CoroutineState state;
        state.instanceName = instanceName;
        state.thread = thread;
        state.coroutine = coro;
        
        m_Coroutines.push_back(state);
    }

    void ScriptLifecycle::UpdateCoroutines(float deltaTime) {
        for (auto it = m_Coroutines.begin(); it != m_Coroutines.end();) {
            if (it->finished) {
                it = m_Coroutines.erase(it);
                continue;
            }
            
            auto result = it->coroutine(deltaTime);
            
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("Coroutine error: {0}", err.what());
                it = m_Coroutines.erase(it);
            } else if (result.get_type() == sol::type::none || 
                       (result.get_type() == sol::type::boolean && !result.get<bool>())) {
                it->finished = true;
                ++it;
            } else {
                ++it;
            }
        }
    }

    ScriptLifecycle::ScriptInstance* ScriptLifecycle::GetInstance(const std::string& name) {
        auto it = m_Instances.find(name);
        return it != m_Instances.end() ? &it->second : nullptr;
    }

    sol::table ScriptLifecycle::GetInstanceTable(const std::string& name) {
        auto instance = GetInstance(name);
        return instance ? instance->instance : sol::nil;
    }

    void ScriptLifecycle::CallScriptFunction(ScriptInstance& instance, 
                                             sol::protected_function& func, 
                                             sol::variadic_args args) {
        try {
            auto result = func(instance.instance, args);
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("Script error in {0}: {1}", instance.name, err.what());
            }
        }
        catch (const sol::error& e) {
            SAGE_ERROR("Exception in script {0}: {1}", instance.name, e.what());
        }
    }

    // ============================================================================
    // GameStateManager Implementation
    // ============================================================================

    void GameStateManager::RegisterState(const std::string& name, sol::table stateTable) {
        GameState state;
        state.name = name;
        state.onEnter = stateTable["OnEnter"];
        state.onExit = stateTable["OnExit"];
        state.onUpdate = stateTable["OnUpdate"];
        state.allowPause = stateTable.get_or("allowPause", true);
        
        m_States[name] = state;
        SAGE_INFO("Registered game state: {0}", name);
    }

    void GameStateManager::PushState(const std::string& name) {
        auto it = m_States.find(name);
        if (it == m_States.end()) {
            SAGE_ERROR("State not found: {0}", name);
            return;
        }
        
        m_StateStack.push_back(name);
        m_CurrentState = name;
        
        if (it->second.onEnter.valid()) {
            auto result = it->second.onEnter();
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("State {0} OnEnter error: {1}", name, err.what());
            }
        }
        
        SAGE_INFO("Pushed state: {0}", name);
    }

    void GameStateManager::PopState() {
        if (m_StateStack.empty()) return;
        
        std::string oldState = m_StateStack.back();
        auto it = m_States.find(oldState);
        
        if (it != m_States.end() && it->second.onExit.valid()) {
            auto result = it->second.onExit();
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("State {0} OnExit error: {1}", oldState, err.what());
            }
        }
        
        m_StateStack.pop_back();
        m_CurrentState = m_StateStack.empty() ? "" : m_StateStack.back();
        
        SAGE_INFO("Popped state: {0}, current: {1}", oldState, m_CurrentState);
    }

    void GameStateManager::ChangeState(const std::string& name) {
        if (!m_StateStack.empty()) {
            PopState();
        }
        PushState(name);
    }

    void GameStateManager::UpdateCurrentState(float deltaTime) {
        if (m_CurrentState.empty()) return;
        
        auto it = m_States.find(m_CurrentState);
        if (it != m_States.end() && it->second.onUpdate.valid()) {
            auto result = it->second.onUpdate(deltaTime);
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("State {0} OnUpdate error: {1}", m_CurrentState, err.what());
            }
        }
    }

    std::string GameStateManager::GetCurrentState() const {
        return m_CurrentState;
    }

    bool GameStateManager::IsInState(const std::string& name) const {
        return m_CurrentState == name;
    }

    // ============================================================================
    // SceneManager Implementation
    // ============================================================================

    void SceneManager::RegisterScene(const std::string& name, sol::table sceneTable) {
        Scene scene;
        scene.name = name;
        scene.sceneTable = sceneTable;
        scene.onLoad = sceneTable["OnLoad"];
        scene.onUnload = sceneTable["OnUnload"];
        scene.onActivate = sceneTable["OnActivate"];
        scene.onDeactivate = sceneTable["OnDeactivate"];
        
        m_Scenes[name] = scene;
        SAGE_INFO("Registered scene: {0}", name);
    }

    void SceneManager::LoadScene(const std::string& name) {
        auto it = m_Scenes.find(name);
        if (it == m_Scenes.end()) {
            SAGE_ERROR("Scene not found: {0}", name);
            return;
        }
        
        auto& scene = it->second;
        if (!scene.loaded && scene.onLoad.valid()) {
            auto result = scene.onLoad();
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("Scene {0} OnLoad error: {1}", name, err.what());
                return;
            }
            scene.loaded = true;
        }
        
        SAGE_INFO("Loaded scene: {0}", name);
    }

    void SceneManager::UnloadScene(const std::string& name) {
        auto it = m_Scenes.find(name);
        if (it == m_Scenes.end() || !it->second.loaded) return;
        
        auto& scene = it->second;
        if (scene.onUnload.valid()) {
            auto result = scene.onUnload();
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("Scene {0} OnUnload error: {1}", name, err.what());
            }
        }
        
        scene.loaded = false;
        SAGE_INFO("Unloaded scene: {0}", name);
    }

    void SceneManager::ActivateScene(const std::string& name) {
        // Deactivate current scene
        if (!m_CurrentSceneName.empty()) {
            auto it = m_Scenes.find(m_CurrentSceneName);
            if (it != m_Scenes.end() && it->second.onDeactivate.valid()) {
                it->second.onDeactivate();
            }
        }
        
        // Load if not loaded
        LoadScene(name);
        
        // Activate new scene
        auto it = m_Scenes.find(name);
        if (it != m_Scenes.end() && it->second.onActivate.valid()) {
            auto result = it->second.onActivate();
            if (!result.valid()) {
                sol::error err = result;
                SAGE_ERROR("Scene {0} OnActivate error: {1}", name, err.what());
                return;
            }
        }
        
        m_CurrentSceneName = name;
        SAGE_INFO("Activated scene: {0}", name);
    }

    SceneManager::Scene* SceneManager::GetCurrentScene() {
        if (m_CurrentSceneName.empty()) return nullptr;
        auto it = m_Scenes.find(m_CurrentSceneName);
        return it != m_Scenes.end() ? &it->second : nullptr;
    }

} // namespace Scripting
} // namespace SAGE
