#include "Sprite.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/Core/Resources/Material.h"
#include "Core/Logger.h"
#include <algorithm>

namespace SAGE {

    Sprite::Sprite(const Ref<Texture>& texture)
        : m_Texture(texture) {
        InitFromTexture();
        if (HasTexture()) {
            // Full texture region
            m_TextureRegion = Rect(0, 0, static_cast<float>(m_Texture->GetWidth()), static_cast<float>(m_Texture->GetHeight()));
            UpdateCachedUV();
        }
    }

    Sprite::Sprite(const Ref<Texture>& texture, const Rect& region)
        : m_Texture(texture) {
        InitFromTexture();
        if (HasTexture()) {
            // Clamp region to texture bounds
            float w = static_cast<float>(m_Texture->GetWidth());
            float h = static_cast<float>(m_Texture->GetHeight());
            Rect r = region;
            if (r.x < 0) r.x = 0;
            if (r.y < 0) r.y = 0;
            if (r.x + r.width > w) r.width = std::max(0.0f, w - r.x);
            if (r.y + r.height > h) r.height = std::max(0.0f, h - r.y);
            m_TextureRegion = r;
            m_Size = Float2(r.width, r.height);
            UpdateCachedUV();
        }
    }

    Sprite::Sprite(const Float2& size, float r, float g, float b, float a)
        : m_Texture(nullptr) {
        m_Size = size;
        m_Color = Color(r, g, b, a);
        m_TextureRegion = Rect(0,0,size.x,size.y);
        // UVs stay default (0..1) for a solid quad
    }

    void Sprite::InitFromTexture() {
        if (HasTexture()) {
            m_Size = Float2(static_cast<float>(m_Texture->GetWidth()), static_cast<float>(m_Texture->GetHeight()));
            m_Color = Color::White();
        } else {
            m_Size = Float2(0.0f, 0.0f);
            m_Color = Color::White();
        }
    }

    // Optional manual recompute (if texture changed externally or region altered)
    static inline Float2 ComputeSizeFromRegion(const Rect& region) {
        return Float2(region.width, region.height);
    }

    void Sprite::RecomputeSizeFromTexture() {
        if (HasTexture()) {
            if (m_TextureRegion.width <= 0.0f || m_TextureRegion.height <= 0.0f) {
                // Reset to full texture if region invalid
                m_TextureRegion = Rect(0,0, static_cast<float>(m_Texture->GetWidth()), static_cast<float>(m_Texture->GetHeight()));
            }
            m_Size = ComputeSizeFromRegion(m_TextureRegion);
            UpdateCachedUV();
            m_CachedDescValid = false;
        }
    }

    void Sprite::SetTexture(const Ref<Texture>& texture) {
        m_Texture = texture;
        if (HasTexture()) {
            m_TextureRegion = Rect(0, 0, static_cast<float>(m_Texture->GetWidth()), static_cast<float>(m_Texture->GetHeight()));
            m_Size = Float2(m_TextureRegion.width, m_TextureRegion.height);
            UpdateCachedUV();
            m_CachedDescValid = false;
        }
    }

    void Sprite::SetTextureRegion(const Rect& region) {
        if (!HasTexture()) {
            SAGE_WARNING("Sprite::SetTextureRegion called without valid texture");
            return;
        }
        float w = static_cast<float>(m_Texture->GetWidth());
        float h = static_cast<float>(m_Texture->GetHeight());
        Rect r = region;
        if (r.x < 0) r.x = 0;
        if (r.y < 0) r.y = 0;
        if (r.x + r.width > w) r.width = std::max(0.0f, w - r.x);
        if (r.y + r.height > h) r.height = std::max(0.0f, h - r.y);
        
        // Проверка валидности после clamping
        if (r.width <= 0.0f || r.height <= 0.0f) {
            SAGE_ERROR("Invalid texture region after clamping: x={}, y={}, w={}, h={}", r.x, r.y, r.width, r.height);
            return;
        }
        
        m_TextureRegion = r;
        m_Size = Float2(r.width, r.height);
        UpdateCachedUV();
        m_CachedDescValid = false;
    }

    void Sprite::UpdateCachedUV() {
        if (!HasTexture()) {
            m_UVMin = Float2(0.0f, 0.0f);
            m_UVMax = Float2(1.0f, 1.0f);
            return;
        }
        float w = static_cast<float>(m_Texture->GetWidth());
        float h = static_cast<float>(m_Texture->GetHeight());
        if (w <= 0 || h <= 0) {
            m_UVMin = Float2(0.0f, 0.0f);
            m_UVMax = Float2(1.0f, 1.0f);
            return;
        }
        float u0 = m_TextureRegion.x / w;
        float v0 = m_TextureRegion.y / h;
        float u1 = (m_TextureRegion.x + m_TextureRegion.width) / w;
        float v1 = (m_TextureRegion.y + m_TextureRegion.height) / h;
        if (m_FlipX) std::swap(u0, u1);
        if (m_FlipY) std::swap(v0, v1);
        m_UVMin = Float2(u0, v0);
        m_UVMax = Float2(u1, v1);
        m_RegionNormalized = true;
    }

    bool Sprite::Draw() {
        if (!IsValid()) {
            return false;
        }

        // Save previous material to restore later
        MaterialId previousMaterial = 0;
        bool materialPushed = false;
        if (m_Material) {
            previousMaterial = Renderer::SetMaterial(m_Material->GetId());
            materialPushed = true;
        }

        bool effectPushed = false;
        if (m_Effect.Type != QuadEffectType::None) {
            Renderer::PushEffect(m_Effect);
            effectPushed = true;
        }

        // Use cached QuadDesc if valid, otherwise recompute
        if (!m_CachedDescValid) {
            UpdateCachedQuadDesc();
        }

        Renderer::DrawQuad(m_CachedDesc);

        if (effectPushed)
            Renderer::PopEffect();

        if (materialPushed) {
            Renderer::SetMaterial(previousMaterial);
        }
        
        return true;
    }

    void Sprite::UpdateCachedQuadDesc() const {
        // Apply origin: position treated as top-left external; adjust for origin anchor
        Float2 scaledSize = Float2(m_Size.x * m_Scale.x, m_Size.y * m_Scale.y);
        Float2 anchorOffset = Float2(scaledSize.x * m_Origin.x, scaledSize.y * m_Origin.y);
        m_CachedDesc.position = Float2(m_Position.x - anchorOffset.x, m_Position.y - anchorOffset.y);
        m_CachedDesc.size = scaledSize;
        m_CachedDesc.rotation = m_Rotation;
        m_CachedDesc.color = m_Color;
        if (HasTexture()) {
            m_CachedDesc.texture = m_Texture;
            m_CachedDesc.uvMin = m_UVMin;
            m_CachedDesc.uvMax = m_UVMax;
        } else {
            m_CachedDesc.texture.reset();
        }
        m_CachedDescValid = true;
    }

}
