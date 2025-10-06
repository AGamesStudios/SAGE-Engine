#include "Renderer.h"

#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
#ifdef DrawTextW
#undef DrawTextW
#endif

#include <glad/glad.h>
// #include "GLDebug.h"  // TODO: GLAD include issues - disabled for alpha
#include "Shader.h"
#include "Texture.h"
#include "Font.h"
#include "Material.h"
#include "ShaderManager.h"
#include "../Core/Application.h"
#include "../Core/Logger.h"
#include "../Core/Profiler.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <random>
#include <utility>
#include <vector>

namespace SAGE {
namespace {

constexpr std::size_t kMaxQuads = 20000;
constexpr std::size_t kVerticesPerQuad = 4;
constexpr std::size_t kIndicesPerQuad = 6;
constexpr std::size_t kMaxVertices = kMaxQuads * kVerticesPerQuad;
constexpr std::size_t kMaxIndices = kMaxQuads * kIndicesPerQuad;
constexpr float kLayerDepthScale = 0.001f;

struct QuadVertex {
    float position[3];
    float color[4];
    float texCoord[2];
    float pulse[2];
};

struct QuadCommand {
    Vector2 position;
    Vector2 size;
    Vector2 uvMin;
    Vector2 uvMax;
    Color color{1.0f, 1.0f, 1.0f, 1.0f};
    Ref<Texture> texture;
    Ref<Material> material;
    QuadEffect effect{};
    float layer = 0.0f;
    bool screenSpace = false;
};

struct BatchKey {
    Ref<Material> material;
    Ref<Texture> texture;
    bool screenSpace = false;

    bool operator==(const BatchKey& other) const {
        return material == other.material && texture == other.texture && screenSpace == other.screenSpace;
    }
};

struct PostProcessResources {
    PostFXSettings settings{};
    GLuint framebuffer = 0;
    GLuint colorTexture = 0;
    GLuint depthBuffer = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    int width = 0;
    int height = 0;
    Ref<Shader> shader;
};

struct RendererData {
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint quadEBO = 0;

    std::vector<QuadCommand> commandQueue;
    std::vector<float> layerStack;
    std::vector<QuadEffect> effectStack;

    Ref<Material> defaultMaterial;
    Ref<Material> currentMaterial;
    MaterialId defaultMaterialId = 0;
    MaterialId currentMaterialId = 0;
    QuadEffect currentEffect{};
    float currentLayer = 0.0f;

    Camera2D camera{};
    Vector2 cameraShakeOffset = Vector2::Zero();
    float shakeTimer = 0.0f;
    float shakeDuration = 0.0f;
    float shakeStrength = 0.0f;

    float totalTime = 0.0f;
    std::mt19937 rng;

    PostProcessResources post;
};

RendererData s_Data{};
std::array<uint32_t, kMaxIndices> s_Indices{};

// Статистика для профилирования
size_t s_DrawCallsThisFrame = 0;
size_t s_VerticesThisFrame = 0;

float NormalizeLayer(float layer) {
    float clamped = std::clamp(layer, -1000.0f, 1000.0f);
    return std::clamp(-clamped * kLayerDepthScale, -1.0f, 1.0f);
}

float ToNDCX(float x, float width) {
    if (width <= 0.0f) {
        return x;
    }
    return (x / width) * 2.0f - 1.0f;
}

float ToNDCY(float y, float height) {
    if (height <= 0.0f) {
        return y;
    }
    return 1.0f - (y / height) * 2.0f;
}

void ApplyBlendMode(BlendMode mode) {
    switch (mode) {
    case BlendMode::Additive:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Alpha:
    default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    }
}

void EnsureDefaultMaterial();
void EnsurePostProcessResources(int width, int height);
void DestroyPostProcessResources();
void ApplyPostProcess();
void FlushCommands();
void FlushBatch(const BatchKey& key, const std::vector<QuadVertex>& vertices,
                std::size_t quadCount, int textureMode, int hasTexture);
void BuildQuadVertices(const QuadCommand& command, float width, float height,
                       std::vector<QuadVertex>& outVertices);
void QueueQuad(const QuadDesc& desc);
void DecodeUtf8(const std::string& text, std::vector<uint32_t>& outCodepoints);

} // namespace

void Renderer::Init() {
    static bool initialized = false;
    if (initialized) {
        SAGE_WARNING("Renderer::Init() already called!");
        return;
    }
    initialized = true;

    ShaderManager::Init();
    MaterialLibrary::Init();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::random_device rd;
    s_Data.rng.seed(rd());

    s_Data.camera = Camera2D{};
    s_Data.cameraShakeOffset = Vector2::Zero();
    s_Data.shakeTimer = 0.0f;
    s_Data.shakeDuration = 0.0f;
    s_Data.shakeStrength = 0.0f;
    s_Data.totalTime = 0.0f;
    s_Data.currentLayer = 0.0f;
    s_Data.currentEffect = QuadEffect{};

    s_Data.commandQueue.clear();
    s_Data.commandQueue.reserve(kMaxQuads);

    for (std::size_t i = 0; i < kMaxIndices; i += kIndicesPerQuad) {
        std::size_t offset = (i / kIndicesPerQuad) * kVerticesPerQuad;
        s_Indices[i + 0] = static_cast<uint32_t>(offset + 0);
        s_Indices[i + 1] = static_cast<uint32_t>(offset + 1);
        s_Indices[i + 2] = static_cast<uint32_t>(offset + 2);
        s_Indices[i + 3] = static_cast<uint32_t>(offset + 2);
        s_Indices[i + 4] = static_cast<uint32_t>(offset + 3);
        s_Indices[i + 5] = static_cast<uint32_t>(offset + 0);
    }

    glGenVertexArrays(1, &s_Data.quadVAO);
    glBindVertexArray(s_Data.quadVAO);

    glGenBuffers(1, &s_Data.quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, s_Data.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxVertices * sizeof(QuadVertex)), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &s_Data.quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxIndices * sizeof(uint32_t)), s_Indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = static_cast<GLsizei>(sizeof(QuadVertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, color)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, texCoord)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, pulse)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    s_Data.currentMaterial.reset();
    s_Data.defaultMaterial.reset();
    s_Data.currentMaterialId = 0;
    s_Data.defaultMaterialId = 0;

    EnsureDefaultMaterial();

    SAGE_INFO("Renderer initialized (batched mode)");
}

void Renderer::Shutdown() {
    s_Data.commandQueue.clear();
    s_Data.layerStack.clear();
    s_Data.effectStack.clear();
    s_Data.currentMaterial.reset();
    s_Data.defaultMaterial.reset();
    s_Data.currentMaterialId = 0;
    s_Data.defaultMaterialId = 0;

    if (s_Data.quadEBO) {
        glDeleteBuffers(1, &s_Data.quadEBO);
        s_Data.quadEBO = 0;
    }
    if (s_Data.quadVBO) {
        glDeleteBuffers(1, &s_Data.quadVBO);
        s_Data.quadVBO = 0;
    }
    if (s_Data.quadVAO) {
        glDeleteVertexArrays(1, &s_Data.quadVAO);
        s_Data.quadVAO = 0;
    }

    DestroyPostProcessResources();
    s_Data.post.shader.reset();

    MaterialLibrary::Shutdown();
    ShaderManager::Shutdown();

    SAGE_INFO("Renderer shutdown");
}

void Renderer::Update(float deltaTime) {
    s_Data.totalTime += deltaTime;

    if (s_Data.shakeTimer > 0.0f) {
        s_Data.shakeTimer = std::max(0.0f, s_Data.shakeTimer - deltaTime);
        float strength = s_Data.shakeStrength;
        if (s_Data.shakeDuration > 0.0f) {
            float t = s_Data.shakeTimer / s_Data.shakeDuration;
            strength *= t * t;
        }

        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        s_Data.cameraShakeOffset = Vector2(dist(s_Data.rng) * strength, dist(s_Data.rng) * strength);

        if (s_Data.shakeTimer <= 0.0f) {
            s_Data.cameraShakeOffset = Vector2::Zero();
            s_Data.shakeDuration = 0.0f;
            s_Data.shakeStrength = 0.0f;
        }
    }
}

void Renderer::SetCamera(const Camera2D& camera) {
    s_Data.camera.position = camera.position;
    s_Data.camera.zoom = std::max(0.01f, camera.zoom);
}

const Camera2D& Renderer::GetCamera() {
    return s_Data.camera;
}

void Renderer::ResetCamera() {
    s_Data.camera = Camera2D{};
    s_Data.cameraShakeOffset = Vector2::Zero();
    s_Data.shakeTimer = 0.0f;
    s_Data.shakeDuration = 0.0f;
    s_Data.shakeStrength = 0.0f;
}

void Renderer::PushScreenShake(float amplitude, float duration) {
    s_Data.shakeStrength = amplitude;
    s_Data.shakeDuration = std::max(0.0f, duration);
    s_Data.shakeTimer = s_Data.shakeDuration;
}

void Renderer::BeginScene() {
    EnsureDefaultMaterial();
    
    // Сброс статистики кадра
    s_DrawCallsThisFrame = 0;
    s_VerticesThisFrame = 0;
    
    int width = 0;
    int height = 0;

    if (Application::HasInstance()) {
        auto& window = Application::Get().GetWindow();
        width = static_cast<int>(window.GetWidth());
        height = static_cast<int>(window.GetHeight());
    }

    if (width <= 0 || height <= 0) {
        width = s_Data.post.width > 0 ? s_Data.post.width : 1280;
        height = s_Data.post.height > 0 ? s_Data.post.height : 720;
    }

    if (s_Data.post.settings.enabled) {
        EnsurePostProcessResources(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.post.framebuffer);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, width, height);
    s_Data.commandQueue.clear();
}

void Renderer::EndScene() {
    FlushCommands();
    
    // Отправка статистики в Profiler
    Profiler::SetDrawCalls(s_DrawCallsThisFrame);
    Profiler::SetVertexCount(s_VerticesThisFrame);
    Profiler::SetTriangleCount(s_VerticesThisFrame / 2); // 4 вершины = 2 треугольника на quad
    
    if (s_Data.post.settings.enabled) {
        ApplyPostProcess();
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (Application::HasInstance()) {
            auto& window = Application::Get().GetWindow();
            glViewport(0, 0, static_cast<GLsizei>(window.GetWidth()), static_cast<GLsizei>(window.GetHeight()));
        }
    }
    s_Data.commandQueue.clear();
}

void Renderer::Clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Clear() {
    Clear(0.1f, 0.1f, 0.15f, 1.0f);
}

void Renderer::SetLayer(float layer) {
    s_Data.currentLayer = layer;
}

void Renderer::PushLayer(float layer) {
    s_Data.layerStack.push_back(s_Data.currentLayer);
    s_Data.currentLayer = layer;
}

void Renderer::PopLayer() {
    if (!s_Data.layerStack.empty()) {
        s_Data.currentLayer = s_Data.layerStack.back();
        s_Data.layerStack.pop_back();
    }
    else {
        s_Data.currentLayer = 0.0f;
    }
}

MaterialId Renderer::SetMaterial(MaterialId materialId) {
    EnsureDefaultMaterial();
    const MaterialId previousId = s_Data.currentMaterialId;
    Ref<Material> material = MaterialLibrary::Get(materialId);
    if (!material) {
        material = s_Data.defaultMaterial;
        materialId = s_Data.defaultMaterialId;
    }

    s_Data.currentMaterial = material;
    s_Data.currentMaterialId = materialId;
    return previousId;
}

void Renderer::PushEffect(const QuadEffect& effect) {
    s_Data.effectStack.push_back(s_Data.currentEffect);
    s_Data.currentEffect = effect;
}

void Renderer::PopEffect() {
    if (!s_Data.effectStack.empty()) {
        s_Data.currentEffect = s_Data.effectStack.back();
        s_Data.effectStack.pop_back();
    }
    else {
        s_Data.currentEffect = QuadEffect{};
    }
}

void Renderer::ConfigurePostFX(const PostFXSettings& settings) {
    s_Data.post.settings.enabled = settings.enabled;
    s_Data.post.settings.tint = settings.tint;
    s_Data.post.settings.intensity = std::clamp(settings.intensity, 0.0f, 1.0f);
    s_Data.post.settings.bloomThreshold = std::clamp(settings.bloomThreshold, 0.0f, 1.0f);
    s_Data.post.settings.pulseSpeed = std::max(0.0f, settings.pulseSpeed);

    if (!s_Data.post.settings.enabled) {
        DestroyPostProcessResources();
        return;
    }

    if (Application::HasInstance()) {
        auto& window = Application::Get().GetWindow();
        EnsurePostProcessResources(static_cast<int>(window.GetWidth()), static_cast<int>(window.GetHeight()));
    }
}

const PostFXSettings& Renderer::GetPostFXSettings() {
    return s_Data.post.settings;
}

void Renderer::EnablePostFX(bool enabled) {
    if (s_Data.post.settings.enabled == enabled) {
        return;
    }

    s_Data.post.settings.enabled = enabled;
    if (!enabled) {
        DestroyPostProcessResources();
        return;
    }

    if (Application::HasInstance()) {
        auto& window = Application::Get().GetWindow();
        EnsurePostProcessResources(static_cast<int>(window.GetWidth()), static_cast<int>(window.GetHeight()));
    }
}

void Renderer::DrawQuad(const QuadDesc& desc) {
    if (desc.size.x == 0.0f || desc.size.y == 0.0f) {
        return;
    }

    QueueQuad(desc);
}

#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
#ifdef DrawTextW
#undef DrawTextW
#endif

void Renderer::DrawText(const TextDesc& desc) {
    if (desc.text.empty() || !desc.font || !desc.font->IsLoaded()) {
        return;
    }

    const Ref<Texture> atlas = desc.font->GetAtlasTexture();
    if (!atlas || !atlas->IsLoaded()) {
        return;
    }

    std::vector<uint32_t> codepoints;
    DecodeUtf8(desc.text, codepoints);

    float cursorX = desc.position.x;
    float baseline = desc.position.y + desc.font->GetAscent() * desc.scale;

    for (uint32_t codepoint : codepoints) {
        if (codepoint == static_cast<uint32_t>('\n')) {
            cursorX = desc.position.x;
            baseline += desc.font->GetLineHeight() * desc.scale;
            continue;
        }

        const Glyph& glyph = desc.font->GetGlyph(codepoint);
        Float2 glyphPosition(cursorX + glyph.bearing.x * desc.scale,
                             baseline + glyph.bearing.y * desc.scale);
        Float2 glyphSize = glyph.size * desc.scale;

        if (glyphSize.x > 0.0f && glyphSize.y > 0.0f) {
            QuadDesc quad;
            quad.position = glyphPosition;
            quad.size = glyphSize;
            quad.texture = atlas;
            quad.uvMin = glyph.uvMin;
            quad.uvMax = glyph.uvMax;
            quad.color = desc.color;
            quad.screenSpace = desc.screenSpace;
            QueueQuad(quad);
        }

        cursorX += glyph.advance * desc.scale;
    }
}

Float2 Renderer::MeasureText(const std::string& text, const Ref<Font>& font, float scale) {
    if (text.empty() || !font || !font->IsLoaded()) {
        return Float2::Zero();
    }

    std::vector<uint32_t> codepoints;
    DecodeUtf8(text, codepoints);

    float lineWidth = 0.0f;
    float maxWidth = 0.0f;
    std::size_t lineCount = 1;

    for (uint32_t codepoint : codepoints) {
        if (codepoint == static_cast<uint32_t>('\n')) {
            maxWidth = std::max(maxWidth, lineWidth);
            lineWidth = 0.0f;
            ++lineCount;
            continue;
        }

        const Glyph& glyph = font->GetGlyph(codepoint);
        lineWidth += glyph.advance * scale;
    }

    maxWidth = std::max(maxWidth, lineWidth);
    float height = static_cast<float>(lineCount) * font->GetLineHeight() * scale;
    return Float2(maxWidth, height);
}

namespace {

void EnsureDefaultMaterial() {
    if (s_Data.defaultMaterial && s_Data.defaultMaterial->GetShader()) {
        s_Data.defaultMaterialId = s_Data.defaultMaterial->GetId();
        if (!s_Data.currentMaterial) {
            s_Data.currentMaterial = s_Data.defaultMaterial;
            s_Data.currentMaterialId = s_Data.defaultMaterialId;
        }
        return;
    }

    const char* vertexSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec2 a_Pulse;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec2 v_Pulse;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_Pulse = a_Pulse;
    gl_Position = vec4(a_Position, 1.0);
}
)";

    const char* fragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec2 v_Pulse;

uniform sampler2D u_Texture;
uniform int u_HasTexture;
uniform int u_TextureMode;
uniform float u_Time;

void main()
{
    vec4 color = v_Color;
    if (v_Pulse.x > 0.0 && v_Pulse.y > 0.0) {
        float amplitude = clamp(v_Pulse.x, 0.0, 1.0);
        float pulse = sin(u_Time * v_Pulse.y) * 0.5 + 0.5;
        float intensity = mix(1.0, pulse, amplitude);
        color.rgb *= intensity;
    }

    if (u_HasTexture == 1) {
        vec4 texColor = texture(u_Texture, v_TexCoord);
        if (u_TextureMode == 1) {
            texColor = vec4(1.0, 1.0, 1.0, texColor.r);
        }
        color *= texColor;
    }

    o_Color = color;
}
)";

    auto shader = ShaderManager::Load("Renderer2D_Default", vertexSrc, fragmentSrc);
    if (!shader) {
        SAGE_ERROR("Failed to create default renderer shader");
        return;
    }

    shader->Bind();
    shader->SetInt("u_Texture", 0);
    shader->SetInt("u_TextureMode", 0);

    auto material = Material::Create("Renderer2D_Default", shader);
    if (!material) {
        SAGE_ERROR("Failed to create default material for renderer");
        return;
    }

    material->SetTint(Color::White());
    MaterialLibrary::RegisterMaterial(material);
    s_Data.defaultMaterial = material;
    s_Data.currentMaterial = material;
    s_Data.defaultMaterialId = material->GetId();
    s_Data.currentMaterialId = s_Data.defaultMaterialId;
}

void EnsurePostProcessResources(int width, int height) {
    width = std::max(width, 1);
    height = std::max(height, 1);

    if (!s_Data.post.shader) {
        if (ShaderManager::Exists("Renderer2D_PostProcess")) {
            s_Data.post.shader = ShaderManager::Get("Renderer2D_PostProcess");
        }
        else {
            const char* vertexSrc = R"(#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";

        const char* fragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_Scene;
uniform vec4 u_Tint;
uniform float u_Intensity;
uniform float u_BloomThreshold;
uniform float u_PulseSpeed;
uniform float u_Time;

void main()
{
    vec4 scene = texture(u_Scene, v_TexCoord);
    float brightness = max(max(scene.r, scene.g), scene.b);
    float bloomFactor = smoothstep(u_BloomThreshold, 1.0, brightness);
    float pulse = (u_PulseSpeed > 0.0) ? (sin(u_Time * u_PulseSpeed) * 0.5 + 0.5) : 0.0;

    float intensity = clamp(u_Intensity, 0.0, 1.0);
    vec3 tintAdd = u_Tint.rgb * (intensity + pulse * 0.25);
    vec3 color = scene.rgb + tintAdd * scene.a;
    color = mix(color, color * (1.0 + bloomFactor * intensity), 0.35);
    color = clamp(color, 0.0, 1.0);

    o_Color = vec4(color, scene.a);
}
)";

            s_Data.post.shader = ShaderManager::Load("Renderer2D_PostProcess", vertexSrc, fragmentSrc);
        }

        if (s_Data.post.shader) {
            s_Data.post.shader->Bind();
            s_Data.post.shader->SetInt("u_Scene", 0);
            s_Data.post.shader->Unbind();
        }
    }

    bool needsFramebuffer = s_Data.post.framebuffer == 0;
    bool sizeChanged = width != s_Data.post.width || height != s_Data.post.height;

    if (needsFramebuffer) {
        glGenFramebuffers(1, &s_Data.post.framebuffer);
    }

    if (s_Data.post.colorTexture == 0) {
        glGenTextures(1, &s_Data.post.colorTexture);
    }

    if (s_Data.post.depthBuffer == 0) {
        glGenRenderbuffers(1, &s_Data.post.depthBuffer);
    }

    if (needsFramebuffer || sizeChanged) {
        glBindTexture(GL_TEXTURE_2D, s_Data.post.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindRenderbuffer(GL_RENDERBUFFER, s_Data.post.depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.post.framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_Data.post.colorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_Data.post.depthBuffer);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            SAGE_ERROR("Post-process framebuffer incomplete (status: {0})", static_cast<int>(status));
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        s_Data.post.width = width;
        s_Data.post.height = height;
    }

    if (s_Data.post.quadVAO == 0) {
        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &s_Data.post.quadVAO);
        glGenBuffers(1, &s_Data.post.quadVBO);

        glBindVertexArray(s_Data.post.quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_Data.post.quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void DestroyPostProcessResources() {
    if (s_Data.post.quadVBO) {
        glDeleteBuffers(1, &s_Data.post.quadVBO);
        s_Data.post.quadVBO = 0;
    }

    if (s_Data.post.quadVAO) {
        glDeleteVertexArrays(1, &s_Data.post.quadVAO);
        s_Data.post.quadVAO = 0;
    }

    if (s_Data.post.colorTexture) {
        glDeleteTextures(1, &s_Data.post.colorTexture);
        s_Data.post.colorTexture = 0;
    }

    if (s_Data.post.depthBuffer) {
        glDeleteRenderbuffers(1, &s_Data.post.depthBuffer);
        s_Data.post.depthBuffer = 0;
    }

    if (s_Data.post.framebuffer) {
        glDeleteFramebuffers(1, &s_Data.post.framebuffer);
        s_Data.post.framebuffer = 0;
    }

    s_Data.post.width = 0;
    s_Data.post.height = 0;
}

void ApplyPostProcess() {
    if (!s_Data.post.settings.enabled || !s_Data.post.framebuffer || !s_Data.post.shader) {
        return;
    }

    if (!Application::HasInstance()) {
        return;
    }

    auto& window = Application::Get().GetWindow();
    GLsizei width = static_cast<GLsizei>(std::max(1, static_cast<int>(window.GetWidth())));
    GLsizei height = static_cast<GLsizei>(std::max(1, static_cast<int>(window.GetHeight())));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthEnabled) {
        glDisable(GL_DEPTH_TEST);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_Data.post.colorTexture);

    s_Data.post.shader->Bind();
    s_Data.post.shader->SetFloat4("u_Tint",
        s_Data.post.settings.tint.r,
        s_Data.post.settings.tint.g,
        s_Data.post.settings.tint.b,
        s_Data.post.settings.tint.a);
    s_Data.post.shader->SetFloat("u_Intensity", s_Data.post.settings.intensity);
    s_Data.post.shader->SetFloat("u_BloomThreshold", s_Data.post.settings.bloomThreshold);
    s_Data.post.shader->SetFloat("u_PulseSpeed", s_Data.post.settings.pulseSpeed);
    s_Data.post.shader->SetFloat("u_Time", s_Data.totalTime);

    glBindVertexArray(s_Data.post.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    s_Data.post.shader->Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);

    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

void FlushCommands() {
    if (s_Data.commandQueue.empty()) {
        return;
    }

    if (!Application::HasInstance()) {
        s_Data.commandQueue.clear();
        return;
    }

    auto& window = Application::Get().GetWindow();
    float width = static_cast<float>(window.GetWidth());
    float height = static_cast<float>(window.GetHeight());

    if (s_Data.post.settings.enabled && s_Data.post.framebuffer) {
        width = static_cast<float>(std::max(1, s_Data.post.width));
        height = static_cast<float>(std::max(1, s_Data.post.height));
    }

    std::stable_sort(s_Data.commandQueue.begin(), s_Data.commandQueue.end(), [](const QuadCommand& a, const QuadCommand& b) {
        if (std::abs(a.layer - b.layer) > 0.00001f) {
            return a.layer < b.layer;
        }
        std::uint32_t aMat = a.material ? a.material->GetId() : 0;
        std::uint32_t bMat = b.material ? b.material->GetId() : 0;
        if (aMat != bMat) {
            return aMat < bMat;
        }
        if (a.screenSpace != b.screenSpace) {
            return !a.screenSpace && b.screenSpace;
        }
        const Texture* aTex = a.texture ? a.texture.get() : nullptr;
        const Texture* bTex = b.texture ? b.texture.get() : nullptr;
        return aTex < bTex;
    });

    std::vector<QuadVertex> vertexBuffer;
    vertexBuffer.reserve(std::min<std::size_t>(s_Data.commandQueue.size() * kVerticesPerQuad, kMaxVertices));

    BatchKey currentKey{};
    bool hasKey = false;
    std::size_t quadCount = 0;
    int textureMode = 0;
    int hasTexture = 0;

    auto flush = [&]() {
        if (!hasKey || quadCount == 0) {
            vertexBuffer.clear();
            quadCount = 0;
            return;
        }
        FlushBatch(currentKey, vertexBuffer, quadCount, textureMode, hasTexture);
        vertexBuffer.clear();
        quadCount = 0;
    };

    for (const QuadCommand& command : s_Data.commandQueue) {
        BatchKey key{ command.material ? command.material : s_Data.defaultMaterial, command.texture, command.screenSpace };
        if (!hasKey || !(key == currentKey) || quadCount >= kMaxQuads) {
            flush();
            currentKey = key;
            hasKey = true;
            if (command.texture && command.texture->IsLoaded()) {
                hasTexture = 1;
                textureMode = command.texture->GetFormat() == Texture::Format::Red ? 1 : 0;
            }
            else {
                hasTexture = 0;
                textureMode = 0;
            }
        }

        BuildQuadVertices(command, width, height, vertexBuffer);
        ++quadCount;
    }

    flush();
}

void FlushBatch(const BatchKey& key, const std::vector<QuadVertex>& vertices,
                std::size_t quadCount, int textureMode, int hasTexture) {
    if (quadCount == 0 || !key.material) {
        return;
    }

    auto shader = key.material->GetShader();
    if (!shader) {
        return;
    }

    shader->Bind();
    shader->SetFloat("u_Time", s_Data.totalTime);
    shader->SetInt("u_HasTexture", hasTexture);
    shader->SetInt("u_TextureMode", textureMode);

    ApplyBlendMode(key.material->GetBlendMode());

    if (hasTexture == 1 && key.texture && key.texture->IsLoaded()) {
        key.texture->Bind();
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(s_Data.quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_Data.quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(vertices.size() * sizeof(QuadVertex)),
                    vertices.data());

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quadCount * kIndicesPerQuad), GL_UNSIGNED_INT, nullptr);

    // Статистика
    s_DrawCallsThisFrame++;
    s_VerticesThisFrame += quadCount * kVerticesPerQuad;

    glBindVertexArray(0);
}

void BuildQuadVertices(const QuadCommand& command, float width, float height,
                       std::vector<QuadVertex>& outVertices) {
    Vector2 renderPos = command.position;
    Vector2 renderSize = command.size;
    if (!command.screenSpace) {
        Vector2 cameraOffset = s_Data.camera.position + s_Data.cameraShakeOffset;
        float zoom = s_Data.camera.zoom;
        renderPos = (command.position - cameraOffset) * zoom;
        renderSize = command.size * zoom;
    }

    float left = ToNDCX(renderPos.x, width);
    float right = ToNDCX(renderPos.x + renderSize.x, width);
    float top = ToNDCY(renderPos.y, height);
    float bottom = ToNDCY(renderPos.y + renderSize.y, height);
    float depth = NormalizeLayer(command.layer);

    Ref<Material> material = command.material ? command.material : s_Data.defaultMaterial;
    Color tint = material ? material->GetTint() : Color::White();
    Color finalColor(command.color.r * tint.r,
                     command.color.g * tint.g,
                     command.color.b * tint.b,
                     command.color.a * tint.a);

    float amplitude = command.effect.pulseAmplitude > 0.0f ? command.effect.pulseAmplitude : (material ? material->GetPulseAmplitude() : 0.0f);
    float frequency = command.effect.pulseFrequency > 0.0f ? command.effect.pulseFrequency : (material ? material->GetPulseFrequency() : 0.0f);

    amplitude = std::clamp(amplitude, 0.0f, 1.0f);
    frequency = std::max(frequency, 0.0f);

    QuadVertex quad[4] = {
        { { left,  bottom, depth }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a }, { command.uvMin.x, command.uvMax.y }, { amplitude, frequency } },
        { { right, bottom, depth }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a }, { command.uvMax.x, command.uvMax.y }, { amplitude, frequency } },
        { { right, top,    depth }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a }, { command.uvMax.x, command.uvMin.y }, { amplitude, frequency } },
        { { left,  top,    depth }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a }, { command.uvMin.x, command.uvMin.y }, { amplitude, frequency } }
    };

    for (const QuadVertex& vertex : quad) {
        outVertices.emplace_back(vertex);
    }
}

void QueueQuad(const QuadDesc& desc) {
    EnsureDefaultMaterial();

    if (s_Data.commandQueue.size() >= kMaxQuads) {
        FlushCommands();
        s_Data.commandQueue.clear();
    }

    QuadCommand command;
    command.position = desc.position;
    command.size = desc.size;
    command.uvMin = desc.uvMin;
    command.uvMax = desc.uvMax;
    command.color = desc.color;
    command.texture = desc.texture;
    command.material = s_Data.currentMaterial ? s_Data.currentMaterial : s_Data.defaultMaterial;
    command.effect = s_Data.currentEffect;
    command.layer = s_Data.currentLayer;
    command.screenSpace = desc.screenSpace;

    s_Data.commandQueue.emplace_back(std::move(command));
}

void DecodeUtf8(const std::string& text, std::vector<uint32_t>& outCodepoints) {
    constexpr uint32_t kFallbackCodepoint = static_cast<uint32_t>('?');
    outCodepoints.clear();
    outCodepoints.reserve(text.size());

    const std::size_t length = text.size();
    std::size_t i = 0;
    while (i < length) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        if (c < 0x80u) {
            outCodepoints.push_back(static_cast<uint32_t>(c));
            ++i;
            continue;
        }

        std::size_t expectedContinuation = 0;
        uint32_t codepoint = 0;

        if ((c & 0xE0u) == 0xC0u) {
            expectedContinuation = 1;
            codepoint = static_cast<uint32_t>(c & 0x1Fu);
        }
        else if ((c & 0xF0u) == 0xE0u) {
            expectedContinuation = 2;
            codepoint = static_cast<uint32_t>(c & 0x0Fu);
        }
        else if ((c & 0xF8u) == 0xF0u) {
            expectedContinuation = 3;
            codepoint = static_cast<uint32_t>(c & 0x07u);
        }
        else {
            outCodepoints.push_back(kFallbackCodepoint);
            ++i;
            continue;
        }

        bool valid = true;
        for (std::size_t j = 0; j < expectedContinuation; ++j) {
            if (i + j + 1 >= length) {
                valid = false;
                break;
            }
            unsigned char cc = static_cast<unsigned char>(text[i + j + 1]);
            if ((cc & 0xC0u) != 0x80u) {
                valid = false;
                break;
            }
            codepoint = (codepoint << 6) | static_cast<uint32_t>(cc & 0x3Fu);
        }

        if (!valid) {
            outCodepoints.push_back(kFallbackCodepoint);
            ++i;
            continue;
        }

        switch (expectedContinuation) {
        case 1:
            if (codepoint < 0x80u) {
                outCodepoints.push_back(kFallbackCodepoint);
            }
            else {
                outCodepoints.push_back(codepoint);
            }
            break;
        case 2:
            if (codepoint < 0x800u) {
                outCodepoints.push_back(kFallbackCodepoint);
            }
            else {
                outCodepoints.push_back(codepoint);
            }
            break;
        case 3:
            if (codepoint < 0x10000u || (codepoint >= 0xD800u && codepoint <= 0xDFFFu) || codepoint > 0x10FFFFu) {
                outCodepoints.push_back(kFallbackCodepoint);
            }
            else {
                outCodepoints.push_back(codepoint);
            }
            break;
        default:
            outCodepoints.push_back(codepoint);
            break;
        }

        i += expectedContinuation + 1;
    }
}

} // namespace

} // namespace SAGE
