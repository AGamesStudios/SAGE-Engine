#version 330 core

// ============================================
// SAGE Engine - Advanced Sprite Shader (Vertex)
// ============================================
// Supports multiple rendering modes and advanced transformations

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out VS_OUT {
    vec2 texCoord;
    vec4 color;
    vec2 worldPos;
    vec2 screenPos;
} vs_out;

uniform mat3 uProjection;
uniform mat3 uView;        // Camera view matrix
uniform mat3 uModel;       // Sprite model matrix (optional, for per-sprite transforms)

uniform bool uUseModelMatrix = false;

void main() {
    vec3 pos = vec3(aPos, 1.0);
    
    // Apply model matrix if enabled (for individual sprite transforms)
    if (uUseModelMatrix) {
        pos = uModel * pos;
    }
    
    // Store world position before projection
    vs_out.worldPos = pos.xy;
    
    // Apply view-projection transformation
    vec3 projectedPos = uProjection * pos;
    
    // Final clip-space position
    gl_Position = vec4(projectedPos.xy, 0.0, 1.0);
    vs_out.screenPos = projectedPos.xy;
    
    // Pass through attributes
    vs_out.texCoord = aTexCoord;
    vs_out.color = aColor;
}
