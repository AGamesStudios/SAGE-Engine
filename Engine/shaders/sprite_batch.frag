#version 330 core

// ============================================
// SAGE Engine - Sprite Batch Fragment Shader
// ============================================
// Renders textured sprites with color tinting and optional effects

// Input from vertex shader
in vec2 vTexCoord;
in vec4 vColor;
in vec2 vWorldPos;

// Output color
out vec4 FragColor;

// Uniforms
uniform sampler2D uTexture;

// Optional: gamma correction (set to 2.2 for sRGB)
const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

// Gamma correction functions (for proper color blending)
vec3 toLinear(vec3 sRGB) {
    return pow(sRGB, vec3(GAMMA));
}

vec3 toSRGB(vec3 linear) {
    return pow(linear, vec3(INV_GAMMA));
}

void main() {
    // Sample texture with bilinear filtering
    vec4 texColor = texture(uTexture, vTexCoord);
    
    // Multiply by vertex color (tint/alpha modulation)
    vec4 finalColor = texColor * vColor;
    
    // Optional: Gamma correction for proper color blending
    // Enable this if textures look washed out or too bright
    // finalColor.rgb = toLinear(finalColor.rgb);
    // finalColor.rgb = toSRGB(finalColor.rgb);
    
    // Early discard for fully transparent pixels (optimization)
    if (finalColor.a < 0.01) {
        discard;
    }
    
    FragColor = finalColor;
}
