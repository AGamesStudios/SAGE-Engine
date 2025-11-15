#include "Graphics/API/Renderer.h"
#include "Graphics/API/RenderSystemRegistry.h"
#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/API/RenderContext.hpp"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/ViewportManager.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLRenderBackend.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLSceneRenderer.h"
// SoftwareRenderBackend REMOVED - SAGEGraphics deleted
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Graphics/Rendering/Graph/RenderGraph.h"
#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Rendering/Graph/Passes/ClearPass.h"
#include "Graphics/Rendering/Graph/Passes/GeometryPass.h"
#include "Graphics/Rendering/Graph/Passes/BatchSubmitPass.h"
#include "Graphics/Rendering/Graph/Passes/BlurPass.h"
#include "Graphics/Rendering/Graph/Passes/CompositePass.h"
#include "Graphics/Rendering/Graph/Passes/ExposurePass.h"
#include "Core/Core.h"
#include "Core/Logger.h"

#include <iostream>

#include <functional>
#include <memory>
#include <utility>

namespace SAGE {
namespace {

struct RendererState {
    std::shared_ptr<IRenderBackend> backend;
    Graphics::RenderSystemRegistry registry;
    Graphics::RenderSystemConfig config{};
    std::unique_ptr<Graphics::RenderContext> renderContext;
    std::unique_ptr<ViewportManager> viewportManager;
    bool defaultFactoriesRegistered = false;
    Vector2 contentScale{1.0f,1.0f};
    // Render graph
    std::unique_ptr<RenderGraph> graph;
    // Camera ownership now centralized
    Camera2D camera; // primary scene camera
    std::function<void()> uiRenderCallback; // deferred UI pass (e.g., ImGui)
} g_RS;

IRenderBackend* TryGetBackendInternal() {
    return g_RS.backend ? g_RS.backend.get() : nullptr;
}

IRenderBackend& RequireBackend() {
    SAGE_ASSERT(g_RS.backend != nullptr, "Renderer backend accessed before initialization");
    return *g_RS.backend;
}

IRenderBackend* GetBackend() { // internal safe raw accessor
    return g_RS.backend.get();
}

void EnsureBackendFactories() {
    if (g_RS.defaultFactoriesRegistered) {
        return;
    }

        g_RS.registry.RegisterBackendFactory(
        Graphics::BackendType::OpenGL,
        []() {
            return std::make_unique<Graphics::OpenGLRenderBackend>();
        });

    // Software backend REMOVED - SAGEGraphics implementation deleted, OpenGL-only now

    g_RS.defaultFactoriesRegistered = true;
}

} // namespace

void Renderer::Init() {
    Init(Graphics::RenderSystemConfig{});
}

void Renderer::Init(const Graphics::RenderSystemConfig& config) {
    EnsureBackendFactories();

    const Graphics::BackendType previousType = g_RS.config.backendType;
    const bool hadBackend = static_cast<bool>(g_RS.backend);
    g_RS.config = config;
    const bool backendTypeChanged = hadBackend && (config.backendType != previousType);

    if (backendTypeChanged && g_RS.backend) {
        g_RS.backend->Shutdown();
        g_RS.backend.reset();
    }

    if (backendTypeChanged && g_RS.renderContext) {
        g_RS.renderContext->Shutdown();
        g_RS.renderContext.reset();
    }

    if (!g_RS.registry.HasBackendFactory(config.backendType)) {
        SAGE_WARNING("Renderer::Init falling back to OpenGL backend; requested backend factory missing");
    }

    if (!g_RS.backend) {
        auto created = g_RS.registry.CreateBackend(config);
        if (created) {
            g_RS.backend = std::shared_ptr<IRenderBackend>(std::move(created));
        } else {
            g_RS.backend = std::make_shared<Graphics::OpenGLRenderBackend>();
        }
    }

    g_RS.backend->Configure(g_RS.config);

    if (!g_RS.backend->IsInitialized()) {
        g_RS.backend->Init();
        g_RS.backend->Configure(g_RS.config);
    }

    if (!g_RS.renderContext) {
        g_RS.renderContext = Graphics::CreateDefaultRenderContext(g_RS.config, g_RS.registry);
    }

    if (g_RS.renderContext) {
        if (!g_RS.renderContext->IsInitialized()) {
            g_RS.renderContext->Init(g_RS.config);
        }
        // Force re-registration safety: if registry has null scene renderer but context created one, re-set it.
        if (!g_RS.registry.GetSceneRenderer()) {
            auto sceneShared = g_RS.renderContext->GetSceneRendererShared();
            if (sceneShared) {
                SAGE_INFO("Renderer::Init re-registering scene renderer from renderContext");
                g_RS.registry.SetSceneRenderer(sceneShared);
            } else {
                SAGE_WARNING("Renderer::Init: renderContext has no scene renderer to register");
            }
        }
    } else {
        SAGE_WARNING("Renderer::Init: render context creation failed for backend type {}", static_cast<int>(g_RS.config.backendType));
    }

    g_RS.registry.SetActiveBackendShared(g_RS.backend);
    
    // Build default render graph if absent
    if (!g_RS.graph) {
        g_RS.graph = std::make_unique<RenderGraph>();
        // Pass order: Clear -> Geometry (build) -> BatchSubmit (flush) -> Blur -> Composite -> Exposure
        g_RS.graph->AddPass(std::make_unique<ClearPass>());
        if (auto scene = Renderer::GetSceneRenderer()) {
            g_RS.graph->AddPass(std::make_unique<GeometryPass>(scene));
            g_RS.graph->AddPass(std::make_unique<BatchSubmitPass>(scene));
            g_RS.graph->AddPass(std::make_unique<BlurPass>(scene));
            g_RS.graph->AddPass(std::make_unique<CompositePass>(scene));
            g_RS.graph->AddPass(std::make_unique<ExposurePass>(scene));
        }
        g_RS.graph->InitializeAll(g_RS.backend.get());
    }
    
    // Initialize ViewportManager
    if (!g_RS.viewportManager) {
        g_RS.viewportManager = std::make_unique<ViewportManager>();
        
        // Register viewport change callback to update backend
        g_RS.viewportManager->RegisterCallback([](const Rect& viewport) {
            // Apply viewport to backend (low-level) if available
            if (IRenderBackend* backend = TryGetBackendInternal()) {
                backend->SetViewport(
                    static_cast<int>(viewport.x),
                    static_cast<int>(viewport.y),
                    static_cast<std::size_t>(viewport.width),
                    static_cast<std::size_t>(viewport.height));
            }

            // Central camera viewport update
            g_RS.camera.SetViewportSize(viewport.width, viewport.height);
        });
    }

    // Final validation log with dedupe: first successful init logs INFO, subsequent calls downgrade to TRACE unless state changed.
    static bool s_InitLogEmitted = false;
    if (auto scene = Renderer::GetSceneRenderer()) {
        const char* initState = scene->IsInitialized() ? "yes" : "no";
        if (!s_InitLogEmitted || backendTypeChanged) {
            SAGE_INFO("Renderer::Init complete: scene renderer valid (initialized={})", initState);
            s_InitLogEmitted = true;
        } else {
            SAGE_TRACE("Renderer::Init re-invoked: scene renderer valid (initialized={})", initState);
        }
    } else {
        // Always warn if scene renderer missing (critical)
        SAGE_WARNING("Renderer::Init complete: scene renderer is NULL. Rendering will be NO-OP until fixed.");
    }
}

void Renderer::Shutdown() {
    if (!g_RS.backend) {
        return;
    }

    if (g_RS.graph) {
        g_RS.graph->ShutdownAll();
        g_RS.graph.reset();
    }

    if (g_RS.renderContext) {
        g_RS.renderContext->Shutdown();
        g_RS.renderContext.reset();
    }

    g_RS.backend->Shutdown();
    g_RS.registry.SetActiveBackendShared(nullptr);
    g_RS.backend.reset();
}

bool Renderer::IsInitialized() {
    return g_RS.backend && g_RS.backend->IsInitialized();
}

Graphics::RenderSystemRegistry& Renderer::GetRegistry() {
    return g_RS.registry;
}

const Graphics::RenderSystemConfig& Renderer::GetConfig() {
    return g_RS.config;
}

void Renderer::RegisterBackendFactory(Graphics::BackendType type,
                                      std::function<std::unique_ptr<IRenderBackend>()> factory) {
    EnsureBackendFactories();
    g_RS.registry.RegisterBackendFactory(type, std::move(factory));
}

Graphics::IRenderDevice* Renderer::GetDevice() {
    auto device = g_RS.registry.GetDevice();
    return device.get();
}

Graphics::IRenderContext* Renderer::GetContext() {
    auto context = g_RS.registry.GetContext();
    return context.get();
}

Graphics::IResourceManager* Renderer::GetResourceManager() {
    auto resources = g_RS.registry.GetResourceManager();
    return resources.get();
}

Graphics::ISceneRenderer* Renderer::GetSceneRenderer() {
    auto sceneRenderer = g_RS.registry.GetSceneRenderer();
    return sceneRenderer.get();
}

void Renderer::Update(float deltaTime) {
    if (auto scene = GetSceneRenderer()) { scene->Update(deltaTime); }
    else if (IRenderBackend* backend = TryGetBackendInternal()) { backend->Update(deltaTime); }
}

void Renderer::SetCamera(const Camera2D& camera) {
    g_RS.camera = camera;
}

const Camera2D& Renderer::GetCamera() {
    return g_RS.camera;
}

void Renderer::ResetCamera() {
    g_RS.camera = Camera2D();
}

void Renderer::OnWindowResize(int width, int height) {
    // Delegate to ViewportManager which will notify all registered callbacks
    if (g_RS.viewportManager) {
        g_RS.viewportManager->OnWindowResize(width, height);
    }
}

void Renderer::SetContentScale(float scaleX, float scaleY) {
    g_RS.contentScale.x = std::max(0.01f, scaleX);
    g_RS.contentScale.y = std::max(0.01f, scaleY);
}

Vector2 Renderer::GetContentScale() {
    return g_RS.contentScale;
}

int Renderer::GetViewportWidth() {
    return static_cast<int>(g_RS.camera.GetViewportWidth());
}

int Renderer::GetViewportHeight() {
    return static_cast<int>(g_RS.camera.GetViewportHeight());
}

Vector2 Renderer::GetViewportSize() {
    return Vector2(g_RS.camera.GetViewportWidth(), g_RS.camera.GetViewportHeight());
}

Rect Renderer::GetViewportBounds() {
    return Rect(0.0f, 0.0f, g_RS.camera.GetViewportWidth(), g_RS.camera.GetViewportHeight());
}

IRenderBackend* Renderer::GetRenderBackend() {
    return GetBackend(); // internal helper returns shared_ptr raw
}

void Renderer::PushScreenShake(float amplitude, float frequency, float duration) {
    if (auto scene = GetSceneRenderer()) {
        scene->PushScreenShake(amplitude, frequency, duration);
    }
}

void Renderer::PopScreenShake() {
    // Will be implemented later as LIFO removal; currently maps to Clear
    ClearScreenShake();
}

void Renderer::ClearScreenShake() {
    // SceneRenderer currently doesn't expose explicit clear; retained for legacy
}

bool Renderer::IsShaking() {
    // To be reimplemented when scene exposes shaking state
    return false;
}

float Renderer::GetShakeIntensity() {
    return 0.0f;
}

#ifdef SAGE_ENGINE_TESTING
Vector2 Renderer::GetCameraShakeOffsetForTesting() {
    if (IRenderBackend* backend = TryGetBackendInternal()) {
        return backend->GetCameraShakeOffsetForTesting();
    }
    return Vector2::Zero();
}

float Renderer::GetShakeStrengthForTesting() {
    if (IRenderBackend* backend = TryGetBackendInternal()) {
        return backend->GetShakeStrengthForTesting();
    }
    return 0.0f;
}

float Renderer::GetShakeDurationForTesting() {
    if (IRenderBackend* backend = TryGetBackendInternal()) {
        return backend->GetShakeDurationForTesting();
    }
    return 0.0f;
}

float Renderer::GetShakeTimerForTesting() {
    if (IRenderBackend* backend = TryGetBackendInternal()) {
        return backend->GetShakeTimerForTesting();
    }
    return 0.0f;
}
#endif

void Renderer::BeginScene() {
    SAGE_TRACE("Renderer::BeginScene called");
    if (auto scene = GetSceneRenderer()) {
        SAGE_TRACE("SceneRenderer valid, calling SetCamera and BeginScene");
        scene->SetCamera(g_RS.camera);
        scene->BeginScene();
        SAGE_TRACE("SceneRenderer BeginScene completed");
    } else {
        SAGE_WARNING("Renderer::BeginScene: SceneRenderer is null!");
    }
    SAGE_TRACE("Renderer::BeginScene returning");
}

bool Renderer::EndScene() {
    auto scene = GetSceneRenderer();
    SAGE_INFO("Renderer::EndScene called, scene={}", scene ? "valid" : "null");
    bool sceneOk = true;
    FrameContext fctx; fctx.deltaTime = 0.0f; fctx.camera = &g_RS.camera; fctx.backend = g_RS.backend.get();
    // Execute graph passes across domains (initial simple single sequence)
    if (g_RS.graph) {
        // World domain
        fctx.pass.domain = RenderDomain::World;
        sceneOk = g_RS.graph->Execute(fctx) && sceneOk;
        // UI domain (future: separate UI renderer). For now reuse same graph.
        fctx.pass.domain = RenderDomain::UI;
        sceneOk = g_RS.graph->Execute(fctx) && sceneOk;
        // PostFX domain
        fctx.pass.domain = RenderDomain::PostFX;
        sceneOk = g_RS.graph->Execute(fctx) && sceneOk;
    }
    if (g_RS.uiRenderCallback) {
        SAGE_TRACE("Executing UI render callback");
        g_RS.uiRenderCallback();
        g_RS.uiRenderCallback = nullptr;
    }
    // Flush geometry + postFX inside scene EndScene
    if (scene) { sceneOk = scene->EndScene() && sceneOk; }
    // Software backend Present() removed - SAGEGraphics deleted, OpenGL-only now
    return sceneOk;
}

void Renderer::SetUIRenderCallback(std::function<void()> callback) {
    g_RS.uiRenderCallback = std::move(callback);
}

void Renderer::Clear(float r, float g, float b, float a) {
    RequireBackend().Clear(r, g, b, a);
}

void Renderer::Clear() {
    RequireBackend().Clear();
}

void Renderer::SetLayer(float layer) {
    if (auto scene = GetSceneRenderer()) { scene->SetLayer(layer); }
}

void Renderer::PushLayer(float layer) {
    if (auto scene = GetSceneRenderer()) { scene->PushLayer(layer); }
}

void Renderer::PopLayer() {
    if (auto scene = GetSceneRenderer()) { scene->PopLayer(); }
}

MaterialId Renderer::SetMaterial(MaterialId materialId) {
    if (auto scene = GetSceneRenderer()) { return scene->SetMaterial(materialId); }
    return materialId;
}

void Renderer::PushBlendMode(BlendMode mode) {
    if (auto scene = GetSceneRenderer()) { scene->PushBlendMode(mode); }
}

void Renderer::PopBlendMode() {
    if (auto scene = GetSceneRenderer()) { scene->PopBlendMode(); }
}

void Renderer::SetBlendMode(BlendMode mode) {
    if (auto scene = GetSceneRenderer()) { scene->SetBlendMode(mode); }
}

BlendMode Renderer::GetBlendMode() {
    if (auto scene = GetSceneRenderer()) { return scene->GetBlendMode(); }
    return BlendMode::Alpha;
}

void Renderer::PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                              float biasConstant, float biasSlope) {
    if (auto scene = GetSceneRenderer()) { scene->PushDepthState(enableTest, enableWrite, function, biasConstant, biasSlope); }
}

void Renderer::PopDepthState() {
    if (auto scene = GetSceneRenderer()) { scene->PopDepthState(); }
}

void Renderer::SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                             float biasConstant, float biasSlope) {
    if (auto scene = GetSceneRenderer()) { scene->SetDepthState(enableTest, enableWrite, function, biasConstant, biasSlope); }
}

DepthSettings Renderer::GetDepthState() {
    if (auto scene = GetSceneRenderer()) { return scene->GetDepthState(); }
    return DepthSettings{};
}

void Renderer::PushEffect(const QuadEffect& effect) {
    if (auto scene = GetSceneRenderer()) { scene->PushEffect(effect); }
}

void Renderer::PopEffect() {
    if (auto scene = GetSceneRenderer()) { scene->PopEffect(); }
}

void Renderer::ConfigurePostFX(const PostFXSettings& settings) {
    if (auto scene = GetSceneRenderer()) { scene->ConfigurePostFX(settings); }
}

const PostFXSettings& Renderer::GetPostFXSettings() {
    if (auto scene = GetSceneRenderer()) { return scene->GetPostFXSettings(); }

    static PostFXSettings s_DefaultSettings{};
    return s_DefaultSettings;
}

void Renderer::EnablePostFX(bool enabled) {
    if (auto scene = GetSceneRenderer()) { scene->EnablePostFX(enabled); }
}

bool Renderer::DrawQuad(const QuadDesc& desc) {
    if (auto scene = GetSceneRenderer()) {
        const bool queued = scene->DrawQuad(desc);
#ifdef SAGE_ENABLE_RENDER_TRACE
        SAGE_TRACE("Renderer::DrawQuad queued=%d pos=(%.2f,%.2f) size=(%.2f,%.2f) alpha=%.2f",
                   queued ? 1 : 0,
                   desc.position.x,
                   desc.position.y,
                   desc.size.x,
                   desc.size.y,
                   desc.color.a);
#endif
        return queued;
    }
    return false;
}

// ================= World <-> Screen helpers =================
Vector2 Renderer::WorldToScreen(const Vector2& world) {
    const Camera2D& cam = Renderer::GetCamera();
    return cam.WorldToScreen(world);
}

Vector2 Renderer::ScreenToWorld(const Vector2& screen) {
    const Camera2D& cam = Renderer::GetCamera();
    return cam.ScreenToWorld(screen);
}

bool Renderer::DrawText(const TextDesc& desc) {
    if (auto scene = GetSceneRenderer()) {
        return scene->DrawText(desc);
    }
    return false;
}

Float2 Renderer::MeasureText(const std::string& text, const Ref<Font>& font, float scale) {
    if (auto scene = GetSceneRenderer()) {
        return scene->MeasureText(text, font, scale);
    }
    return Float2(0.0f, 0.0f);
}

Renderer::RenderStats Renderer::GetRenderStats() {
    RenderStats rs;
    if (auto scene = GetSceneRenderer()) {
        // Downcast to OpenGLSceneRenderer to access Stats; dynamic cast safe if different implementation
        auto* oglScene = dynamic_cast<Graphics::OpenGLSceneRenderer*>(scene);
        if (oglScene) {
            auto s = oglScene->GetStats();
            rs.drawCalls = s.drawCalls;
            rs.vertices = s.vertices;
            rs.triangles = s.triangles;
            rs.requestedQuads = s.requestedQuads;
            rs.requestedGlyphs = s.requestedTextGlyphs;
            rs.requestedTiles = s.requestedTiles;
        }
    }
    return rs;
}

// Debug примитивы для физики
void Renderer::DrawLine(const Vector2& p1, const Vector2& p2, const Color& color, float thickness) {
    // Рисуем линию как тонкий прямоугольник
    Vector2 dir = p2 - p1;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.001f) return;
    
    // Calculate rotation angle from direction vector
    float angle = std::atan2(dir.y, dir.x) * (180.0f / 3.14159265359f);
    Vector2 center = (p1 + p2) * 0.5f;
    
    QuadDesc desc;
    // Position is top-left corner, not center
    // So we offset the center by half the size
    desc.position = Vector2(center.x - length * 0.5f, center.y - thickness * 0.5f);
    desc.size = Vector2(length, thickness);
    desc.rotation = angle;
    desc.color = color;
    
    DrawQuad(desc);
}

void Renderer::DrawCircle(const Vector2& center, float radius, const Color& color, float thickness) {
    // Аппроксимируем круг линиями
    const int segments = 32;
    const float angleStep = 2.0f * 3.14159265359f / segments;
    
    for (int i = 0; i < segments; ++i) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        
        Vector2 p1(center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1));
        Vector2 p2(center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2));
        
        DrawLine(p1, p2, color, thickness);
    }
}

void Renderer::DrawCircleFilled(const Vector2& center, float radius, const Color& color) {
    // Рисуем круг как треугольный веер
    const int segments = 32;
    const float angleStep = 2.0f * 3.14159265359f / segments;
    
    for (int i = 0; i < segments; ++i) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        
        Vector2 p1 = center;
        Vector2 p2(center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1));
        Vector2 p3(center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2));
        
        DrawTriangleFilled(p1, p2, p3, color);
    }
}

void Renderer::DrawTriangleFilled(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) {
    // Вычисляем центр и размер треугольника для QuadDesc
    Vector2 center = (p1 + p2 + p3) / 3.0f;
    
    // Простая реализация: рисуем как quad (не идеально, но работает для debug)
    Vector2 minP(std::min({p1.x, p2.x, p3.x}), std::min({p1.y, p2.y, p3.y}));
    Vector2 maxP(std::max({p1.x, p2.x, p3.x}), std::max({p1.y, p2.y, p3.y}));
    Vector2 size = maxP - minP;
    
    QuadDesc desc;
    desc.position = center;
    desc.size = size;
    desc.color = color;
    
    DrawQuad(desc);
}

ViewportManager* Renderer::GetViewportManager() {
    return g_RS.viewportManager.get();
}

RenderGraph* Renderer::GetGraph() {
    return g_RS.graph.get();
}

void Renderer::ReplaceGraph(std::unique_ptr<RenderGraph> newGraph) {
    if (!newGraph) {
        SAGE_WARNING("Renderer::ReplaceGraph called with null graph; ignoring");
        return;
    }
    
    // Shutdown old graph passes if present
    if (g_RS.graph) {
        g_RS.graph->ShutdownAll();
    }
    
    // Install new graph
    g_RS.graph = std::move(newGraph);
    
    // Initialize new graph passes with current backend
    if (g_RS.backend) {
        g_RS.graph->InitializeAll(g_RS.backend.get());
    } else {
        SAGE_WARNING("Renderer::ReplaceGraph: backend not initialized; graph passes not initialized");
    }
}

} // namespace SAGE
