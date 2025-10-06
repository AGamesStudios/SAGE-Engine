#include "Sprite.h"
#include "Renderer.h"

namespace SAGE {

        Sprite::Sprite(const Ref<Texture>& texture)
                : m_Texture(texture), m_Position(0.0f, 0.0f),
                    m_Size(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())) {
        m_Color[0] = m_Color[1] = m_Color[2] = m_Color[3] = 1.0f;
    }

        Sprite::Sprite(const Float2& size, float r, float g, float b, float a)
        : m_Texture(nullptr), m_Position(0.0f, 0.0f), m_Size(size) {
        m_Color[0] = r;
        m_Color[1] = g;
        m_Color[2] = b;
        m_Color[3] = a;
    }

    void Sprite::Draw() {
        QuadDesc desc;
        desc.position = m_Position;
        desc.size = m_Size;
        desc.color = Color(m_Color[0], m_Color[1], m_Color[2], m_Color[3]);

        if (m_Texture && m_Texture->IsLoaded()) {
            desc.texture = m_Texture;
        }

        Renderer::DrawQuad(desc);
    }

}
