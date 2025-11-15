#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Core/Color.h"

namespace SAGE {

/**
 * @brief Vertex data for custom meshes
 */
struct MeshVertex {
    Float3 position;    // 3D position
    Float4 color;       // RGBA color
    Float2 texCoord;    // UV coordinates
    
    MeshVertex() = default;
    
    MeshVertex(const Float3& pos, const Float4& col = {1.0f, 1.0f, 1.0f, 1.0f}, 
               const Float2& uv = {0.0f, 0.0f})
        : position(pos), color(col), texCoord(uv) {}
    
    MeshVertex(float x, float y, float z = 0.0f, 
               float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : position{x, y, z}, color{r, g, b, a}, texCoord{0.0f, 0.0f} {}
};

/**
 * @brief Mesh data container
 * 
 * Contains vertex and index data for rendering arbitrary geometry.
 * Supports both 2D and 3D meshes.
 */
class Mesh {
public:
    Mesh() = default;
    ~Mesh() = default;
    
    // Add vertex
    void AddVertex(const MeshVertex& vertex);
    void AddVertex(const Float3& position, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    void AddVertex(float x, float y, float z = 0.0f);
    
    // Add triangle indices
    void AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
    
    // Add quad indices (two triangles)
    void AddQuad(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3);
    
    // Setters
    void SetVertices(const std::vector<MeshVertex>& vertices);
    void SetIndices(const std::vector<uint32_t>& indices);
    
    // Getters
    const std::vector<MeshVertex>& GetVertices() const { return m_Vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
    std::vector<MeshVertex>& GetVerticesMutable() { return m_Vertices; }
    std::vector<uint32_t>& GetIndicesMutable() { return m_Indices; }
    
    size_t GetVertexCount() const { return m_Vertices.size(); }
    size_t GetIndexCount() const { return m_Indices.size(); }
    size_t GetTriangleCount() const { return m_Indices.size() / 3; }
    
    bool IsEmpty() const { return m_Vertices.empty(); }
    
    // Clear mesh data
    void Clear();
    
    // Reserve memory
    void ReserveVertices(size_t count);
    void ReserveIndices(size_t count);
    
    // Static factory methods for common shapes
    static Mesh CreateQuad(float width = 1.0f, float height = 1.0f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateCircle(float radius = 1.0f, int segments = 32, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateTriangle(float size = 1.0f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateRegularPolygon(int sides, float radius = 1.0f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateRing(float innerRadius, float outerRadius, int segments = 32, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    
    // Advanced shapes
    static Mesh CreateStar(int points = 5, float outerRadius = 1.0f, float innerRadius = 0.5f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateLine(const Float3& start, const Float3& end, float thickness = 0.1f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    static Mesh CreateGrid(int width, int height, float cellSize = 1.0f, const Float4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    
private:
    std::vector<MeshVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
};

/**
 * @brief GPU mesh resource
 * 
 * Manages OpenGL VAO/VBO/EBO for mesh rendering.
 */
class MeshResource {
public:
    MeshResource();
    ~MeshResource();
    
    // Non-copyable
    MeshResource(const MeshResource&) = delete;
    MeshResource& operator=(const MeshResource&) = delete;
    
    // Movable
    MeshResource(MeshResource&& other) noexcept;
    MeshResource& operator=(MeshResource&& other) noexcept;
    
    // Upload mesh to GPU
    void Upload(const Mesh& mesh, bool dynamic = false);
    
    // Render mesh
    void Render() const;
    
    // Cleanup
    void Destroy();
    
    bool IsValid() const { return m_VAO != 0; }
    
    uint32_t GetVAO() const { return m_VAO; }
    uint32_t GetVBO() const { return m_VBO; }
    uint32_t GetEBO() const { return m_EBO; }
    
    size_t GetIndexCount() const { return m_IndexCount; }
    size_t GetVertexCount() const { return m_VertexCount; }
    
private:
    uint32_t m_VAO = 0;
    uint32_t m_VBO = 0;
    uint32_t m_EBO = 0;
    size_t m_VertexCount = 0;
    size_t m_IndexCount = 0;
};

} // namespace SAGE
