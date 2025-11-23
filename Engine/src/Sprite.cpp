#include "SAGE/Graphics/Sprite.h"

namespace SAGE {

Sprite::Sprite(std::shared_ptr<Texture> texture)
    : m_Texture(texture)
{
}

void Sprite::SetTexture(std::shared_ptr<Texture> texture) {
    m_Texture = texture;
}

Vector2 Sprite::GetSize() const {
    if (!m_Texture) {
        return Vector2::Zero();
    }
    float width = static_cast<float>(m_Texture->GetWidth());
    float height = static_cast<float>(m_Texture->GetHeight());
    float scaledWidth = width * transform.scale.x;
    float scaledHeight = height * transform.scale.y;
    return Vector2(scaledWidth, scaledHeight);
}

Rect Sprite::GetBounds() const {
    Vector2 sz = GetSize();
    Vector2 pos = transform.position;
    return Rect(pos.x, pos.y, sz.x, sz.y);
}

} // namespace SAGE
