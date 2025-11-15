#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace SAGE {

/// @brief Vertex attribute type enumeration
enum class VertexAttributeType {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    Mat3,
    Mat4
};

/// @brief Single vertex attribute description
struct VertexAttribute {
    std::string name;
    VertexAttributeType type;
    unsigned int location;
    size_t offset;
    bool normalized;

    VertexAttribute(const std::string& n, VertexAttributeType t, unsigned int loc, size_t off = 0, bool norm = false)
        : name(n), type(t), location(loc), offset(off), normalized(norm) {}

    /// Get component count for the attribute type
    static int GetComponentCount(VertexAttributeType type);
    
    /// Get size in bytes for the attribute type
    static size_t GetSizeInBytes(VertexAttributeType type);
    
    /// Get OpenGL type enum
    static unsigned int GetGLType(VertexAttributeType type);
};

/// @brief Vertex layout definition for flexible vertex attribute configuration
/// Allows custom vertex formats beyond the hardcoded engine defaults
class VertexLayout {
public:
    VertexLayout() = default;
    
    /// Add an attribute to the layout
    /// @param name Attribute name (e.g., "a_Position", "a_Normal")
    /// @param type Attribute data type
    /// @param location Shader location binding
    /// @param normalized Whether integer types should be normalized to [0,1] or [-1,1]
    void AddAttribute(const std::string& name, VertexAttributeType type, unsigned int location, bool normalized = false);
    
    /// Calculate stride (total size of one vertex)
    size_t GetStride() const { return m_Stride; }
    
    /// Get all attributes
    const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }
    
    /// Apply this layout to the current OpenGL VAO
    /// Configures vertex attribute pointers based on the layout
    void Apply() const;
    
    /// Get attribute by name
    const VertexAttribute* FindAttribute(const std::string& name) const;
    
    /// Check if layout has an attribute
    bool HasAttribute(const std::string& name) const;
    
    /// Clear all attributes
    void Clear();
    
    // --- Predefined layouts for common use cases ---
    
    /// Default engine layout: Position(3), Color(4), TexCoord(2), Pulse(2)
    static VertexLayout CreateDefaultBatchLayout();
    
    /// Simple 3D layout: Position(3), Normal(3), TexCoord(2)
    static VertexLayout CreateSimple3DLayout();
    
    /// PBR layout: Position(3), Normal(3), Tangent(3), TexCoord(2)
    static VertexLayout CreatePBRLayout();
    
    /// Skinned mesh layout: Position(3), Normal(3), TexCoord(2), BoneIDs(4), BoneWeights(4)
    static VertexLayout CreateSkinnedLayout();

private:
    std::vector<VertexAttribute> m_Attributes;
    size_t m_Stride = 0;
    
    void RecalculateOffsets();
};

} // namespace SAGE
