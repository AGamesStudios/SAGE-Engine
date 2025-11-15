#pragma once

#include "Graphics/Core/Types/MathTypes.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Material.h"
#include "Memory/Ref.h"
#include "Core/Color.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include <vector>
#include <cstdint>

namespace SAGE {

    /**
     * @brief SpriteBatchSoA - Structure of Arrays sprite batch for SIMD bulk operations
     * 
     * Experimental data-oriented design for high-performance sprite management.
     * Separates sprite data into contiguous arrays enabling AVX2/SIMD transformations.
     * 
     * Use cases:
     * - Particle systems (10k+ sprites)
     * - Bullet patterns
     * - Background tile layers
     * - Massive UI element batches
     * 
     * Performance advantages:
     * - Cache-friendly sequential access
     * - SIMD vectorized updates (position, color, alpha)
     * - Reduced memory fragmentation
     * - Batch frustum culling
     * 
     * Limitations:
     * - No per-sprite animation frames (use single texture atlas)
     * - Shared material/effect per batch
     * - Fixed capacity (resize requires realloc)
     */
    class SpriteBatchSoA {
    public:
        explicit SpriteBatchSoA(size_t capacity = 1024);
        ~SpriteBatchSoA() = default;

        // Capacity management
        void Reserve(size_t newCapacity);
        void Clear();
        size_t GetCapacity() const { return m_Capacity; }
        size_t GetCount() const { return m_Count; }

        // Add sprite (returns index for future updates)
        uint32_t AddSprite(const Float2& position, const Float2& size, const Color& color = Color::White());
    // Add sprite from spritesheet frame (expects UVs already computed externally)
    uint32_t AddSpriteFromSheet(const Float2& position, const Float2& size, const Float2& uvMin, const Float2& uvMax, const Color& color = Color::White());
        
        // Batch operations (single sprite)
        void SetPosition(uint32_t index, const Float2& pos);
        void SetSize(uint32_t index, const Float2& size);
        void SetColor(uint32_t index, const Color& color);
        void SetRotation(uint32_t index, float radians);
        void SetScale(uint32_t index, const Float2& scale);
        void SetUV(uint32_t index, const Float2& uvMin, const Float2& uvMax);
        void SetActive(uint32_t index, bool active);

        // Getters
        Float2 GetPosition(uint32_t index) const { return m_Positions[index]; }
        Float2 GetSize(uint32_t index) const { return m_Sizes[index]; }
        Color GetColor(uint32_t index) const { return m_Colors[index]; }
        bool IsActive(uint32_t index) const { return m_Active[index]; }

        // SIMD bulk operations (AVX2 optimized when available)
        void OffsetAllPositions(const Float2& delta);
        void MultiplyAllColors(const Color& tint);
        void FadeAllAlpha(float multiplier);
        void ScaleAllSizes(const Float2& scale);
        void RotateAll(float deltaRadians);
        
        // Batch rendering (requires shared texture/material)
        void SetTexture(const Ref<Texture>& texture) { m_Texture = texture; }
        void SetMaterial(const Ref<Material>& material) { m_Material = material; }
        void Draw(); // Submits all active sprites to renderer

        // Frustum culling (marks inactive if outside camera bounds)
        void CullOutsideRect(const Rect& visibleBounds);

    private:
        size_t m_Capacity = 0;
        size_t m_Count = 0;

        // SoA data layout (cache-aligned for SIMD)
        std::vector<Float2> m_Positions;
        std::vector<Float2> m_Sizes;
        std::vector<Float2> m_Scales;
        std::vector<Color>  m_Colors;
        std::vector<float>  m_Rotations;
        std::vector<Float2> m_UVMin;
        std::vector<Float2> m_UVMax;
        std::vector<uint8_t> m_Active; // 0 = culled/inactive, 1 = active

        // Shared resources
        Ref<Texture> m_Texture;
        Ref<Material> m_Material;

        // Helper: ensure index valid
        inline bool ValidIndex(uint32_t idx) const { return idx < m_Count; }
    };

} // namespace SAGE
