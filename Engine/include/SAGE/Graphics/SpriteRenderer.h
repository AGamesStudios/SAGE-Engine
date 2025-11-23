#pragma once

#include "SAGE/Math/Matrix3.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Rect.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Texture.h"
#include "SAGE/Graphics/Shader.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace SAGE {

class SpriteRenderer {
public:
    void Init();
    void Shutdown();

    struct BatchStats {
        uint32_t drawCalls = 0;
        uint32_t vertices = 0;
        uint32_t triangles = 0;
    };

    void Begin(const Matrix3& projection);
    void Submit(const Sprite& sprite);
    BatchStats Flush();
    bool HasPendingSprites() const { return !m_Commands.empty(); }

private:
    struct SpriteCommand {
        Matrix3 transform;
        Color tint;
        Rect uv;
        Vector2 size;
        Vector2 origin;
        std::shared_ptr<Texture> texture;
        int layer = 0;
        bool flipX = false;
        bool flipY = false;
    };

    struct SpriteVertex {
        Vector2 position;
        Vector2 texCoord;
        Color color;
    };

    void EnsureGPUResources();

    std::vector<SpriteCommand> m_Commands;
    std::vector<SpriteVertex> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;

    Matrix3 m_Projection = Matrix3::Identity();

    std::shared_ptr<Shader> m_Shader;

    uint32_t m_VAO = 0;
    uint32_t m_VBO = 0;
    uint32_t m_EBO = 0;

    uint32_t m_BufferOffset = 0; // Offset in vertices for ring buffer

    bool m_Initialized = false;
};

} // namespace SAGE
