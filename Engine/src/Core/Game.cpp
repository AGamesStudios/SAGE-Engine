#include "SAGE/Core/Game.h"
#include "SAGE/Time.h"
#include "SAGE/Log.h"
#include "SAGE/Graphics/Renderer.h"

namespace SAGE {

Game::Game(const ApplicationConfig& config)
    : Application(config)
{
}

Game::~Game() = default;

void Game::OnInit() {
    SAGE_INFO("Game: Initializing...");
    
    InitializeRendering();
    OnGameInit();
    
    SAGE_INFO("Game: Initialization complete");
}

void Game::InitializeRendering() {
    // Get window size
    int width = 1280, height = 720;
    GetWindow().GetFramebufferSize(width, height);

    // Enforce aspect ratio based on initial size
    GetWindow().SetAspectRatio(width, height);

    // Create camera
    m_Camera = std::make_unique<Camera2D>(static_cast<float>(width), static_cast<float>(height));
    m_Camera->SetPosition({static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f});
    m_Camera->SetZoom(1.0f);

    // Set projection matrix for immediate rendering
    Renderer::SetCamera(*m_Camera);

    SAGE_INFO("Game: Rendering initialized ({} x {})", width, height);
}

void Game::OnUpdate(double deltaTime) {
    m_DeltaTime = static_cast<float>(deltaTime);
    
    // Fixed Update Loop
    static float accumulator = 0.0f;
    accumulator += m_DeltaTime;
    const float fixedStep = 1.0f / 60.0f;
    
    while (accumulator >= fixedStep) {
        SceneManager::Get().FixedUpdate(fixedStep);
        accumulator -= fixedStep;
    }

    // Update camera
    UpdateCamera();
    
    // Update scenes
    SceneManager::Get().Update(m_DeltaTime);
    
    // Game-specific update
    OnGameUpdate(m_DeltaTime);
    
    // Rendering
    if (m_Camera) {
        Renderer::SetCamera(*m_Camera);
    }

    Renderer::BeginFrame();
    Renderer::Clear();
    Renderer::BeginSpriteBatch(m_Camera.get());

    SceneManager::Get().Render();
    OnGameRender();

    Renderer::FlushSpriteBatch();
    Renderer::EndFrame();

#if defined(SAGE_DEBUG)
    static float s_RenderLogTimer = 0.0f;
    s_RenderLogTimer += m_DeltaTime;
    if (s_RenderLogTimer >= 1.0f) {
        const auto& stats = Renderer::GetStats();
        SAGE_TRACE("Render stats - DrawCalls: {}, Vertices: {}, Triangles: {}", stats.drawCalls, stats.vertices, stats.triangles);
        s_RenderLogTimer = 0.0f;
    }
#endif
}

void Game::OnShutdown() {
    SAGE_INFO("Game: Shutting down...");
    
    OnGameShutdown();
    
    SAGE_INFO("Game: Shutdown complete");
}

void Game::OnResize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    SAGE_INFO("Game: Window resized to {} x {}", width, height);
    
    // Update camera viewport
    if (m_Camera) {
        m_Camera->SetViewportSize(static_cast<float>(width), static_cast<float>(height));
        Renderer::SetCamera(*m_Camera);
    }

}

void Game::UpdateCamera() {
    if (!m_Camera) {
        return;
    }

    m_Camera->Update(m_DeltaTime);
}

} // namespace SAGE
