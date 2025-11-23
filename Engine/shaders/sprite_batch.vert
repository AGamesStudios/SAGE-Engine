#version 330 core

// ============================================
// SAGE Engine - Sprite Batch Vertex Shader
// ============================================
// Transforms sprites from world space to NDC using view-projection matrix

// Input vertex attributes
layout (location = 0) in vec2 aPos;        // Position in world space
layout (location = 1) in vec2 aTexCoord;   // Texture coordinates (UV)
layout (location = 2) in vec4 aColor;      // Vertex color/tint

// Output to fragment shader
out vec2 vTexCoord;
out vec4 vColor;
out vec2 vWorldPos;  // For advanced effects (parallax, etc.)

// Uniforms
uniform mat3 uProjection;  // View-Projection matrix (3x3 for 2D)

void main() {
    // Transform position from world space to NDC (Normalized Device Coordinates)
    // Using homogeneous coordinates for 2D: (x, y, 1)
    vec3 worldPosHomogeneous = vec3(aPos, 1.0);
    vec3 projectedPos = uProjection * worldPosHomogeneous;
    
    // Convert to clip space (OpenGL expects vec4)
    // Z=0 for 2D rendering, W=1 for orthographic projection
    gl_Position = vec4(projectedPos.xy, 0.0, 1.0);
    
    // Pass through texture coordinates and color
    vTexCoord = aTexCoord;
    vColor = aColor;
    vWorldPos = aPos;
}
