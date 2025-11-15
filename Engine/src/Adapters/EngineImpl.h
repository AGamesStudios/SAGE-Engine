#pragma once

#include <SAGE/IEngine.h>
#include <Core/Application.h>
#include <Core/SceneManager.h>
#include "RendererAdapter.h"
#include "ResourceManagerAdapter.h"
#include <memory>
#include <vector>

namespace SAGE {
namespace Internal {

class SceneImpl;

/**
 * @brief Internal implementation of IEngine interface
 * 
 * Bridges public API to internal Engine components.
 */
class EngineImpl : public IEngine {
public:
    EngineImpl();
    ~EngineImpl() override;
    
    // IEngine interface
    bool Initialize(const EngineConfig& config) override;
    void Shutdown() override;
    bool IsRunning() const override;
    
    void Run() override;
    void Update(float deltaTime) override;
    void Render() override;
    
    IRenderer* GetRenderer() override;
    IResourceManager* GetResourceManager() override;
    
    IScene* CreateScene(const std::string& name) override;
    void DestroyScene(IScene* scene) override;
    void SetActiveScene(IScene* scene) override;
    IScene* GetActiveScene() override;
    
    float GetDeltaTime() const override;
    float GetTime() const override;
    
private:
    std::unique_ptr<Application> m_Application;
    std::unique_ptr<RendererAdapter> m_RendererAdapter;
    std::unique_ptr<ResourceManagerAdapter> m_ResourceManagerAdapter;
    std::unique_ptr<SceneManager> m_SceneManager;
    
    std::vector<std::unique_ptr<SceneImpl>> m_Scenes;
    SceneImpl* m_ActiveScene = nullptr;
    
    float m_DeltaTime = 0.0f;
    float m_Time = 0.0f;
    bool m_Initialized = false;
};

} // namespace Internal
} // namespace SAGE
