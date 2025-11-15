#include "Graphics/Core/RenderContext.h"
#include "Graphics/Rendering/StateManagement/RenderStateManager.h"
#include "Graphics/Rendering/Batching/BatchRenderer.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLSceneRenderer.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLRenderBackend.h"
#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"

#include <memory>

namespace SAGE::Graphics {

struct RenderContext::Impl {
    bool initialized = false;
    
    // Subsystems (owned)
    std::unique_ptr<StateManagement::RenderStateManager> stateManager;
    std::unique_ptr<Batching::BatchRenderer> batchRenderer;
    std::unique_ptr<ShaderManager> shaderManager;
    std::unique_ptr<IRenderBackend> backend;
    std::shared_ptr<ISceneRenderer> sceneRenderer;  // shared_ptr for registry compatibility
    
    RenderSystemConfig config;
};

RenderContext::RenderContext()
    : m_Impl(std::make_unique<Impl>())
{
}

RenderContext::~RenderContext()
{
    Shutdown();
}

RenderContext::RenderContext(RenderContext&&) noexcept = default;
RenderContext& RenderContext::operator=(RenderContext&&) noexcept = default;

void RenderContext::Init(const RenderSystemConfig& config)
{
    if (m_Impl->initialized) {
        SAGE_WARNING("RenderContext already initialized");
        return;
    }
    
    m_Impl->config = config;
    
    // Create backend
    m_Impl->backend = std::make_unique<OpenGLRenderBackend>();
    m_Impl->backend->Init();
    m_Impl->backend->Configure(config);
    
    // Create scene renderer
    m_Impl->sceneRenderer = std::make_shared<OpenGLSceneRenderer>(m_Impl->backend.get());
    m_Impl->sceneRenderer->Init();
    
    // Register scene renderer with global registry so Renderer::GetSceneRenderer() works
    Renderer::GetRegistry().SetSceneRenderer(m_Impl->sceneRenderer);
    
    // Create state manager (instance-based, not static)
    m_Impl->stateManager = std::make_unique<StateManagement::RenderStateManager>();
    
    // Create batch renderer
    m_Impl->batchRenderer = std::make_unique<Batching::BatchRenderer>();
    m_Impl->batchRenderer->Initialize();
    
    // Create shader manager (instance-based, not static)
    m_Impl->shaderManager = std::make_unique<ShaderManager>();
    
    m_Impl->initialized = true;
    SAGE_INFO("RenderContext initialized");
}

void RenderContext::Shutdown()
{
    if (!m_Impl->initialized) {
        return;
    }
    
    // Unregister scene renderer from global registry
    Renderer::GetRegistry().SetSceneRenderer(nullptr);
    
    // Shutdown in reverse order
    if (m_Impl->shaderManager) {
        // ShaderManager cleanup handled by destructor
    }
    
    if (m_Impl->batchRenderer) {
        m_Impl->batchRenderer->Shutdown();
    }
    
    if (m_Impl->stateManager) {
        // StateManager cleanup handled by destructor
    }
    
    if (m_Impl->sceneRenderer) {
        m_Impl->sceneRenderer->Shutdown();
    }
    
    if (m_Impl->backend) {
        m_Impl->backend->Shutdown();
    }
    
    m_Impl->initialized = false;
    SAGE_INFO("RenderContext shutdown");
}

bool RenderContext::IsInitialized() const
{
    return m_Impl->initialized;
}

void RenderContext::Update(float deltaTime)
{
    if (!m_Impl->initialized) {
        return;
    }
    
    if (m_Impl->sceneRenderer) {
        m_Impl->sceneRenderer->Update(deltaTime);
    }
}

StateManagement::RenderStateManager& RenderContext::GetStateManager()
{
    return *m_Impl->stateManager;
}

const StateManagement::RenderStateManager& RenderContext::GetStateManager() const
{
    return *m_Impl->stateManager;
}

Batching::BatchRenderer& RenderContext::GetBatchRenderer()
{
    return *m_Impl->batchRenderer;
}

const Batching::BatchRenderer& RenderContext::GetBatchRenderer() const
{
    return *m_Impl->batchRenderer;
}

ShaderManager& RenderContext::GetShaderManager()
{
    return *m_Impl->shaderManager;
}

const ShaderManager& RenderContext::GetShaderManager() const
{
    return *m_Impl->shaderManager;
}

ISceneRenderer& RenderContext::GetSceneRenderer()
{
    return *m_Impl->sceneRenderer;
}

const ISceneRenderer& RenderContext::GetSceneRenderer() const
{
    return *m_Impl->sceneRenderer;
}

std::shared_ptr<ISceneRenderer> RenderContext::GetSceneRendererShared() const {
    return m_Impl->sceneRenderer;
}

IRenderBackend& RenderContext::GetBackend()
{
    return *m_Impl->backend;
}

const IRenderBackend& RenderContext::GetBackend() const
{
    return *m_Impl->backend;
}

} // namespace SAGE::Graphics
