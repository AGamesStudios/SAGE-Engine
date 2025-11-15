#pragma once

#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Graphics/Backend/Common/BackendType.h"
#include "Graphics/API/RenderSystemRegistry.h"
#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Camera2D.h"  // Use new Camera2D class

#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
#ifdef DrawTextW
#undef DrawTextW
#endif
#include <functional>
#include <memory>

namespace SAGE {

class ViewportManager;
class RenderGraph; // Forward declaration for graph customization API

namespace Graphics {
    class RenderSystemRegistry;
    struct RenderSystemConfig;
    class IRenderDevice;
    class IRenderContext;
    class IResourceManager;
    class ISceneRenderer;
}

    class Renderer {
    public:
        static void Init();
        static void Init(const Graphics::RenderSystemConfig& config);
        static void Shutdown();
        static bool IsInitialized();

        static Graphics::RenderSystemRegistry& GetRegistry();
        static const Graphics::RenderSystemConfig& GetConfig();
    static void RegisterBackendFactory(Graphics::BackendType type,
                       std::function<std::unique_ptr<IRenderBackend>()> factory);

        static Graphics::IRenderDevice* GetDevice();
        static Graphics::IRenderContext* GetContext();
        static Graphics::IResourceManager* GetResourceManager();
        static Graphics::ISceneRenderer* GetSceneRenderer();

        static void Update(float deltaTime);

    static void SetCamera(const Camera2D& camera);
    static const Camera2D& GetCamera();
    
    /// @brief Reset camera to default values
    /// Position: (viewportWidth/2, viewportHeight/2)
    /// Zoom: 1.0f
    /// Rotation: 0.0f
    static void ResetCamera();
    
    /// Notify renderer of window/viewport resize
    /// Updates viewport and recalculates projection matrices
    static void OnWindowResize(int width, int height);
    
    /// Get current viewport width in pixels
    static int GetViewportWidth();
    
    /// Get current viewport height in pixels
    static int GetViewportHeight();
    
    /// Get current viewport size as Vector2 (width, height)
    static Vector2 GetViewportSize();
    
    /// Get current viewport bounds as Rect (x, y, width, height)
    static Rect GetViewportBounds();
    
    /// Convert world coordinates to screen coordinates
    static Vector2 WorldToScreen(const Vector2& world);
    
    /// Convert screen coordinates to world coordinates
    static Vector2 ScreenToWorld(const Vector2& screen);
    
    /// Set content scale for HiDPI displays (affects UI rendering)
    static void SetContentScale(float scaleX, float scaleY);
    
    /// Get current content scale
    static Vector2 GetContentScale();
    
    /// Get ViewportManager for advanced viewport control
    static ViewportManager* GetViewportManager();

    static void PushScreenShake(float amplitude, float frequency, float duration);
    
    /// Stop current screen shake effect
    static void PopScreenShake();
    
    /// Clear all screen shake effects
    static void ClearScreenShake();
    
    /// Check if screen shake is currently active
    static bool IsShaking();
    
    /// Get current shake intensity (0.0 - 1.0)
    static float GetShakeIntensity();
    
#ifdef SAGE_ENGINE_TESTING
    static Vector2 GetCameraShakeOffsetForTesting();
    static float GetShakeStrengthForTesting();
    static float GetShakeDurationForTesting();
    static float GetShakeTimerForTesting();
#endif

        static void BeginScene();
        static bool EndScene();

        static void Clear(float r, float g, float b, float a);
        static void Clear();

        static void SetLayer(float layer);
        static void PushLayer(float layer);
        static void PopLayer();

        static MaterialId SetMaterial(MaterialId materialId);

        static void PushBlendMode(BlendMode mode);
        static void PopBlendMode();
        static void SetBlendMode(BlendMode mode);
        static BlendMode GetBlendMode();

        static void PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                   float biasConstant, float biasSlope);
        static void PopDepthState();
        static void SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                  float biasConstant, float biasSlope);
        static DepthSettings GetDepthState();

        static void PushEffect(const QuadEffect& effect);
        static void PopEffect();

        static void ConfigurePostFX(const PostFXSettings& settings);
        static const PostFXSettings& GetPostFXSettings();
        static void EnablePostFX(bool enabled);

        // Safe backend accessor (may return nullptr if not initialized)
        static IRenderBackend* GetRenderBackend();

        // Render graph customization API
        // Get pointer to current render graph (non-owning, may be null before Init)
        static RenderGraph* GetGraph();
        
        // Replace current render graph with custom one
        // Shuts down old graph passes before installing new graph
        // Initializes new graph passes with current backend
        static void ReplaceGraph(std::unique_ptr<RenderGraph> newGraph);

        static bool DrawQuad(const QuadDesc& desc);

        static bool DrawText(const TextDesc& desc);

        static Float2 MeasureText(const std::string& text, const Ref<Font>& font, float scale = 1.0f);
        struct RenderStats {
            std::size_t drawCalls = 0;
            std::size_t vertices = 0;
            std::size_t triangles = 0;
            std::size_t requestedQuads = 0;
            std::size_t requestedGlyphs = 0;
            std::size_t requestedTiles = 0; // logical tile draws from TilemapRenderer
        };
        static RenderStats GetRenderStats();
        // Allows higher level systems (e.g., editor) to queue UI rendering that must
        // execute after the render graph has finished for the frame.
        static void SetUIRenderCallback(std::function<void()> callback);
        
        // Debug примитивы для физики
        static void DrawLine(const Vector2& p1, const Vector2& p2, const Color& color, float thickness = 1.0f);
        static void DrawCircle(const Vector2& center, float radius, const Color& color, float thickness = 1.0f);
        static void DrawCircleFilled(const Vector2& center, float radius, const Color& color);
        static void DrawTriangleFilled(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color);
    };

} // namespace SAGE
