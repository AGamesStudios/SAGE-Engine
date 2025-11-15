#include "SpriteBatchSoA.h"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"
#include <algorithm>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(__AVX2__)
#include <immintrin.h>
#define SAGE_USE_AVX2 1
#else
#define SAGE_USE_AVX2 0
#endif

namespace SAGE {

    SpriteBatchSoA::SpriteBatchSoA(size_t capacity)
        : m_Capacity(capacity), m_Count(0) {
        Reserve(capacity);
    }

    void SpriteBatchSoA::Reserve(size_t newCapacity) {
        m_Capacity = newCapacity;
        m_Positions.reserve(newCapacity);
        m_Sizes.reserve(newCapacity);
        m_Scales.reserve(newCapacity);
        m_Colors.reserve(newCapacity);
        m_Rotations.reserve(newCapacity);
        m_UVMin.reserve(newCapacity);
        m_UVMax.reserve(newCapacity);
        m_Active.reserve(newCapacity);
    }

    void SpriteBatchSoA::Clear() {
        m_Count = 0;
        m_Positions.clear();
        m_Sizes.clear();
        m_Scales.clear();
        m_Colors.clear();
        m_Rotations.clear();
        m_UVMin.clear();
        m_UVMax.clear();
        m_Active.clear();
    }

    uint32_t SpriteBatchSoA::AddSprite(const Float2& position, const Float2& size, const Color& color) {
        if (m_Count >= m_Capacity) {
            SAGE_WARNING("SpriteBatchSoA: capacity exceeded, expanding from {} to {}", m_Capacity, m_Capacity * 2);
            Reserve(m_Capacity * 2);
        }

        uint32_t index = static_cast<uint32_t>(m_Count);
        m_Positions.push_back(position);
        m_Sizes.push_back(size);
        m_Scales.push_back(Float2(1.0f, 1.0f));
        m_Colors.push_back(color);
        m_Rotations.push_back(0.0f);
        m_UVMin.push_back(Float2(0.0f, 0.0f));
        m_UVMax.push_back(Float2(1.0f, 1.0f));
        m_Active.push_back(1);
        m_Count++;
        return index;
    }

    uint32_t SpriteBatchSoA::AddSpriteFromSheet(const Float2& position, const Float2& size, const Float2& uvMin, const Float2& uvMax, const Color& color) {
        uint32_t idx = AddSprite(position, size, color);
        // Overwrite default full texture UVs with frame specific
        m_UVMin[idx] = uvMin;
        m_UVMax[idx] = uvMax;
        return idx;
    }

    void SpriteBatchSoA::SetPosition(uint32_t index, const Float2& pos) {
        if (ValidIndex(index)) m_Positions[index] = pos;
    }

    void SpriteBatchSoA::SetSize(uint32_t index, const Float2& size) {
        if (ValidIndex(index)) m_Sizes[index] = size;
    }

    void SpriteBatchSoA::SetColor(uint32_t index, const Color& color) {
        if (ValidIndex(index)) m_Colors[index] = color;
    }

    void SpriteBatchSoA::SetRotation(uint32_t index, float radians) {
        if (ValidIndex(index)) m_Rotations[index] = radians;
    }

    void SpriteBatchSoA::SetScale(uint32_t index, const Float2& scale) {
        if (ValidIndex(index)) m_Scales[index] = scale;
    }

    void SpriteBatchSoA::SetUV(uint32_t index, const Float2& uvMin, const Float2& uvMax) {
        if (ValidIndex(index)) {
            m_UVMin[index] = uvMin;
            m_UVMax[index] = uvMax;
        }
    }

    void SpriteBatchSoA::SetActive(uint32_t index, bool active) {
        if (ValidIndex(index)) m_Active[index] = active ? 1 : 0;
    }

    // SIMD bulk operations
    void SpriteBatchSoA::OffsetAllPositions(const Float2& delta) {
#if SAGE_USE_AVX2
        // AVX2: process 4 Float2 (8 floats) per iteration
        size_t simdCount = (m_Count / 4) * 4;
        __m256 deltaVec = _mm256_set_ps(delta.y, delta.x, delta.y, delta.x, delta.y, delta.x, delta.y, delta.x);
        
        for (size_t i = 0; i < simdCount; i += 4) {
            __m256 pos = _mm256_loadu_ps(reinterpret_cast<float*>(&m_Positions[i]));
            pos = _mm256_add_ps(pos, deltaVec);
            _mm256_storeu_ps(reinterpret_cast<float*>(&m_Positions[i]), pos);
        }
        
        // Scalar fallback for remainder
        for (size_t i = simdCount; i < m_Count; ++i) {
            m_Positions[i].x += delta.x;
            m_Positions[i].y += delta.y;
        }
#else
        // Scalar fallback (no AVX2)
        for (size_t i = 0; i < m_Count; ++i) {
            m_Positions[i].x += delta.x;
            m_Positions[i].y += delta.y;
        }
#endif
    }

    void SpriteBatchSoA::MultiplyAllColors(const Color& tint) {
        // Scalar version (SIMD color multiply requires careful RGBA packing)
        for (size_t i = 0; i < m_Count; ++i) {
            m_Colors[i].r *= tint.r;
            m_Colors[i].g *= tint.g;
            m_Colors[i].b *= tint.b;
            m_Colors[i].a *= tint.a;
        }
    }

    void SpriteBatchSoA::FadeAllAlpha(float multiplier) {
#if SAGE_USE_AVX2
        // AVX2: process 2 colors (8 floats) per iteration
        size_t simdCount = (m_Count / 2) * 2;
        __m256 alphaMask = _mm256_set_ps(multiplier, 1.0f, 1.0f, 1.0f, multiplier, 1.0f, 1.0f, 1.0f);
        
        for (size_t i = 0; i < simdCount; i += 2) {
            __m256 color = _mm256_loadu_ps(reinterpret_cast<float*>(&m_Colors[i]));
            color = _mm256_mul_ps(color, alphaMask);
            _mm256_storeu_ps(reinterpret_cast<float*>(&m_Colors[i]), color);
        }
        
        for (size_t i = simdCount; i < m_Count; ++i) {
            m_Colors[i].a *= multiplier;
        }
#else
        for (size_t i = 0; i < m_Count; ++i) {
            m_Colors[i].a *= multiplier;
        }
#endif
    }

    void SpriteBatchSoA::ScaleAllSizes(const Float2& scale) {
        for (size_t i = 0; i < m_Count; ++i) {
            m_Sizes[i].x *= scale.x;
            m_Sizes[i].y *= scale.y;
        }
    }

    void SpriteBatchSoA::RotateAll(float deltaRadians) {
        for (size_t i = 0; i < m_Count; ++i) {
            m_Rotations[i] += deltaRadians;
        }
    }

    void SpriteBatchSoA::Draw() {
        if (m_Count == 0) return;
        SAGE_INFO("SpriteBatchSoA::Draw count={}", (unsigned)m_Count);

        // Set shared material if present
        MaterialId prevMaterial = 0;
        bool materialPushed = false;
        if (m_Material) {
            prevMaterial = Renderer::SetMaterial(m_Material->GetId());
            materialPushed = true;
        }

        // Submit each active sprite
        for (size_t i = 0; i < m_Count; ++i) {
            if (!m_Active[i]) continue;
            // Debug log first few sprites only
            if(i < 1) {
                SAGE_INFO("Draw sprite[0] pos=(%.1f,%.1f) size=(%.1f,%.1f) uv=(%.3f,%.3f)-(%.3f,%.3f)",
                          m_Positions[i].x, m_Positions[i].y, m_Sizes[i].x, m_Sizes[i].y,
                          m_UVMin[i].x, m_UVMin[i].y, m_UVMax[i].x, m_UVMax[i].y);
            }

            QuadDesc desc;
            desc.position = m_Positions[i];
            Float2 scaledSize(m_Sizes[i].x * m_Scales[i].x, m_Sizes[i].y * m_Scales[i].y);
            desc.size = scaledSize;
            desc.color = m_Colors[i];
            desc.rotation = m_Rotations[i];
            desc.texture = m_Texture;
            desc.uvMin = m_UVMin[i];
            desc.uvMax = m_UVMax[i];

            Renderer::DrawQuad(desc);
        }

        if (materialPushed) {
            Renderer::SetMaterial(prevMaterial);
        }
    }

    void SpriteBatchSoA::CullOutsideRect(const Rect& visibleBounds) {
        for (size_t i = 0; i < m_Count; ++i) {
            Float2 pos = m_Positions[i];
            Float2 size = m_Sizes[i];
            
            // Simple AABB overlap test
            bool visible = !(pos.x + size.x < visibleBounds.x ||
                             pos.x > visibleBounds.x + visibleBounds.width ||
                             pos.y + size.y < visibleBounds.y ||
                             pos.y > visibleBounds.y + visibleBounds.height);
            
            m_Active[i] = visible ? 1 : 0;
        }
    }

} // namespace SAGE
