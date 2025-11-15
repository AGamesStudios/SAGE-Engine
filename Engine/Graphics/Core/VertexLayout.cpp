#include "VertexLayout.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace SAGE {

// ========== VertexAttribute ==========

int VertexAttribute::GetComponentCount(VertexAttributeType type)
{
    switch (type) {
    case VertexAttributeType::Float:   return 1;
    case VertexAttributeType::Float2:  return 2;
    case VertexAttributeType::Float3:  return 3;
    case VertexAttributeType::Float4:  return 4;
    case VertexAttributeType::Int:     return 1;
    case VertexAttributeType::Int2:    return 2;
    case VertexAttributeType::Int3:    return 3;
    case VertexAttributeType::Int4:    return 4;
    case VertexAttributeType::Mat3:    return 9;  // 3x3
    case VertexAttributeType::Mat4:    return 16; // 4x4
    default: return 0;
    }
}

size_t VertexAttribute::GetSizeInBytes(VertexAttributeType type)
{
    switch (type) {
    case VertexAttributeType::Float:   return 4;
    case VertexAttributeType::Float2:  return 8;
    case VertexAttributeType::Float3:  return 12;
    case VertexAttributeType::Float4:  return 16;
    case VertexAttributeType::Int:     return 4;
    case VertexAttributeType::Int2:    return 8;
    case VertexAttributeType::Int3:    return 12;
    case VertexAttributeType::Int4:    return 16;
    case VertexAttributeType::Mat3:    return 36;  // 9 floats
    case VertexAttributeType::Mat4:    return 64;  // 16 floats
    default: return 0;
    }
}

unsigned int VertexAttribute::GetGLType(VertexAttributeType type)
{
    switch (type) {
    case VertexAttributeType::Float:
    case VertexAttributeType::Float2:
    case VertexAttributeType::Float3:
    case VertexAttributeType::Float4:
    case VertexAttributeType::Mat3:
    case VertexAttributeType::Mat4:
        return GL_FLOAT;
        
    case VertexAttributeType::Int:
    case VertexAttributeType::Int2:
    case VertexAttributeType::Int3:
    case VertexAttributeType::Int4:
        return GL_INT;
        
    default:
        return GL_FLOAT;
    }
}

// ========== VertexLayout ==========

void VertexLayout::AddAttribute(const std::string& name, VertexAttributeType type, unsigned int location, bool normalized)
{
    size_t offset = m_Stride;
    m_Attributes.emplace_back(name, type, location, offset, normalized);
    m_Stride += VertexAttribute::GetSizeInBytes(type);
}

void VertexLayout::Apply() const
{
    if (m_Attributes.empty()) {
        SAGE_WARNING("VertexLayout::Apply called on empty layout");
        return;
    }
    
    for (const auto& attr : m_Attributes) {
        const unsigned int location = attr.location;
        const int componentCount = VertexAttribute::GetComponentCount(attr.type);
        const unsigned int glType = VertexAttribute::GetGLType(attr.type);
        const GLboolean normalized = attr.normalized ? GL_TRUE : GL_FALSE;
        const GLsizei stride = static_cast<GLsizei>(m_Stride);
        const void* pointer = reinterpret_cast<const void*>(attr.offset);
        
        glEnableVertexAttribArray(location);
        
        // Handle matrices specially - they need multiple attribute slots
        if (attr.type == VertexAttributeType::Mat3) {
            // Mat3 uses 3 vec3 attributes
            for (int i = 0; i < 3; ++i) {
                glEnableVertexAttribArray(location + i);
                glVertexAttribPointer(location + i, 3, GL_FLOAT, GL_FALSE, stride,
                                     reinterpret_cast<const void*>(attr.offset + i * 12));
            }
        } else if (attr.type == VertexAttributeType::Mat4) {
            // Mat4 uses 4 vec4 attributes
            for (int i = 0; i < 4; ++i) {
                glEnableVertexAttribArray(location + i);
                glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, stride,
                                     reinterpret_cast<const void*>(attr.offset + i * 16));
            }
        } else {
            // Regular attributes
            if (glType == GL_INT) {
                glVertexAttribIPointer(location, componentCount, glType, stride, pointer);
            } else {
                glVertexAttribPointer(location, componentCount, glType, normalized, stride, pointer);
            }
        }
    }
}

const VertexAttribute* VertexLayout::FindAttribute(const std::string& name) const
{
    for (const auto& attr : m_Attributes) {
        if (attr.name == name) {
            return &attr;
        }
    }
    return nullptr;
}

bool VertexLayout::HasAttribute(const std::string& name) const
{
    return FindAttribute(name) != nullptr;
}

void VertexLayout::Clear()
{
    m_Attributes.clear();
    m_Stride = 0;
}

void VertexLayout::RecalculateOffsets()
{
    m_Stride = 0;
    for (auto& attr : m_Attributes) {
        attr.offset = m_Stride;
        m_Stride += VertexAttribute::GetSizeInBytes(attr.type);
    }
}

// ========== Predefined Layouts ==========

VertexLayout VertexLayout::CreateDefaultBatchLayout()
{
    VertexLayout layout;
    layout.AddAttribute("a_Position", VertexAttributeType::Float3, 0);
    layout.AddAttribute("a_Color", VertexAttributeType::Float4, 1);
    layout.AddAttribute("a_TexCoord", VertexAttributeType::Float2, 2);
    layout.AddAttribute("a_Pulse", VertexAttributeType::Float2, 3);
    return layout;
}

VertexLayout VertexLayout::CreateSimple3DLayout()
{
    VertexLayout layout;
    layout.AddAttribute("a_Position", VertexAttributeType::Float3, 0);
    layout.AddAttribute("a_Normal", VertexAttributeType::Float3, 1);
    layout.AddAttribute("a_TexCoord", VertexAttributeType::Float2, 2);
    return layout;
}

VertexLayout VertexLayout::CreatePBRLayout()
{
    VertexLayout layout;
    layout.AddAttribute("a_Position", VertexAttributeType::Float3, 0);
    layout.AddAttribute("a_Normal", VertexAttributeType::Float3, 1);
    layout.AddAttribute("a_Tangent", VertexAttributeType::Float3, 2);
    layout.AddAttribute("a_TexCoord", VertexAttributeType::Float2, 3);
    return layout;
}

VertexLayout VertexLayout::CreateSkinnedLayout()
{
    VertexLayout layout;
    layout.AddAttribute("a_Position", VertexAttributeType::Float3, 0);
    layout.AddAttribute("a_Normal", VertexAttributeType::Float3, 1);
    layout.AddAttribute("a_TexCoord", VertexAttributeType::Float2, 2);
    layout.AddAttribute("a_BoneIDs", VertexAttributeType::Int4, 3);
    layout.AddAttribute("a_BoneWeights", VertexAttributeType::Float4, 4);
    return layout;
}

} // namespace SAGE
