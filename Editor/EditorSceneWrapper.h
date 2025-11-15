#pragma once

#include <Core/Scene.h>

namespace SAGE {
namespace Editor {

// Wrapper scene to satisfy Application::Run() requirement
// The actual editor logic runs directly in EditorApplication, not through Scene system
class EditorSceneWrapper : public Scene {
public:
    EditorSceneWrapper() : Scene("Editor Scene") {}
    virtual ~EditorSceneWrapper() = default;

    // Editor doesn't use the standard Scene lifecycle
    void OnEnter(const TransitionContext& /*context*/) override {}
    void OnExit() override {}
    void OnUpdate(float /*deltaTime*/) override {}
    void OnRender() override {}
    void OnEvent(Event& /*event*/) override {}
};

} // namespace Editor
} // namespace SAGE
