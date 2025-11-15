#pragma once

#include "Graphics/Core/Types/MathTypes.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Material.h"
#include "Memory/Ref.h"
#include "Core/Color.h"
#include "Graphics/Core/Types/RendererTypes.h"

namespace SAGE {

    /**
     * @brief Sprite - 2D визуальный объект, который может быть текстурированным или цветным прямоугольником.
     * 
     * Возможности:
     * - Полная текстура или произвольный регион (pixel-space Rect)
     * - Материал (опционально) для блендинга и шейдерных настроек
     * - Цветовой множитель (tint) + alpha
     * - Поворот, масштаб, origin (якорь) для вращения
     * - Flip по X/Y (через перестановку UV)
     * - Анимация кадров (массив Rect регионов)
     */
    class Sprite {
    public:
        virtual ~Sprite() = default;
        
        enum class PivotPreset {
            TopLeft,
            TopCenter,
            TopRight,
            CenterLeft,
            Center,
            CenterRight,
            BottomLeft,
            BottomCenter,
            BottomRight
        };
    // Full texture region constructor (использует всю текстуру)
        explicit Sprite(const Ref<Texture>& texture);
    // Custom region (pixel-space) constructor (регион в пикселях: x,y,width,height)
        Sprite(const Ref<Texture>& texture, const Rect& region);
    // Solid color quad sprite (no texture) - рисуется как просто залитый прямоугольник
        Sprite(const Float2& size, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

        // Position / Size
    void SetPosition(const Float2& position) { m_Position = position; m_CachedDescValid = false; }
    void SetSize(const Float2& size) { m_Size = size; m_CachedDescValid = false; }
        const Float2& GetPosition() const { return m_Position; }
        const Float2& GetSize() const { return m_Size; }
    // Position helpers (semantic clarity): m_Position хранит логический anchor (по умолчанию top-left до учёта origin)
    Float2 GetCenter() const { return Float2(m_Position.x + m_Size.x * 0.5f, m_Position.y + m_Size.y * 0.5f); }
    void SetCenter(const Float2& center) { m_Position = Float2(center.x - m_Size.x * 0.5f, center.y - m_Size.y * 0.5f); m_CachedDescValid = false; }
    Float2 GetTopLeft() const { return m_Position; }
    void SetTopLeft(const Float2& tl) { m_Position = tl; m_CachedDescValid = false; }

        // Color
    void SetColor(float r, float g, float b, float a) { m_Color = Color(r, g, b, a); m_CachedDescValid = false; }
    void SetColor(const Color& c) { m_Color = c; m_CachedDescValid = false; }
    void SetAlpha(float a) { m_Color.a = a; m_CachedDescValid = false; }
    void MultiplyColor(const Color& c) { m_Color.r *= c.r; m_Color.g *= c.g; m_Color.b *= c.b; m_Color.a *= c.a; m_CachedDescValid = false; }
    void ModulateColor(float intensity) { m_Color.r *= intensity; m_Color.g *= intensity; m_Color.b *= intensity; m_CachedDescValid = false; }
        const Color& GetColor() const { return m_Color; }

        // Texture binding / region (pixel-space rectangle, NOT normalized)
        void SetTexture(const Ref<Texture>& texture);
        bool HasTexture() const { return m_Texture && m_Texture->IsLoaded(); }
        void SetTextureRegion(const Rect& region); // pixel-space
        const Rect& GetTextureRegion() const { return m_TextureRegion; }
        std::pair<Float2, Float2> GetUVCoords() const { return { m_UVMin, m_UVMax }; }

        // Frame animation support
        void SetFrames(const std::vector<Rect>& frames) {
            m_Frames = frames; m_CurrentFrame = 0; m_FrameTimeAccumulator = 0.0f; m_CachedDescValid = false;
            if (!m_Frames.empty()) { SetTextureRegion(m_Frames[0]); }
        }
        void SetFrameIndex(size_t idx) {
            if (idx < m_Frames.size()) { m_CurrentFrame = idx; SetTextureRegion(m_Frames[m_CurrentFrame]); }
        }
        size_t GetFrameIndex() const { return m_CurrentFrame; }
        size_t GetFrameCount() const { return m_Frames.size(); }
        void AdvanceFrame() {
            if (m_Frames.empty()) return; 
            m_CurrentFrame = (m_CurrentFrame + 1) % m_Frames.size();
            SetTextureRegion(m_Frames[m_CurrentFrame]);
        }
        // Time-based update with spiral-of-death protection
        void UpdateAnimation(float deltaTime, float frameDurationSeconds) {
            if (m_Frames.empty() || frameDurationSeconds <= 0.0f) return;
            
            m_FrameTimeAccumulator += deltaTime;
            
            // Защита от спирали смерти - максимум 10 пропущенных кадров
            constexpr size_t maxFrameSkips = 10;
            size_t framesAdvanced = 0;
            
            while (m_FrameTimeAccumulator >= frameDurationSeconds && framesAdvanced < maxFrameSkips) {
                m_FrameTimeAccumulator -= frameDurationSeconds;
                AdvanceFrame();
                ++framesAdvanced;
            }
            
            // Если пропущено слишком много - сброс
            if (framesAdvanced >= maxFrameSkips) {
                m_FrameTimeAccumulator = 0.0f;
            }
        }

        // Material (optional)
    void SetMaterial(const Ref<Material>& material) { m_Material = material; m_CachedDescValid = false; }
        Ref<Material> GetMaterial() const { return m_Material; }

        // Flip / transform
    void SetFlip(bool fx, bool fy) { m_FlipX = fx; m_FlipY = fy; UpdateCachedUV(); m_CachedDescValid = false; }
        bool IsFlippedX() const { return m_FlipX; }
        bool IsFlippedY() const { return m_FlipY; }

    void SetRotation(float radians) { m_Rotation = radians; m_CachedDescValid = false; }
        float GetRotation() const { return m_Rotation; }
    void SetScale(const Float2& scale) { m_Scale = scale; m_CachedDescValid = false; }
        const Float2& GetScale() const { return m_Scale; }

    // Origin (0..1 normalized relative to size) for rotation anchor (0,0 = top-left, 0.5,0.5 = center)
    void SetOrigin(const Float2& origin01) { m_Origin = origin01; m_CachedDescValid = false; }
        const Float2& GetOrigin() const { return m_Origin; }
        void SetPivotNormalized(float nx, float ny) { m_Origin = Float2(nx, ny); m_CachedDescValid = false; }
        void SetPivotPreset(PivotPreset preset) {
            switch (preset) {
            case PivotPreset::TopLeft: m_Origin = {0.0f,0.0f}; break;
            case PivotPreset::TopCenter: m_Origin = {0.5f,0.0f}; break;
            case PivotPreset::TopRight: m_Origin = {1.0f,0.0f}; break;
            case PivotPreset::CenterLeft: m_Origin = {0.0f,0.5f}; break;
            case PivotPreset::Center: m_Origin = {0.5f,0.5f}; break;
            case PivotPreset::CenterRight: m_Origin = {1.0f,0.5f}; break;
            case PivotPreset::BottomLeft: m_Origin = {0.0f,1.0f}; break;
            case PivotPreset::BottomCenter: m_Origin = {0.5f,1.0f}; break;
            case PivotPreset::BottomRight: m_Origin = {1.0f,1.0f}; break;
            }
            m_CachedDescValid = false;
        }
        void SetPivotPixels(float px, float py) {
            if (m_Size.x <= 0.0f || m_Size.y <= 0.0f) {
                SAGE_WARN("SetPivotPixels: Cannot set pivot on sprite with zero size");
                return;
            }
            m_Origin = Float2(px / m_Size.x, py / m_Size.y);
            m_CachedDescValid = false;
        }

        // Validation
    // IsValid: размеры > 0 и либо есть текстура, либо непрозрачный цвет.
    bool IsValid() const { return m_Size.x > 0.0f && m_Size.y > 0.0f && (HasTexture() || m_Color.a > 0.0f); }
    bool IsSolidColor() const { return !HasTexture(); }

        // Draw sprite (returns false if skipped)
    bool Draw();

    // Per-sprite effect
    void SetEffect(const QuadEffect& effect) { m_Effect = effect; m_CachedDescValid = false; }
    const QuadEffect& GetEffect() const { return m_Effect; }

    // Dirty flag API - using m_CachedDescValid
    bool IsDirty() const { return !m_CachedDescValid; }
    void ClearDirty() { /* no-op, kept for compatibility */ }
    void MarkDirty() { m_CachedDescValid = false; }
    // Conditional submit helper
    bool SubmitIfDirty() { if (m_CachedDescValid) return false; bool r = Draw(); return r; }

    // Cached QuadDesc access (internal optimization)
    const QuadDesc& GetCachedQuadDesc() const { return m_CachedDesc; }

    private:
    void InitFromTexture();
    void UpdateCachedUV();

        Ref<Texture> m_Texture;
        Ref<Material> m_Material;

        Rect     m_TextureRegion{ 0,0,0,0 }; // pixel region of texture
        Float2   m_UVMin{ 0.0f, 0.0f };
        Float2   m_UVMax{ 1.0f, 1.0f };
        bool     m_RegionNormalized = false; // region converted to UV

        Float2   m_Position{ 0.0f, 0.0f };
        Float2   m_Size{ 1.0f, 1.0f };
        Float2   m_Scale{ 1.0f, 1.0f };
        Float2   m_Origin{ 0.5f, 0.5f }; // center origin default
        float    m_Rotation = 0.0f;
        Color    m_Color{ 1,1,1,1 };
        bool     m_FlipX = false;
        bool     m_FlipY = false;
    QuadEffect m_Effect{}; // Optional per-sprite quad shader effect

        // Animation frames
        std::vector<Rect> m_Frames; 
        size_t  m_CurrentFrame = 0;
        float   m_FrameTimeAccumulator = 0.0f;

        // Cached QuadDesc to avoid recompute on every Draw when not dirty
        mutable QuadDesc m_CachedDesc;
        mutable bool m_CachedDescValid = false;

        void RecomputeSizeFromTexture();
        void UpdateCachedQuadDesc() const;
    };

}