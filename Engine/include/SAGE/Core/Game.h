#pragma once

#include "SAGE/Application.h"
#include "SAGE/Core/SceneManager.h"
#include "SAGE/Graphics/Camera2D.h"
#include <memory>

namespace SAGE {

// Game Application - higher level than Application
// Provides scene management, rendering pipeline, and game loop
class Game : public Application {
public:
    explicit Game(const ApplicationConfig& config = {});
    virtual ~Game() override;

    // Game lifecycle (override these in your game)
    virtual void OnGameInit() {}
    virtual void OnGameUpdate([[maybe_unused]] float deltaTime) {}
    virtual void OnGameRender() {}
    virtual void OnGameShutdown() {}

    // Scene management
    SceneManager& GetSceneManager() { return SceneManager::Get(); }
    
    // Camera access
    Camera2D& GetCamera() { return *m_Camera; }
    const Camera2D& GetCamera() const { return *m_Camera; }
    
protected:
    // Application overrides
    void OnInit() override;
    void OnUpdate(double deltaTime) override;
    void OnShutdown() override;
    void OnResize(int width, int height) override;

private:
    void InitializeRendering();
    void UpdateCamera();

    std::unique_ptr<Camera2D> m_Camera;
    float m_DeltaTime = 0.0f;
};

} // namespace SAGE
