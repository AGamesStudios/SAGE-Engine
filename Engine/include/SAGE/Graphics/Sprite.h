#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Math/Rect.h"
#include "SAGE/Math/Matrix3.h"
#include "SAGE/Graphics/Texture.h"

#include <memory>

namespace SAGE {

struct Transform2D {
    Vector2 position{0.0f, 0.0f};
    Vector2 scale{1.0f, 1.0f};
    float rotation = 0.0f;
    Vector2 origin{0.5f, 0.5f}; // Pivot point (0,0 = top-left, 0.5,0.5 = center)

    Transform2D() = default;
    
    inline Matrix3 GetMatrix() const {
        Matrix3 trans = Matrix3::Translation(position);
        Matrix3 rot = Matrix3::Rotation(rotation);
        Matrix3 scl = Matrix3::Scale(scale);
        return trans * rot * scl;
    }
};

class Sprite {
public:
    Sprite() = default;
    Sprite(std::shared_ptr<Texture> texture);

    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }

    Transform2D transform;
    Color tint = Color::White();
    Rect textureRect{0.0f, 0.0f, 1.0f, 1.0f}; // UV coordinates
    bool flipX = false;
    bool flipY = false;
    int layer = 0;
    bool visible = true;

    Vector2 GetSize() const;
    Rect GetBounds() const;

private:
    std::shared_ptr<Texture> m_Texture;
};

} // namespace SAGE
