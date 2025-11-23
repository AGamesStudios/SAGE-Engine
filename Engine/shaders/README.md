# SAGE Engine - Shader Library

## Overview
Collection of optimized GLSL shaders for 2D sprite rendering with advanced effects.

## Shader Files

### Basic Sprite Rendering

#### `sprite_batch.vert` / `sprite_batch.frag`
**Purpose:** High-performance batched sprite rendering  
**Features:**
- Optimized for rendering thousands of sprites per frame
- Proper NDC transformation with 3x3 projection matrix
- Vertex color tinting and alpha blending
- Gamma correction support (optional)

**Vertex Attributes:**
- `layout(0)` - `vec2 aPos` - World position
- `layout(1)` - `vec2 aTexCoord` - Texture UV coordinates
- `layout(2)` - `vec4 aColor` - Tint color with alpha

**Uniforms:**
- `mat3 uProjection` - View-projection matrix (world → NDC)
- `sampler2D uTexture` - Sprite texture atlas

**Usage:**
```cpp
shader->SetMat3("uProjection", camera.GetViewProjectionMatrix().m.data());
shader->SetInt("uTexture", 0);
```

---

### Advanced Sprite Effects

#### `advanced_sprite.vert` / `advanced_sprite.frag`
**Purpose:** Feature-rich sprite shader with post-processing effects  
**Features:**
- Outline rendering
- Glow/emissive maps
- Simple 2D lighting
- Normal mapping support
- Per-sprite model matrix transforms

**Additional Uniforms:**
- `mat3 uView` - Camera view matrix
- `mat3 uModel` - Per-sprite model transform
- `bool uUseModelMatrix` - Enable individual sprite transforms
- `bool uEnableOutline` - Enable outline effect
- `vec4 uOutlineColor` - Outline color and thickness
- `bool uEnableGlow` - Enable glow effect
- `vec3 uGlowColor` - Glow color
- `float uGlowIntensity` - Glow strength
- `sampler2D uNormalMap` - Normal map texture
- `sampler2D uEmissiveMap` - Emissive/glow texture
- `vec2 uLightPos` - 2D light position
- `vec3 uLightColor` - Light color
- `float uLightIntensity` - Light strength

**Usage Example:**
```cpp
// Basic setup
shader->SetMat3("uProjection", projection.m.data());
shader->SetMat3("uView", camera.GetViewMatrix().m.data());
shader->SetInt("uTexture", 0);

// Enable outline
shader->SetBool("uEnableOutline", true);
shader->SetVec4("uOutlineColor", {1.0f, 1.0f, 1.0f, 1.0f});
shader->SetFloat("uOutlineWidth", 2.0f);

// Enable glow
shader->SetBool("uEnableGlow", true);
shader->SetVec3("uGlowColor", {1.0f, 0.5f, 0.0f});
shader->SetFloat("uGlowIntensity", 2.5f);

// 2D lighting
shader->SetBool("uEnableLighting", true);
shader->SetVec2("uLightPos", {playerX, playerY});
shader->SetVec3("uLightColor", {1.0f, 0.9f, 0.7f});
shader->SetFloat("uLightIntensity", 1.5f);
```

---

## Mathematical Foundations

### Projection Matrix (3x3 for 2D)

The engine uses 3x3 matrices with homogeneous coordinates for 2D transformations:

```
[sx  0  tx]   [x]   [sx*x + tx]
[ 0 sy  ty] × [y] = [sy*y + ty]
[ 0  0   1]   [1]   [    1    ]
```

### Orthographic Projection

Maps world coordinates to Normalized Device Coordinates (NDC) [-1, 1]:

```cpp
Matrix3::Ortho(left, right, bottom, top):
    sx = 2.0 / (right - left)
    sy = 2.0 / (top - bottom)
    tx = -(right + left) / (right - left)
    ty = -(top + bottom) / (top - bottom)
```

### Vertex Transformation Pipeline

1. **World Space** → Sprite position in game world
2. **View Transform** → Camera position/rotation/zoom
3. **Projection** → Map to NDC [-1, 1]
4. **Clip Space** → GPU clips triangles outside view
5. **Screen Space** → Final pixel coordinates

Shader code:
```glsl
vec3 worldPos = vec3(aPos, 1.0);
vec3 ndc = uProjection * worldPos;
gl_Position = vec4(ndc.xy, 0.0, 1.0);
```

---

## Performance Tips

### Batching
- Sort sprites by texture to minimize texture switches
- Use texture atlas to batch sprites with different textures
- Current engine batches by `(layer, texture)` → optimal draw calls

### Alpha Blending
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

### Discard vs Blending
- Use `discard` for binary alpha (fully opaque or transparent)
- Use blending for smooth transparency (smoke, glass, etc.)

### Shader Variants
Create specialized shaders instead of many conditionals:
- `sprite_batch.frag` - Fast, no effects
- `advanced_sprite.frag` - Full effects, use sparingly

---

## Gamma Correction

Modern displays use sRGB color space (gamma ≈ 2.2). Enable gamma correction if:
- Textures look washed out
- Colors don't blend correctly
- Dark areas are too bright

**Enable in shader:**
```glsl
// In fragment shader
finalColor.rgb = toLinear(finalColor.rgb);  // sRGB → linear
// ... do color math ...
finalColor.rgb = toSRGB(finalColor.rgb);    // linear → sRGB
```

Or enable in OpenGL:
```cpp
glEnable(GL_FRAMEBUFFER_SRGB);  // Automatic sRGB conversion
```

---

## Future Enhancements

- [ ] Compute shader for particle systems
- [ ] Deferred rendering for many lights
- [ ] Shadow mapping for 2D shadows
- [ ] Screen-space reflections
- [ ] Bloom/HDR post-processing
- [ ] Palette swapping shader
- [ ] Distortion effects (heat wave, water)
- [ ] Pixel art scaling shader (nearest + smoothing)

---

## Troubleshooting

### Sprites Not Rendering
- Check projection matrix is set correctly
- Verify texture is bound to slot 0
- Ensure vertices are in correct winding order (CCW)
- Check alpha values (fully transparent = invisible)

### Incorrect Colors
- Verify texture format matches shader expectations
- Check if gamma correction is needed
- Ensure vertex colors are [0, 1], not [0, 255]

### Performance Issues
- Profile draw calls (target: <100 per frame)
- Reduce shader complexity (disable unused features)
- Use texture atlas to increase batch size
- Consider instanced rendering for many identical sprites

### Matrix Math Errors
- Ensure matrices are column-major in memory
- Check multiplication order (OpenGL: right-to-left)
- Use `Matrix3::TransformPoint()` for positions
- Use `Matrix3::TransformVector()` for directions

---

**Last Updated:** 2025-11-17  
**GLSL Version:** 3.30 Core  
**Target:** OpenGL 3.3+
