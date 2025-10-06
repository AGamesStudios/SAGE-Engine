#pragma once

#include "SceneState.h"

#include <string>

namespace SAGE {

    class Event;

    class Scene {
    public:
        explicit Scene(std::string name = "Scene")
            : m_Name(std::move(name)) {}
        virtual ~Scene() = default;

        const std::string& GetName() const { return m_Name; }
        void SetName(std::string name) { m_Name = std::move(name); }

        virtual void OnPause() {}
        virtual void OnResume() {}

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(float /*deltaTime*/) {}
        virtual void OnRender() {}
        virtual void OnEvent(Event& /*event*/) {}

        virtual void SaveState(SceneState& /*outState*/) const {}
        virtual void LoadState(const SceneState& /*state*/) {}

    protected:
        std::string m_Name;
    };

} // namespace SAGE
