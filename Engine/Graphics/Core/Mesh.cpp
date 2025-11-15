#include "Mesh.h"
#include "Core/Log.h"
#include <glad/glad.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SAGE {

// ============================================================================
// Mesh Implementation
// ============================================================================

void Mesh::AddVertex(const MeshVertex& vertex) {
    m_Vertices.push_back(vertex);
}

void Mesh::AddVertex(const Float3& position, const Float4& color) {
    m_Vertices.emplace_back(position, color);
}

void Mesh::AddVertex(float x, float y, float z) {
    m_Vertices.emplace_back(Float3{x, y, z}, Float4{1.0f, 1.0f, 1.0f, 1.0f});
}

void Mesh::AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
    m_Indices.push_back(i0);
    m_Indices.push_back(i1);
    m_Indices.push_back(i2);
}

void Mesh::AddQuad(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) {
    // First triangle
    m_Indices.push_back(i0);
    m_Indices.push_back(i1);
    m_Indices.push_back(i2);
    
    // Second triangle
    m_Indices.push_back(i0);
    m_Indices.push_back(i2);
    m_Indices.push_back(i3);
}

void Mesh::SetVertices(const std::vector<MeshVertex>& vertices) {
    m_Vertices = vertices;
}

void Mesh::SetIndices(const std::vector<uint32_t>& indices) {
    m_Indices = indices;
}

void Mesh::Clear() {
    m_Vertices.clear();
    m_Indices.clear();
}

void Mesh::ReserveVertices(size_t count) {
    m_Vertices.reserve(count);
}

void Mesh::ReserveIndices(size_t count) {
    m_Indices.reserve(count);
}

// ============================================================================
// Shape Generators
// ============================================================================

Mesh Mesh::CreateQuad(float width, float height, const Float4& color) {
    Mesh mesh;
    mesh.ReserveVertices(4);
    mesh.ReserveIndices(6);
    
    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;
    
    // Vertices (counter-clockwise from bottom-left)
    mesh.AddVertex(Float3{-halfWidth, -halfHeight, 0.0f}, color);  // 0: bottom-left
    mesh.AddVertex(Float3{ halfWidth, -halfHeight, 0.0f}, color);  // 1: bottom-right
    mesh.AddVertex(Float3{ halfWidth,  halfHeight, 0.0f}, color);  // 2: top-right
    mesh.AddVertex(Float3{-halfWidth,  halfHeight, 0.0f}, color);  // 3: top-left
    
    // Indices (two triangles)
    mesh.AddQuad(0, 1, 2, 3);
    
    return mesh;
}

Mesh Mesh::CreateCircle(float radius, int segments, const Float4& color) {
    if (segments < 3) segments = 3;
    
    Mesh mesh;
    mesh.ReserveVertices(segments + 1);  // +1 for center
    mesh.ReserveIndices(segments * 3);
    
    // Center vertex
    mesh.AddVertex(Float3{0.0f, 0.0f, 0.0f}, color);
    
    // Perimeter vertices
    for (int i = 0; i < segments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * static_cast<float>(M_PI);
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        mesh.AddVertex(Float3{x, y, 0.0f}, color);
    }
    
    // Triangles (fan from center)
    for (int i = 0; i < segments; ++i) {
        uint32_t next = (i + 1) % segments;
        mesh.AddTriangle(0, i + 1, next + 1);
    }
    
    return mesh;
}

Mesh Mesh::CreateTriangle(float size, const Float4& color) {
    Mesh mesh;
    mesh.ReserveVertices(3);
    mesh.ReserveIndices(3);
    
    const float height = size * std::sqrt(3.0f) / 2.0f;
    const float halfSize = size * 0.5f;
    
    // Equilateral triangle centered at origin
    mesh.AddVertex(Float3{ 0.0f,      height * 0.666f, 0.0f}, color);  // top
    mesh.AddVertex(Float3{-halfSize, -height * 0.333f, 0.0f}, color);  // bottom-left
    mesh.AddVertex(Float3{ halfSize, -height * 0.333f, 0.0f}, color);  // bottom-right
    
    mesh.AddTriangle(0, 1, 2);
    
    return mesh;
}

Mesh Mesh::CreateRegularPolygon(int sides, float radius, const Float4& color) {
    if (sides < 3) sides = 3;
    
    Mesh mesh;
    mesh.ReserveVertices(sides + 1);
    mesh.ReserveIndices(sides * 3);
    
    // Center vertex
    mesh.AddVertex(Float3{0.0f, 0.0f, 0.0f}, color);
    
    // Perimeter vertices
    for (int i = 0; i < sides; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(sides)) * 2.0f * static_cast<float>(M_PI);
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        mesh.AddVertex(Float3{x, y, 0.0f}, color);
    }
    
    // Triangles
    for (int i = 0; i < sides; ++i) {
        uint32_t next = (i + 1) % sides;
        mesh.AddTriangle(0, i + 1, next + 1);
    }
    
    return mesh;
}

Mesh Mesh::CreateRing(float innerRadius, float outerRadius, int segments, const Float4& color) {
    if (segments < 3) segments = 3;
    if (innerRadius > outerRadius) std::swap(innerRadius, outerRadius);
    
    Mesh mesh;
    mesh.ReserveVertices(segments * 2);
    mesh.ReserveIndices(segments * 6);
    
    // Create vertices
    for (int i = 0; i < segments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * static_cast<float>(M_PI);
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        
        // Inner vertex
        mesh.AddVertex(Float3{innerRadius * cosA, innerRadius * sinA, 0.0f}, color);
        // Outer vertex
        mesh.AddVertex(Float3{outerRadius * cosA, outerRadius * sinA, 0.0f}, color);
    }
    
    // Create quads
    for (int i = 0; i < segments; ++i) {
        uint32_t inner1 = i * 2;
        uint32_t outer1 = i * 2 + 1;
        uint32_t inner2 = ((i + 1) % segments) * 2;
        uint32_t outer2 = ((i + 1) % segments) * 2 + 1;
        
        mesh.AddQuad(inner1, outer1, outer2, inner2);
    }
    
    return mesh;
}

Mesh Mesh::CreateStar(int points, float outerRadius, float innerRadius, const Float4& color) {
    if (points < 3) points = 3;
    
    Mesh mesh;
    const int totalPoints = points * 2;
    mesh.ReserveVertices(totalPoints + 1);
    mesh.ReserveIndices(totalPoints * 3);
    
    // Center vertex
    mesh.AddVertex(Float3{0.0f, 0.0f, 0.0f}, color);
    
    // Alternating outer and inner vertices
    for (int i = 0; i < totalPoints; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(totalPoints)) * 2.0f * static_cast<float>(M_PI);
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        mesh.AddVertex(Float3{x, y, 0.0f}, color);
    }
    
    // Triangles
    for (int i = 0; i < totalPoints; ++i) {
        uint32_t next = (i + 1) % totalPoints;
        mesh.AddTriangle(0, i + 1, next + 1);
    }
    
    return mesh;
}

Mesh Mesh::CreateLine(const Float3& start, const Float3& end, float thickness, const Float4& color) {
    Mesh mesh;
    mesh.ReserveVertices(4);
    mesh.ReserveIndices(6);
    
    // Calculate perpendicular vector
    Float3 direction = {end.x - start.x, end.y - start.y, end.z - start.z};
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length < 0.0001f) {
        // Degenerate line, create tiny quad
        return CreateQuad(thickness, thickness, color);
    }
    
    direction.x /= length;
    direction.y /= length;
    
    // Perpendicular vector
    Float3 perp = {-direction.y, direction.x, 0.0f};
    float halfThickness = thickness * 0.5f;
    
    // Four corners
    mesh.AddVertex(Float3{start.x + perp.x * halfThickness, start.y + perp.y * halfThickness, start.z}, color);
    mesh.AddVertex(Float3{start.x - perp.x * halfThickness, start.y - perp.y * halfThickness, start.z}, color);
    mesh.AddVertex(Float3{end.x - perp.x * halfThickness, end.y - perp.y * halfThickness, end.z}, color);
    mesh.AddVertex(Float3{end.x + perp.x * halfThickness, end.y + perp.y * halfThickness, end.z}, color);
    
    mesh.AddQuad(0, 1, 2, 3);
    
    return mesh;
}

Mesh Mesh::CreateGrid(int width, int height, float cellSize, const Float4& color) {
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    
    Mesh mesh;
    const int lineCount = (width + 1) + (height + 1);
    mesh.ReserveVertices(lineCount * 2);
    mesh.ReserveIndices(lineCount * 2);
    
    const float totalWidth = width * cellSize;
    const float totalHeight = height * cellSize;
    const float halfWidth = totalWidth * 0.5f;
    const float halfHeight = totalHeight * 0.5f;
    
    // Vertical lines
    for (int x = 0; x <= width; ++x) {
        float xPos = x * cellSize - halfWidth;
        uint32_t v0 = static_cast<uint32_t>(mesh.GetVertexCount());
        mesh.AddVertex(Float3{xPos, -halfHeight, 0.0f}, color);
        mesh.AddVertex(Float3{xPos,  halfHeight, 0.0f}, color);
        mesh.AddTriangle(v0, v0 + 1, v0);  // Degenerate triangle for line rendering
    }
    
    // Horizontal lines
    for (int y = 0; y <= height; ++y) {
        float yPos = y * cellSize - halfHeight;
        uint32_t v0 = static_cast<uint32_t>(mesh.GetVertexCount());
        mesh.AddVertex(Float3{-halfWidth, yPos, 0.0f}, color);
        mesh.AddVertex(Float3{ halfWidth, yPos, 0.0f}, color);
        mesh.AddTriangle(v0, v0 + 1, v0);
    }
    
    return mesh;
}

// ============================================================================
// MeshResource Implementation
// ============================================================================

MeshResource::MeshResource() = default;

MeshResource::~MeshResource() {
    Destroy();
}

MeshResource::MeshResource(MeshResource&& other) noexcept
    : m_VAO(other.m_VAO)
    , m_VBO(other.m_VBO)
    , m_EBO(other.m_EBO)
    , m_VertexCount(other.m_VertexCount)
    , m_IndexCount(other.m_IndexCount)
{
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_VertexCount = 0;
    other.m_IndexCount = 0;
}

MeshResource& MeshResource::operator=(MeshResource&& other) noexcept {
    if (this != &other) {
        Destroy();
        
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_VertexCount = other.m_VertexCount;
        m_IndexCount = other.m_IndexCount;
        
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_VertexCount = 0;
        other.m_IndexCount = 0;
    }
    return *this;
}

void MeshResource::Upload(const Mesh& mesh, bool dynamic) {
    if (mesh.IsEmpty()) {
        SAGE_WARN("MeshResource::Upload - Mesh is empty");
        return;
    }
    
    // Clean up old resources if exists
    Destroy();
    
    const auto& vertices = mesh.GetVertices();
    const auto& indices = mesh.GetIndices();
    
    m_VertexCount = vertices.size();
    m_IndexCount = indices.size();
    
    // Create VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    // Create and upload VBO
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 vertices.size() * sizeof(MeshVertex), 
                 vertices.data(), 
                 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    
    // Create and upload EBO
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 indices.size() * sizeof(uint32_t), 
                 indices.data(), 
                 GL_STATIC_DRAW);
    
    // Setup vertex attributes
    constexpr GLsizei stride = sizeof(MeshVertex);
    
    // Position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 
                         reinterpret_cast<const void*>(offsetof(MeshVertex, position)));
    
    // Color (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, 
                         reinterpret_cast<const void*>(offsetof(MeshVertex, color)));
    
    // TexCoord (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, 
                         reinterpret_cast<const void*>(offsetof(MeshVertex, texCoord)));
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SAGE_ERROR("MeshResource::Upload - OpenGL error: {}", error);
        Destroy();
    }
}

void MeshResource::Render() const {
    if (!IsValid()) {
        SAGE_WARN("MeshResource::Render - Invalid mesh resource");
        return;
    }
    
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_IndexCount), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void MeshResource::Destroy() {
    if (m_EBO) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    m_VertexCount = 0;
    m_IndexCount = 0;
}

} // namespace SAGE
