#include "SceneStack.h"

#include "Event.h"
#include "GameObject.h"
#include <algorithm>


namespace SAGE {

    SceneStack::~SceneStack() {
        Clear();
    }

    void SceneStack::PushScene(Scope<Scene> scene,
                               SceneParameters params,
                               bool stateRestored) {
        if (!scene) {
            return;
        }

        std::lock_guard<std::recursive_mutex> lock(m_Mutex);

        SceneEntry* previousEntry = GetTopEntryUnsafe();
        Scene* previous = previousEntry ? previousEntry->Instance.get() : nullptr;
        if (previousEntry && previous) {
            previous->OnPause();
            previousEntry->Active = false;
        }

        Scene::TransitionContext context{ params, previous, stateRestored };
        Scene* entering = scene.get();

        SceneEntry entry;
        entry.Instance = std::move(scene);
        entry.LastEnterParams = params;
        entry.LastStateRestored = stateRestored;
        entry.Active = true;

        m_Scenes.emplace_back(std::move(entry));

        if (entering) {
            entering->OnEnter(context);
        }
    }

    void SceneStack::PopScene(Scene* scene,
                              SceneParameters resumeParams,
                              bool stateRestored) {
        if (!scene) {
            return;
        }

        std::lock_guard<std::recursive_mutex> lock(m_Mutex);

        auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [scene](const SceneEntry& entry) {
            return entry.Instance.get() == scene;
        });
        if (it == m_Scenes.end()) {
            return;
        }

        SceneEntry removedEntry = std::move(*it);
        m_Scenes.erase(it);
        Scene* removedRaw = removedEntry.Instance.get();

        if (removedRaw) {
            removedRaw->OnExit();
            GameObject::DestroySceneObjects(removedRaw);
        }

        SceneEntry* resumeEntry = GetTopEntryUnsafe();
        if (resumeEntry && resumeEntry->Instance) {
            resumeEntry->Active = true;
            resumeEntry->LastResumeParams = std::move(resumeParams);
            resumeEntry->LastStateRestored = stateRestored;
            Scene::TransitionContext resumeContext{ resumeEntry->LastResumeParams,
                                                    removedRaw,
                                                    stateRestored };
            resumeEntry->Instance->OnResume(resumeContext);
        }
    }

    void SceneStack::PopTopScene(SceneParameters resumeParams, bool stateRestored) {
        Scene* top = GetTopScene();
        if (!top) {
            return;
        }
        PopScene(top, std::move(resumeParams), stateRestored);
    }

    void SceneStack::ReplaceTopScene(Scope<Scene> scene,
                                     SceneParameters params,
                                     bool stateRestored) {
        if (!scene) {
            return;
        }

        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        if (m_Scenes.empty()) {
            PushScene(std::move(scene), std::move(params), stateRestored);
            return;
        }

        SceneEntry& topEntry = m_Scenes.back();
        Scene* previous = nullptr;
        if (m_Scenes.size() >= 2) {
            previous = m_Scenes[m_Scenes.size() - 2].Instance.get();
        }

        Scope<Scene> outgoing = std::move(topEntry.Instance);
        if (outgoing) {
            outgoing->OnExit();
            GameObject::DestroySceneObjects(outgoing.get());
        }

        topEntry.Instance = std::move(scene);
        topEntry.LastEnterParams = params;
        topEntry.LastResumeParams = SceneParameters{};
        topEntry.LastStateRestored = stateRestored;
        topEntry.Active = true;

        if (topEntry.Instance) {
            Scene::TransitionContext context{ topEntry.LastEnterParams, previous, stateRestored };
            topEntry.Instance->OnEnter(context);
        }
    }

    void SceneStack::Clear() {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        while (!m_Scenes.empty()) {
            SceneEntry entry = std::move(m_Scenes.back());
            m_Scenes.pop_back();
            if (entry.Instance) {
                entry.Instance->OnExit();
                GameObject::DestroySceneObjects(entry.Instance.get());
            }
        }
    }

    void SceneStack::OnUpdate(float deltaTime) {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        SceneEntry* entry = GetTopEntryUnsafe();
        if (entry && entry->Instance) {
            // Сначала пользовательская логика
            entry->Instance->OnUpdate(deltaTime);
            // Затем обновление ECS систем сцены
            entry->Instance->GetECS().Update(deltaTime);
        }
    }

    void SceneStack::OnFixedUpdate(float fixedDeltaTime) {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        SceneEntry* entry = GetTopEntryUnsafe();
        if (entry && entry->Instance) {
            // Сначала пользовательская Fixed Update логика
            entry->Instance->OnFixedUpdate(fixedDeltaTime);
            // Затем физика и другие fixed системы ECS
            entry->Instance->GetECS().FixedUpdate(fixedDeltaTime);
        }
    }

    void SceneStack::OnRender() {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto& entry : m_Scenes) {
            if (entry.Instance) {
                entry.Instance->OnRender();
            }
        }
    }

    void SceneStack::OnEvent(Event& event) {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto it = m_Scenes.rbegin(); it != m_Scenes.rend(); ++it) {
            SceneEntry& entry = *it;
            if (!entry.Instance) {
                continue;
            }
            entry.Instance->OnEvent(event);
            if (event.Handled) {
                break;
            }
        }
    }

    bool SceneStack::Empty() const {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        return m_Scenes.empty();
    }

    std::size_t SceneStack::Size() const {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        return m_Scenes.size();
    }

    Scene* SceneStack::GetTopScene() {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        return GetTopSceneUnsafe();
    }

    const Scene* SceneStack::GetTopScene() const {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        return GetTopSceneUnsafe();
    }

    Scene* SceneStack::GetSceneBelowTop() {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        if (m_Scenes.size() < 2) {
            return nullptr;
        }
        return m_Scenes[m_Scenes.size() - 2].Instance.get();
    }

    const Scene* SceneStack::GetSceneBelowTop() const {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        if (m_Scenes.size() < 2) {
            return nullptr;
        }
        return m_Scenes[m_Scenes.size() - 2].Instance.get();
    }

    Scene* SceneStack::FindScene(const std::string& name) {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto& entry : m_Scenes) {
            if (entry.Instance && entry.Instance->GetName() == name) {
                return entry.Instance.get();
            }
        }
        return nullptr;
    }

    Scene* SceneStack::FindSceneByID(SceneID id) {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto& entry : m_Scenes) {
            if (entry.Instance && entry.Instance->GetID() == id) {
                return entry.Instance.get();
            }
        }
        return nullptr;
    }

    SceneStack::SceneEntry* SceneStack::GetTopEntryUnsafe() {
        if (m_Scenes.empty()) {
            return nullptr;
        }
        return &m_Scenes.back();
    }

    const SceneStack::SceneEntry* SceneStack::GetTopEntryUnsafe() const {
        if (m_Scenes.empty()) {
            return nullptr;
        }
        return &m_Scenes.back();
    }

    Scene* SceneStack::GetTopSceneUnsafe() const {
        const SceneEntry* entry = GetTopEntryUnsafe();
        return entry ? entry->Instance.get() : nullptr;
    }

} // namespace SAGE
