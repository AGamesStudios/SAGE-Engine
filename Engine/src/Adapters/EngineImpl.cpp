#include "EngineImpl.h"
#include "SceneImpl.h"
#include <Core/Logger.h>
#include <algorithm>

namespace SAGE {

// Factory implementation
IEngine* CreateEngine() {
    return new Internal::EngineImpl();
}

void DestroyEngine(IEngine* engine) {
    delete engine;
}

namespace Internal {

EngineImpl::EngineImpl() {
    SAGE_INFO("Creating SAGE Engine...");
}

EngineImpl::~EngineImpl() {
    Shutdown();
    SAGE_INFO("SAGE Engine destroyed");
}

bool EngineImpl::Initialize(const EngineConfig& config) {
    if (m_Initialized) {
        SAGE_WARN("Engine already initialized");
        return true;
    }
    
    SAGE_INFO("Initializing SAGE Engine...");
    SAGE_INFO("  Title: {}", config.windowTitle);
    SAGE_INFO("  Resolution: {}x{}", config.windowWidth, config.windowHeight);
    
    // Initialize Application (creates window, input, etc.)
    m_Application = std::make_unique<Application>(config.windowTitle);
    
    // Initialize adapters for public API access
    m_RendererAdapter = std::make_unique<RendererAdapter>();
    m_ResourceManagerAdapter = std::make_unique<ResourceManagerAdapter>();
    
    // Initialize SceneManager
    m_SceneManager = std::make_unique<SceneManager>();
    
    m_Initialized = true;
    SAGE_INFO("SAGE Engine initialized successfully");
    
    return true;
}

void EngineImpl::Shutdown() {
    if (!m_Initialized) return;
    
    SAGE_INFO("Shutting down SAGE Engine...");
    
    // Destroy all scenes
    m_Scenes.clear();
    m_ActiveScene = nullptr;
    
    // Shutdown subsystems in reverse order
    m_SceneManager.reset();
    m_ResourceManagerAdapter.reset();
    m_RendererAdapter.reset();
    m_Application.reset();
    
    m_Initialized = false;
    SAGE_INFO("SAGE Engine shutdown complete");
}

bool EngineImpl::IsRunning() const {
    return m_Application && m_Application->IsRunning();
}

void EngineImpl::Run() {
    if (!m_Initialized) {
        SAGE_ERROR("Cannot run engine - not initialized!");
        return;
    }
    
    if (m_Application) {
        m_Application->Run();
    }
}

void EngineImpl::Update(float deltaTime) {
    m_DeltaTime = deltaTime;
    m_Time += deltaTime;
    
    if (m_ActiveScene) {
        m_ActiveScene->Update(deltaTime);
    }
}

void EngineImpl::Render() {
    if (m_RendererAdapter) {
        m_RendererAdapter->Clear({0.2f, 0.2f, 0.2f, 1.0f});
    }
    
    if (m_ActiveScene) {
        m_ActiveScene->Render();
    }
}

IRenderer* EngineImpl::GetRenderer() {
    return m_RendererAdapter.get();
}

IResourceManager* EngineImpl::GetResourceManager() {
    return m_ResourceManagerAdapter.get();
}

IScene* EngineImpl::CreateScene(const std::string& name) {
    auto scene = std::make_unique<SceneImpl>(this, name);
    IScene* scenePtr = scene.get();
    m_Scenes.push_back(std::move(scene));
    
    SAGE_INFO("Created scene: {}", name);
    return scenePtr;
}

void EngineImpl::DestroyScene(IScene* scene) {
    if (!scene) return;
    
    // Don't destroy active scene
    if (scene == m_ActiveScene) {
        SAGE_WARN("Cannot destroy active scene. Set another scene as active first.");
        return;
    }
    
    auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [scene](const std::unique_ptr<SceneImpl>& s) { return s.get() == scene; });
    
    if (it != m_Scenes.end()) {
        SAGE_INFO("Destroyed scene: {}", (*it)->GetName());
        m_Scenes.erase(it);
    }
}

void EngineImpl::SetActiveScene(IScene* scene) {
    m_ActiveScene = static_cast<SceneImpl*>(scene);
    
    if (m_ActiveScene) {
        SAGE_INFO("Set active scene: {}", m_ActiveScene->GetName());
    }
}

IScene* EngineImpl::GetActiveScene() {
    return m_ActiveScene;
}

float EngineImpl::GetDeltaTime() const {
    return m_DeltaTime;
}

float EngineImpl::GetTime() const {
    return m_Time;
}

} // namespace Internal
} // namespace SAGE
