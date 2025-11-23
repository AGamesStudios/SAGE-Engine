#version 330 core

// ============================================
// SAGE Engine - Advanced Sprite Shader (Fragment)
// ============================================
// Supports effects: glow, outline, color grading, distortion

in VS_OUT {
    vec2 texCoord;
    vec4 color;
    vec2 worldPos;
    vec2 screenPos;
} fs_in;

out vec4 FragColor;

// Textures
uniform sampler2D uTexture;
uniform sampler2D uNormalMap;      // Optional normal map
uniform sampler2D uEmissiveMap;    // Optional emissive/glow map

// Effect flags
uniform bool uUseNormalMap = false;
uniform bool uUseEmissive = false;
uniform bool uEnableOutline = false;
uniform bool uEnableGlow = false;

// Effect parameters
uniform vec4 uOutlineColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float uOutlineWidth = 1.0;
uniform vec3 uGlowColor = vec3(1.0, 1.0, 1.0);
uniform float uGlowIntensity = 1.0;
uniform float uTime = 0.0;  // For animated effects

// Lighting (simple 2D lighting)
uniform vec2 uLightPos = vec2(0.0, 0.0);
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
uniform float uLightIntensity = 1.0;
uniform bool uEnableLighting = false;

// Outline detection using texture alpha
vec4 getOutline(sampler2D tex, vec2 uv, float width) {
    float alpha = texture(tex, uv).a;
    
    if (alpha > 0.1) {
        return vec4(0.0);  // Inside sprite, no outline
    }
    
    // Sample surrounding pixels
    vec2 texelSize = width / textureSize(tex, 0);
    float maxAlpha = 0.0;
    
    for (float x = -1.0; x <= 1.0; x += 1.0) {
        for (float y = -1.0; y <= 1.0; y += 1.0) {
            vec2 offset = vec2(x, y) * texelSize;
            maxAlpha = max(maxAlpha, texture(tex, uv + offset).a);
        }
    }
    
    // If neighboring pixel is opaque, this is outline
    if (maxAlpha > 0.1) {
        return uOutlineColor;
    }
    
    return vec4(0.0);
}

// Simple 2D lighting calculation
vec3 calculateLighting(vec3 baseColor, vec2 worldPos) {
    if (!uEnableLighting) {
        return baseColor;
    }
    
    vec2 lightDir = normalize(uLightPos - worldPos);
    float distance = length(uLightPos - worldPos);
    float attenuation = 1.0 / (1.0 + distance * 0.01);
    
    vec3 lighting = uLightColor * uLightIntensity * attenuation;
    return baseColor * lighting;
}

void main() {
    vec4 texColor = texture(uTexture, fs_in.texCoord);
    
    // Apply vertex color tint
    vec4 baseColor = texColor * fs_in.color;
    
    // Outline effect
    if (uEnableOutline) {
        vec4 outline = getOutline(uTexture, fs_in.texCoord, uOutlineWidth);
        if (outline.a > 0.0) {
            FragColor = outline;
            return;
        }
    }
    
    // Emissive/glow
    vec3 emissive = vec3(0.0);
    if (uEnableGlow && uUseEmissive) {
        vec3 emissiveColor = texture(uEmissiveMap, fs_in.texCoord).rgb;
        emissive = emissiveColor * uGlowColor * uGlowIntensity;
    } else if (uEnableGlow) {
        // Simple glow based on brightness
        float brightness = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
        emissive = baseColor.rgb * brightness * uGlowIntensity;
    }
    
    // Lighting
    vec3 litColor = calculateLighting(baseColor.rgb, fs_in.worldPos);
    
    // Combine everything
    vec3 finalColor = litColor + emissive;
    
    // Discard transparent pixels
    if (baseColor.a < 0.01) {
        discard;
    }
    
    FragColor = vec4(finalColor, baseColor.a);
}
