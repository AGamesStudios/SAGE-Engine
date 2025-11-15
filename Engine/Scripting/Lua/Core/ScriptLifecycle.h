#pragma once

#include "Core/Logger.h"
#include "LuaForward.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

class ScriptLifecycle {
public:
    struct ScriptInstance {
        std::string name;
        sol::table instance;
        bool started = false;
        bool enabled = true;

        sol::protected_function onLoad;
        sol::protected_function onStart;
        sol::protected_function onUpdate;
        sol::protected_function onFixedUpdate;
        sol::protected_function onDestroy;

        std::unordered_map<std::string, sol::protected_function> eventHandlers;
    };

    explicit ScriptLifecycle(sol::state& lua) : m_Lua(lua) {}

    bool LoadScript(const std::string& name, const std::string& filepath);
    ScriptInstance* CreateInstance(const std::string& scriptName, const std::string& instanceName);
    void DestroyInstance(const std::string& instanceName);

    void StartAll();
    void UpdateAll(float deltaTime);
    void FixedUpdateAll(float fixedDeltaTime);
    void DestroyAll();

    void BroadcastEvent(const std::string& eventName, sol::object data = sol::nil);
    void SendEventTo(const std::string& instanceName, const std::string& eventName, sol::object data = sol::nil);

    void StartCoroutine(const std::string& instanceName, sol::protected_function coroutine);
    void UpdateCoroutines(float deltaTime);

    ScriptInstance* GetInstance(const std::string& name);
    sol::table GetInstanceTable(const std::string& name);

private:
    sol::state& m_Lua;
    std::unordered_map<std::string, sol::table> m_ScriptTemplates;
    std::unordered_map<std::string, ScriptInstance> m_Instances;

    struct CoroutineState {
        std::string instanceName;
        sol::thread thread;
        sol::coroutine coroutine;
        bool finished = false;
    };
    std::vector<CoroutineState> m_Coroutines;

    void CallScriptFunction(ScriptInstance& instance, sol::protected_function& func, sol::variadic_args args = sol::nil);
};

class GameStateManager {
public:
    using StateEnterCallback = std::function<void()>;
    using StateExitCallback = std::function<void()>;
    using StateUpdateCallback = std::function<void(float)>;

    struct GameState {
        std::string name;
        sol::protected_function onEnter;
        sol::protected_function onExit;
        sol::protected_function onUpdate;
        bool allowPause = true;
    };

    explicit GameStateManager(sol::state& lua) : m_Lua(lua) {}

    void RegisterState(const std::string& name, sol::table stateTable);
    void PushState(const std::string& name);
    void PopState();
    void ChangeState(const std::string& name);
    void UpdateCurrentState(float deltaTime);

    std::string GetCurrentState() const;
    bool IsInState(const std::string& name) const;

private:
    sol::state& m_Lua;
    std::unordered_map<std::string, GameState> m_States;
    std::vector<std::string> m_StateStack;
    std::string m_CurrentState;
};

class SceneManager {
public:
    struct Scene {
        std::string name;
        sol::table sceneTable;
        sol::protected_function onLoad;
        sol::protected_function onUnload;
        sol::protected_function onActivate;
        sol::protected_function onDeactivate;
        bool loaded = false;
    };

    explicit SceneManager(sol::state& lua) : m_Lua(lua) {}

    void RegisterScene(const std::string& name, sol::table sceneTable);
    void LoadScene(const std::string& name);
    void UnloadScene(const std::string& name);
    void ActivateScene(const std::string& name);

    Scene* GetCurrentScene();
    const std::string& GetCurrentSceneName() const { return m_CurrentSceneName; }

private:
    sol::state& m_Lua;
    std::unordered_map<std::string, Scene> m_Scenes;
    std::string m_CurrentSceneName;
};

#else

class ScriptLifecycle {
public:
    struct ScriptInstance {
        std::string name;
        bool started = false;
        bool enabled = true;
    };

    explicit ScriptLifecycle(sol::state&) {}

    bool LoadScript(const std::string&, const std::string&) { return false; }
    ScriptInstance* CreateInstance(const std::string&, const std::string&) { return nullptr; }
    void DestroyInstance(const std::string&) {}

    void StartAll() {}
    void UpdateAll(float) {}
    void FixedUpdateAll(float) {}
    void DestroyAll() {}

    void BroadcastEvent(const std::string&, sol::object = sol::nil) {}
    void SendEventTo(const std::string&, const std::string&, sol::object = sol::nil) {}

    void StartCoroutine(const std::string&, sol::protected_function) {}
    void UpdateCoroutines(float) {}

    ScriptInstance* GetInstance(const std::string&) { return nullptr; }
    sol::table GetInstanceTable(const std::string&) { return {}; }
};

class GameStateManager {
public:
    explicit GameStateManager(sol::state&) {}

    void RegisterState(const std::string&, sol::table) {}
    void PushState(const std::string&) {}
    void PopState() {}
    void ChangeState(const std::string&) {}
    void UpdateCurrentState(float) {}

    std::string GetCurrentState() const { return {}; }
    bool IsInState(const std::string&) const { return false; }
};

class SceneManager {
public:
    explicit SceneManager(sol::state&) {}

    void RegisterScene(const std::string&, sol::table) {}
    void LoadScene(const std::string&) {}
    void UnloadScene(const std::string&) {}
    void ActivateScene(const std::string&) {}

    struct Scene { std::string name; };
    Scene* GetCurrentScene() { return nullptr; }
    const std::string& GetCurrentSceneName() const { return m_CurrentSceneName; }

private:
    std::string m_CurrentSceneName;
};

#endif

} // namespace Scripting
} // namespace SAGE
