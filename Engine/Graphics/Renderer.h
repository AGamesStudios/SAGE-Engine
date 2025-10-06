#pragma once

#include "RendererTypes.h"
#include "Material.h"

#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
#ifdef DrawTextW
#undef DrawTextW
#endif

namespace SAGE {

    class Renderer {
    public:
        static void Init();
        static void Shutdown();

        static void Update(float deltaTime);

        static void SetCamera(const Camera2D& camera);
        static const Camera2D& GetCamera();
        static void ResetCamera();

        static void PushScreenShake(float amplitude, float duration);

        static void BeginScene();
        static void EndScene();

        static void Clear(float r, float g, float b, float a);
        static void Clear();

        static void SetLayer(float layer);
        static void PushLayer(float layer);
        static void PopLayer();

        static MaterialId SetMaterial(MaterialId materialId);

        static void PushEffect(const QuadEffect& effect);
        static void PopEffect();

    static void ConfigurePostFX(const PostFXSettings& settings);
    static const PostFXSettings& GetPostFXSettings();
    static void EnablePostFX(bool enabled);

        static void DrawQuad(const QuadDesc& desc);

        static void DrawText(const TextDesc& desc);

        static Float2 MeasureText(const std::string& text, const Ref<Font>& font, float scale = 1.0f);
    };

} // namespace SAGE
