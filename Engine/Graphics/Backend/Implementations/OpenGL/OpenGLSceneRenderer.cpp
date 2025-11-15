#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Resources/Shader.h"
// Clean implementation with functional batching integration.
#include "OpenGLSceneRenderer.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Graphics/Backend/Implementations/OpenGL/Utils/GLErrorScope.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/Rendering/Batching/BatchRenderer.h"
#include "Graphics/Rendering/Batching/CommandBuffer.h"
#include "Graphics/Interfaces/IShaderManager.h"
#include "Core/ServiceLocator.h"
#include "Core/Logger.h"
#include "Core/UTF8Utils.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace SAGE::Graphics {

struct OpenGLSceneRenderer::Impl {
    // Backend (low-level) pointer
    IRenderBackend* backend = nullptr;
    bool initialized = false;

    // Camera
    Camera2D camera;

    // Layer & state stacks
    float currentLayer = 0.0f;
    std::vector<float> layerStack;
    BlendMode currentBlendMode = BlendMode::Alpha;
    std::vector<BlendMode> blendStack;
    DepthSettings currentDepthState{};
    std::vector<DepthSettings> depthStack;
    std::vector<QuadEffect> effectStack;
    QuadEffect currentEffect{};
    // Material tracking (currently by id; real binding to shader will occur later)
    MaterialId currentMaterialId = 0; // 0 treated as 'default'
    Ref<Material> currentMaterialRef; // cached Ref for commands

    // PostFX
    PostFXSettings postFXSettings{};
    bool postFXEnabled = false;
    // PostFX GPU resources
    unsigned int postFXFramebuffer = 0;
    unsigned int postFXColorTexture = 0;
    unsigned int postFXDepthRenderbuffer = 0;
    std::vector<unsigned int> postFXBlurTextures; // ping-pong для blur iterations
    Ref<Shader> postFXShader;
    unsigned int postFXVAO = 0; // для fullscreen quad
    unsigned int postFXVBO = 0;
    bool postFXResourcesReady = false;
    int postFXViewportWidth = 0;  // track resolution для resize
    int postFXViewportHeight = 0;

    // Batching subsystem
    Batching::BatchRenderer batchRenderer;
    bool batchInitialized = false;
    std::size_t maxQuadsHint = Batching::BatchRenderer::kDefaultMaxQuads;
    bool allowDynamicBatchResize = true;

    // Cached matrices (recomputed each flush until projection hook improves this)
    Matrix4 projection{ Matrix4::Identity() };
    bool projectionDirty = true; // mark when viewport or camera parameters change
    Matrix4 view{ Matrix4::Identity() };
    Matrix4 viewProjection{ Matrix4::Identity() };
    Matrix4 screenProjection{ Matrix4::Identity() };

    // Metrics (per-scene)
    std::size_t quadCountThisScene = 0;      // logical quad requests
    std::size_t textQuadCountThisScene = 0;  // logical text requests (will become glyph quads later)
    std::size_t tileQuadCountThisScene = 0;  // logical tile requests (subset of quadCount)
    // Batch flush counters (accumulated during EndScene)
    std::size_t drawCalls = 0;
    std::size_t vertices = 0;
    std::size_t triangles = 0;

    // Timing
    float elapsedTime = 0.0f;

    // Runtime state flags
    bool defaultMaterialReady = false;
    Ref<Shader> defaultBatchShader;
    
    // Default white texture (1x1 white pixel for untextured quads)
    unsigned int whiteTexture = 0;
    bool whiteTextureCreated = false;

    // Screen shake (simple version; will migrate to stack later)
    float shakeAmplitude = 0.0f;
    float shakeFrequency = 0.0f;
    float shakeDuration = 0.0f;
    float shakeTimer = 0.0f;
    Vector2 shakeOffset = Vector2::Zero();

    Ref<Shader> AcquireDefaultBatchShader();
    Ref<Shader> AcquirePostFXShader();
    bool EnsureDefaultSpriteMaterial();
    void CreateWhiteTexture();
    void DestroyWhiteTexture();
};

// Internal local helper lambda (defined later) will perform flush; removed global helper to avoid private access issues.

namespace {

constexpr const char* kDefaultBatchShaderName = "Renderer.DefaultBatch";
constexpr const char* kDefaultBatchVertexPath = "assets/shaders/DefaultBatch.vert";
constexpr const char* kDefaultBatchFragmentPath = "assets/shaders/DefaultBatch.frag";

constexpr const char* kPostFXShaderName = "Renderer.PostFX";
constexpr const char* kPostFXVertexPath = "assets/shaders/PostFX.vert";
constexpr const char* kPostFXFragmentPath = "assets/shaders/PostFX.frag";

constexpr int kPostFXSceneTextureSlot = 0;

IShaderManager* TryGetShaderManager()
{
    if (!ServiceLocator::HasGlobalInstance()) {
        return nullptr;
    }

    auto& services = ServiceLocator::GetGlobalInstance();
    if (!services.HasShaderManager()) {
        return nullptr;
    }

    return &services.GetShaderManager();
}

void LabelGlObject(GLenum identifier, GLuint handle, const std::string& label)
{
    if (handle == 0 || label.empty()) {
        return;
    }

    if (glObjectLabel) {
        glObjectLabel(identifier, handle, static_cast<GLsizei>(label.size()), label.c_str());
    }
}

const char* s_DefaultBatchVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec2 a_Pulse; // x = amplitude, y = frequency

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec2 v_Pulse;

void main() {
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_Pulse = a_Pulse;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}
)GLSL";

const char* s_DefaultBatchFragmentShader = R"GLSL(
#version 330 core
in vec4 v_Color;
in vec2 v_TexCoord;
in vec2 v_Pulse;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform int u_HasTexture;
uniform int u_TextureMode; // 0 = RGBA, 1 = RED-only atlas
uniform float u_Time;

vec4 SampleTexture() {
    if (u_HasTexture == 0) {
        return vec4(1.0);
    }

    vec4 tex = texture(u_Texture, v_TexCoord);
    if (u_TextureMode == 1) {
        tex = vec4(tex.rrr, tex.r);
    }
    return tex;
}

float ComputePulseScale() {
    float amplitude = max(v_Pulse.x, 0.0);
    float frequency = max(v_Pulse.y, 0.0);
    if (amplitude <= 0.0001 || frequency <= 0.0001) {
        return 1.0;
    }
    float pulse = sin(u_Time * frequency);
    return 1.0 + amplitude * pulse;
}

void main() {
    vec4 baseColor = v_Color * SampleTexture();
    baseColor.rgb *= ComputePulseScale();
    FragColor = baseColor;
}
)GLSL";

const char* s_PostFXVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)GLSL";

const char* s_PostFXFragmentShader = R"GLSL(
#version 330 core
in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_SceneTexture;
uniform vec4 u_Tint;
uniform float u_Intensity;
uniform float u_Gamma;
uniform float u_Exposure;
uniform float u_PulseTime;

void main() {
    vec4 color = texture(u_SceneTexture, v_TexCoord);
    
    // Exposure tone mapping
    color.rgb *= u_Exposure;
    
    // Gamma correction
    color.rgb = pow(color.rgb, vec3(1.0 / u_Gamma));
    
    // Tint overlay (additive blending with intensity)
    color.rgb = mix(color.rgb, color.rgb + u_Tint.rgb, u_Intensity * u_Tint.a);
    
    // Pulse effect (optional, if pulseSpeed > 0)
    if (u_PulseTime > 0.0) {
        float pulse = 0.5 + 0.5 * sin(u_PulseTime);
        color.rgb *= (0.9 + 0.1 * pulse);
    }
    
    FragColor = color;
}
)GLSL";

} // namespace

Ref<Shader> OpenGLSceneRenderer::Impl::AcquireDefaultBatchShader()
{
    Ref<Shader> shader;

    if (auto* shaderManager = TryGetShaderManager()) {
        shader = shaderManager->Get(kDefaultBatchShaderName);
        if (!shader || !shader->IsValid()) {
            shader = shaderManager->LoadFromFile(kDefaultBatchShaderName, kDefaultBatchVertexPath, kDefaultBatchFragmentPath);
        }
    }

    if (!shader || !shader->IsValid()) {
        if (defaultBatchShader && defaultBatchShader->IsValid()) {
            shader = defaultBatchShader;
        } else {
            shader = CreateRef<Shader>(s_DefaultBatchVertexShader, s_DefaultBatchFragmentShader);
            if (!shader || !shader->IsValid()) {
                SAGE_ERROR("Failed to create default batch shader");
                shader.reset();
            }
        }
    }

    if (shader && defaultBatchShader != shader) {
        defaultBatchShader = shader;
    } else if (!shader) {
        defaultBatchShader.reset();
    }

    return defaultBatchShader;
}

Ref<Shader> OpenGLSceneRenderer::Impl::AcquirePostFXShader()
{
    Ref<Shader> shader;

    if (auto* shaderManager = TryGetShaderManager()) {
        shader = shaderManager->Get(kPostFXShaderName);
        if (!shader || !shader->IsValid()) {
            shader = shaderManager->LoadFromFile(kPostFXShaderName, kPostFXVertexPath, kPostFXFragmentPath);
        }
    }

    if (!shader || !shader->IsValid()) {
        shader = CreateRef<Shader>(s_PostFXVertexShader, s_PostFXFragmentShader);
        if (!shader || !shader->IsValid()) {
            SAGE_ERROR("Failed to create PostFX shader");
            return nullptr;
        }
    }

    return shader;
}

bool OpenGLSceneRenderer::Impl::EnsureDefaultSpriteMaterial()
{
    const unsigned char* version = glGetString(GL_VERSION);
    if (!version) {
        SAGE_WARNING("EnsureDefaultSpriteMaterial: OpenGL context is not current; deferring material creation");
        return false;
    }

    Ref<Shader> shader = AcquireDefaultBatchShader();
    if (!shader) {
        return false;
    }

    Ref<Material> defaultMaterial = MaterialLibrary::GetDefault();
    if (defaultMaterial) {
        defaultMaterial->SetShader(shader);
        defaultMaterial->SetBlendMode(BlendMode::Alpha);
        return defaultMaterial->GetShader() == shader;
    }

    auto material = Material::Create("DefaultSpriteMaterial", shader);
    if (!material) {
        SAGE_ERROR("Failed to create default sprite material");
        return false;
    }
    material->SetBlendMode(BlendMode::Alpha);
    auto registered = MaterialLibrary::RegisterMaterial(material);
    if (!registered) {
        SAGE_ERROR("Failed to register default sprite material");
        return false;
    }
    return true;
}

void OpenGLSceneRenderer::Impl::CreateWhiteTexture() {
    if (whiteTextureCreated) return;
    
    GLErrorScope errorScope("CreateWhiteTexture");
    
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    
    // 1x1 white pixel
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    LabelGlObject(GL_TEXTURE, whiteTexture, "DefaultWhiteTexture");
    whiteTextureCreated = true;
    
    SAGE_INFO("Created default white texture (1x1)");
}

void OpenGLSceneRenderer::Impl::DestroyWhiteTexture() {
    if (whiteTexture) {
        glDeleteTextures(1, &whiteTexture);
        whiteTexture = 0;
    }
    whiteTextureCreated = false;
}
OpenGLSceneRenderer::OpenGLSceneRenderer(IRenderBackend* backend)
    : m_Impl(std::make_unique<Impl>())
{
    m_Impl->backend = backend;
}

OpenGLSceneRenderer::~OpenGLSceneRenderer() {
    Shutdown();
}

void OpenGLSceneRenderer::Init() {
    if (m_Impl->initialized) {
        SAGE_WARNING("OpenGLSceneRenderer already initialized");
        return;
    }
    // Initialize batching
    m_Impl->batchRenderer.Initialize(m_Impl->maxQuadsHint, m_Impl->allowDynamicBatchResize);
    m_Impl->batchInitialized = true;
    
    // Initialize default material once
    m_Impl->defaultMaterialReady = m_Impl->EnsureDefaultSpriteMaterial();
    
    // Create default white texture
    m_Impl->CreateWhiteTexture();
    
    m_Impl->initialized = true;
    SAGE_INFO("OpenGLSceneRenderer initialized (batch maxQuads={}, dynamicResize={})", m_Impl->maxQuadsHint, m_Impl->allowDynamicBatchResize);
}

void OpenGLSceneRenderer::Shutdown() {
    if (!m_Impl->initialized) return;
    
    // Cleanup PostFX resources if active
    if (m_Impl->postFXResourcesReady) {
        DestroyPostFXResources();
    }
    
    // Destroy white texture
    m_Impl->DestroyWhiteTexture();
    
    if (m_Impl->batchInitialized) {
        m_Impl->batchRenderer.Shutdown();
        m_Impl->batchInitialized = false;
    }
    m_Impl->defaultBatchShader.reset();
    m_Impl->defaultMaterialReady = false;
    m_Impl->currentMaterialRef.reset();
    m_Impl->postFXShader.reset();
    m_Impl->initialized = false;
    SAGE_INFO("OpenGLSceneRenderer shutdown");
}

bool OpenGLSceneRenderer::IsInitialized() const { return m_Impl->initialized; }

void OpenGLSceneRenderer::Update(float deltaTime) {
    m_Impl->elapsedTime += deltaTime;
    // Simple shake progression
    if (m_Impl->shakeTimer > 0.0f) {
        m_Impl->shakeTimer -= deltaTime;
        if (m_Impl->shakeTimer <= 0.0f) {
            m_Impl->shakeOffset = Vector2::Zero();
            m_Impl->shakeTimer = 0.0f;
        } else {
            float progress = 1.0f - (m_Impl->shakeTimer / m_Impl->shakeDuration);
            float strength = m_Impl->shakeAmplitude * (1.0f - progress);
            float t = m_Impl->shakeTimer * m_Impl->shakeFrequency;
            float offsetX = std::sin(t * 11.0f) * strength;
            float offsetY = std::cos(t * 13.0f) * strength;
            m_Impl->shakeOffset = Vector2(offsetX, offsetY);
        }
    }
}

void OpenGLSceneRenderer::SetCamera(const Camera2D& camera) {
    m_Impl->camera = camera;
    
    MarkProjectionDirty();
}
const Camera2D& OpenGLSceneRenderer::GetCamera() const { return m_Impl->camera; }
void OpenGLSceneRenderer::ResetCamera() {
    m_Impl->camera = Camera2D();
    MarkProjectionDirty();
}

void OpenGLSceneRenderer::PushScreenShake(float amplitude, float frequency, float duration) {
    m_Impl->shakeAmplitude = amplitude;
    m_Impl->shakeFrequency = frequency;
    m_Impl->shakeDuration = duration;
    m_Impl->shakeTimer = duration;
    MarkProjectionDirty();
}

#ifdef SAGE_ENGINE_TESTING
Vector2 OpenGLSceneRenderer::GetCameraShakeOffsetForTesting() const { return m_Impl->shakeOffset; }
float OpenGLSceneRenderer::GetShakeStrengthForTesting() const { return m_Impl->shakeAmplitude; }
float OpenGLSceneRenderer::GetShakeDurationForTesting() const { return m_Impl->shakeDuration; }
float OpenGLSceneRenderer::GetShakeTimerForTesting() const { return m_Impl->shakeTimer; }
#endif

void OpenGLSceneRenderer::BeginScene() {
    if (!m_Impl->defaultMaterialReady) {
    m_Impl->defaultMaterialReady = m_Impl->EnsureDefaultSpriteMaterial();
    } else {
        Ref<Material> defaultMaterial = MaterialLibrary::GetDefault();
        const bool materialValid = defaultMaterial && defaultMaterial->GetShader() && defaultMaterial->GetShader()->IsValid();
        if (!materialValid) {
            m_Impl->defaultMaterialReady = m_Impl->EnsureDefaultSpriteMaterial();
        }
    }

    // Set OpenGL viewport to match camera dimensions
    const int w = std::max(1, static_cast<int>(m_Impl->camera.GetViewportWidth()));
    const int h = std::max(1, static_cast<int>(m_Impl->camera.GetViewportHeight()));
    glViewport(0, 0, w, h);

    m_Impl->quadCountThisScene = 0;
    m_Impl->textQuadCountThisScene = 0;
    m_Impl->tileQuadCountThisScene = 0;
    m_Impl->drawCalls = 0;
    m_Impl->vertices = 0;
    m_Impl->triangles = 0;
    
    // Bind PostFX framebuffer if enabled
    if (m_Impl->postFXEnabled && m_Impl->postFXResourcesReady) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Impl->postFXFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    if (m_Impl->batchInitialized) {
        m_Impl->batchRenderer.BeginFrame();
    }

    // Apply initial shake offset for this frame prior to any pass needing view matrix
    ApplyShake();
}

bool OpenGLSceneRenderer::EndScene() {
    if (!m_Impl->batchInitialized) {
        return true;
    }

    ApplyShake();
    RecomputeProjection();

    Batching::FlushContext context{};
    context.viewportWidth = m_Impl->camera.GetViewportWidth();
    context.viewportHeight = m_Impl->camera.GetViewportHeight();
    context.totalTime = m_Impl->elapsedTime;
    context.camera = &m_Impl->camera;
    context.cameraShakeOffset = m_Impl->shakeOffset;
    context.projection = &m_Impl->projection;
    context.view = &m_Impl->view;
    context.viewProjection = &m_Impl->viewProjection;
    context.screenProjection = &m_Impl->screenProjection;
    context.drawCallCounter = &m_Impl->drawCalls;
    context.vertexCounter = &m_Impl->vertices;
    context.textureSlotBase = 0;

    const bool flushed = m_Impl->batchRenderer.Flush(context);
    if (flushed) {
        m_Impl->triangles = m_Impl->vertices / 3;
    }
    return flushed;
}

void OpenGLSceneRenderer::ApplyShake() {
    // Re-run lightweight shake progression without advancing timer (timer advanced in Update)
    // Ensures any mid-frame changes to amplitude/frequency take effect before projection
    if (m_Impl->shakeTimer <= 0.0f) {
        m_Impl->shakeOffset = Vector2::Zero();
        return;
    }
    float progress = 1.0f - (m_Impl->shakeTimer / m_Impl->shakeDuration);
    float strength = m_Impl->shakeAmplitude * (1.0f - progress);
    float t = m_Impl->shakeTimer * m_Impl->shakeFrequency;
    float offsetX = std::sin(t * 11.0f) * strength;
    float offsetY = std::cos(t * 13.0f) * strength;
    m_Impl->shakeOffset = Vector2(offsetX, offsetY);
}

void OpenGLSceneRenderer::MarkProjectionDirty() { m_Impl->projectionDirty = true; }

void OpenGLSceneRenderer::SetLayer(float layer) { m_Impl->currentLayer = layer; }
void OpenGLSceneRenderer::PushLayer(float layer) { m_Impl->layerStack.push_back(m_Impl->currentLayer); m_Impl->currentLayer = layer; }
void OpenGLSceneRenderer::PopLayer() { if (!m_Impl->layerStack.empty()) { m_Impl->currentLayer = m_Impl->layerStack.back(); m_Impl->layerStack.pop_back(); } }

MaterialId OpenGLSceneRenderer::SetMaterial(MaterialId materialId) {
    MaterialId previous = m_Impl->currentMaterialId;
    m_Impl->currentMaterialId = materialId;
    // Resolve material ref (fallback to default)
    Ref<Material> resolved;
    auto ensureDefaultMaterial = [&]() -> Ref<Material> {
        if (!m_Impl->defaultMaterialReady) {
            m_Impl->defaultMaterialReady = m_Impl->EnsureDefaultSpriteMaterial();
        }
        if (!m_Impl->defaultMaterialReady) {
            return nullptr;
        }
        return MaterialLibrary::GetDefault();
    };
    if (materialId == 0) {
        resolved = ensureDefaultMaterial();
        m_Impl->currentMaterialId = MaterialLibrary::GetDefaultId();
    } else {
        resolved = MaterialLibrary::Get(materialId);
        if (!resolved || !resolved->GetShader() || !resolved->GetShader()->IsValid()) {
            if (!resolved) {
                SAGE_WARNING("Material id {0} not found; falling back to default material", materialId);
            } else {
                SAGE_WARNING("Material '{0}' has no valid shader; falling back to default material", resolved->GetName());
            }
            resolved = ensureDefaultMaterial();
            m_Impl->currentMaterialId = MaterialLibrary::GetDefaultId();
        }
    }

    if (resolved && (!resolved->GetShader() || !resolved->GetShader()->IsValid())) {
        SAGE_WARNING("Default material '{0}' is not usable (missing or invalid shader)", resolved->GetName());
        resolved.reset();
        m_Impl->currentMaterialId = 0;
    }

    m_Impl->currentMaterialRef = resolved;
    if (!m_Impl->currentMaterialRef) {
        m_Impl->currentMaterialId = 0;
    }
    return previous;
}

void OpenGLSceneRenderer::PushBlendMode(BlendMode mode) { m_Impl->blendStack.push_back(m_Impl->currentBlendMode); m_Impl->currentBlendMode = mode; }
void OpenGLSceneRenderer::PopBlendMode() { if (!m_Impl->blendStack.empty()) { m_Impl->currentBlendMode = m_Impl->blendStack.back(); m_Impl->blendStack.pop_back(); } }
void OpenGLSceneRenderer::SetBlendMode(BlendMode mode) { m_Impl->currentBlendMode = mode; }
BlendMode OpenGLSceneRenderer::GetBlendMode() const { return m_Impl->currentBlendMode; }

void OpenGLSceneRenderer::PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                         float biasConstant, float biasSlope) {
    m_Impl->depthStack.push_back(m_Impl->currentDepthState);
    m_Impl->currentDepthState.testEnabled = enableTest;
    m_Impl->currentDepthState.writeEnabled = enableWrite;
    m_Impl->currentDepthState.function = function;
    m_Impl->currentDepthState.biasConstant = biasConstant;
    m_Impl->currentDepthState.biasSlope = biasSlope;
}
void OpenGLSceneRenderer::PopDepthState() { if (!m_Impl->depthStack.empty()) { m_Impl->currentDepthState = m_Impl->depthStack.back(); m_Impl->depthStack.pop_back(); } }
void OpenGLSceneRenderer::SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                        float biasConstant, float biasSlope) {
    m_Impl->currentDepthState.testEnabled = enableTest;
    m_Impl->currentDepthState.writeEnabled = enableWrite;
    m_Impl->currentDepthState.function = function;
    m_Impl->currentDepthState.biasConstant = biasConstant;
    m_Impl->currentDepthState.biasSlope = biasSlope;
}
DepthSettings OpenGLSceneRenderer::GetDepthState() const { return m_Impl->currentDepthState; }

void OpenGLSceneRenderer::PushEffect(const QuadEffect& effect) { m_Impl->effectStack.push_back(m_Impl->currentEffect); m_Impl->currentEffect = effect; }
void OpenGLSceneRenderer::PopEffect() { if (!m_Impl->effectStack.empty()) { m_Impl->currentEffect = m_Impl->effectStack.back(); m_Impl->effectStack.pop_back(); } else { m_Impl->currentEffect = QuadEffect{}; } }

void OpenGLSceneRenderer::ConfigurePostFX(const PostFXSettings& settings) { m_Impl->postFXSettings = settings; }
const PostFXSettings& OpenGLSceneRenderer::GetPostFXSettings() const { return m_Impl->postFXSettings; }

void OpenGLSceneRenderer::EnablePostFX(bool enabled) {
    if (m_Impl->postFXEnabled == enabled) return; // no change
    
    if (enabled) {
        // Create resources on enable
        if (!m_Impl->postFXResourcesReady) {
            CreatePostFXResources();
        }
    } else {
        // Destroy resources on disable
        if (m_Impl->postFXResourcesReady) {
            DestroyPostFXResources();
        }
    }
    
    m_Impl->postFXEnabled = enabled;
}

bool OpenGLSceneRenderer::DrawQuad(const QuadDesc& desc) {
    if (desc.size.x == 0.0f || desc.size.y == 0.0f) return true; // ignore degenerate
    m_Impl->quadCountThisScene++;
    // Explicit classification via QuadDesc::source
    if (desc.source == QuadDesc::QuadSource::Tile) {
        m_Impl->tileQuadCountThisScene++;
    }
    if (!m_Impl->batchInitialized) return true; // fallback: metrics only

    if (!m_Impl->currentMaterialRef) {
        m_Impl->currentMaterialRef = MaterialLibrary::GetDefault();
        m_Impl->currentMaterialId = MaterialLibrary::GetDefaultId();
    }

    Batching::QuadCommand q{};
    q.position = Vector2(desc.position.x, desc.position.y);
    q.size = Vector2(desc.size.x, desc.size.y);
    q.uvMin = Vector2(desc.uvMin.x, desc.uvMin.y);
    q.uvMax = Vector2(desc.uvMax.x, desc.uvMax.y);
    q.color = desc.color;
    q.texture = desc.texture; // may be null
    // Material binding: for now store nothing, but could look up by id later
    q.material = m_Impl->currentMaterialRef;
    q.materialId = m_Impl->currentMaterialId; // requires QuadCommand to have materialId; if absent will be ignored
    q.effect = m_Impl->currentEffect;
    q.layer = m_Impl->currentLayer;
    q.rotation = desc.rotation;
    q.screenSpace = desc.screenSpace;
    q.blendMode = m_Impl->currentBlendMode;
    q.depthState = m_Impl->currentDepthState;

    // Capacity-aware pre-flush: if adding one exceeds capacity and we have pending commands, flush first.
    if (m_Impl->batchRenderer.HasPendingCommands() &&
        m_Impl->batchRenderer.GetPendingCommandCount() + 1 > m_Impl->batchRenderer.GetMaxQuads()) {
        if (!EndScene()) {
            SAGE_ERROR("OpenGLSceneRenderer: pre-flush failed before queuing quad");
            return false;
        }
        // Start a new frame post-flush to clear buffers
        m_Impl->batchRenderer.BeginFrame();
    }
    const bool queued = m_Impl->batchRenderer.QueueQuad(q, nullptr);
    if (!queued) {
        SAGE_ERROR("OpenGLSceneRenderer: failed to queue quad (capacity?)");
        return false;
    }
    return true;
}

bool OpenGLSceneRenderer::DrawText(const TextDesc& desc) {
    if (desc.text.empty()) return true;
    if (!m_Impl->batchInitialized) return true; // metrics updated after queue now

    if (!m_Impl->currentMaterialRef) {
        m_Impl->currentMaterialRef = MaterialLibrary::GetDefault();
        m_Impl->currentMaterialId = MaterialLibrary::GetDefaultId();
    }

    Batching::TextCommand t{};
    t.text = desc.text;
    t.position = desc.position;
    t.font = desc.font;
    t.scale = desc.scale;
    t.color = desc.color;
    t.screenSpace = desc.screenSpace;
    t.material = m_Impl->currentMaterialRef;
    t.materialId = m_Impl->currentMaterialId; // requires TextCommand to have materialId; if absent will be ignored
    t.effect = m_Impl->currentEffect;
    t.layer = m_Impl->currentLayer;
    t.blendMode = m_Impl->currentBlendMode;
    t.depthState = m_Impl->currentDepthState;

    // Same capacity-aware logic (heuristic: treat text command as number of glyph quads estimated by text length)
    std::size_t estimatedGlyphs = desc.text.size();
    if (estimatedGlyphs == 0) estimatedGlyphs = 1;
    if (m_Impl->batchRenderer.HasPendingCommands() &&
        m_Impl->batchRenderer.GetPendingCommandCount() + estimatedGlyphs > m_Impl->batchRenderer.GetMaxQuads()) {
        if (!EndScene()) {
            SAGE_ERROR("OpenGLSceneRenderer: pre-flush failed before queuing text");
            return false;
        }
        m_Impl->batchRenderer.BeginFrame();
    }
    std::size_t glyphsQueued = m_Impl->batchRenderer.QueueText(t, nullptr);
    m_Impl->textQuadCountThisScene += glyphsQueued;
    // Treat zero glyphs when text non-empty as a warning (could be all whitespace/newlines or font issues)
    if (glyphsQueued == 0 && !desc.text.empty()) {
        SAGE_WARNING("OpenGLSceneRenderer: text queued produced zero glyph quads (possibly all newlines or missing glyphs)");
    }
    return true;
}

Float2 OpenGLSceneRenderer::MeasureText(const std::string& text, const Ref<Font>& font, float scale) {
    if (!font || !font->IsLoaded() || text.empty()) {
        return Float2{0.0f, 0.0f};
    }
    std::string_view view(text);
    size_t utf8Offset = 0;
    uint32_t codepoint = 0;
    uint32_t prev = 0;
    bool hasPrev = false;
    float cursorX = 0.0f;
    float maxWidthCurrentLine = 0.0f;
    float maxWidth = 0.0f;
    float totalHeight = font->GetLineHeight() * scale;
    float lineHeight = font->GetLineHeight() * scale;
    while (SAGE::Core::UTF8Utils::NextCodePoint(view, utf8Offset, codepoint)) {
        if (codepoint == static_cast<uint32_t>('\n')) {
            maxWidth = std::max(maxWidth, maxWidthCurrentLine);
            cursorX = 0.0f;
            maxWidthCurrentLine = 0.0f;
            totalHeight += lineHeight;
            hasPrev = false;
            prev = 0;
            continue;
        }
        if (hasPrev) {
            cursorX += font->GetKerning(prev, codepoint) * scale;
        }
        const Glyph& g = font->GetGlyph(codepoint);
        float advance = g.advance * scale;
        float glyphWidth = g.extent.x * scale;
        maxWidthCurrentLine = std::max(maxWidthCurrentLine, cursorX + glyphWidth);
        cursorX += advance;
        prev = codepoint;
        hasPrev = true;
    }
    maxWidth = std::max(maxWidth, maxWidthCurrentLine);
    return Float2{ maxWidth, totalHeight };
}

// ========== PostFX Implementation ==========

void OpenGLSceneRenderer::CreatePostFXResources() {
    // Check if viewport size changed - destroy and recreate
    const int width = static_cast<int>(m_Impl->camera.GetViewportWidth());
    const int height = static_cast<int>(m_Impl->camera.GetViewportHeight());
    
    if (width <= 0 || height <= 0) {
        SAGE_WARNING("Cannot create PostFX resources with invalid viewport: {}x{}", width, height);
        return;
    }
    
    // FIXED: Recreate resources if viewport size changed
    if (m_Impl->postFXResourcesReady && 
        (m_Impl->postFXViewportWidth != width || m_Impl->postFXViewportHeight != height)) {
        SAGE_INFO("PostFX viewport changed from {}x{} to {}x{} - recreating resources", 
                  m_Impl->postFXViewportWidth, m_Impl->postFXViewportHeight, width, height);
        DestroyPostFXResources();
    }
    
    if (m_Impl->postFXResourcesReady) return;  // Already created at correct size
    
    m_Impl->postFXViewportWidth = width;
    m_Impl->postFXViewportHeight = height;
    
    {
        GLErrorScope errorScope("CreatePostFXResources::Framebuffer");
        
    // Create framebuffer
    glGenFramebuffers(1, &m_Impl->postFXFramebuffer);
    LabelGlObject(GL_FRAMEBUFFER, m_Impl->postFXFramebuffer, "PostFX.Framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, m_Impl->postFXFramebuffer);
        
        // Create color texture
    glGenTextures(1, &m_Impl->postFXColorTexture);
    LabelGlObject(GL_TEXTURE, m_Impl->postFXColorTexture, "PostFX.ColorTexture");
        glBindTexture(GL_TEXTURE_2D, m_Impl->postFXColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Impl->postFXColorTexture, 0);
        
        // Create depth renderbuffer (optional, for depth testing in effects)
    glGenRenderbuffers(1, &m_Impl->postFXDepthRenderbuffer);
    LabelGlObject(GL_RENDERBUFFER, m_Impl->postFXDepthRenderbuffer, "PostFX.DepthStencil");
        glBindRenderbuffer(GL_RENDERBUFFER, m_Impl->postFXDepthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Impl->postFXDepthRenderbuffer);
        
        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            SAGE_ERROR("PostFX framebuffer incomplete!");
            DestroyPostFXResources();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    // Create shader program
    m_Impl->postFXShader = m_Impl->AcquirePostFXShader();
    if (!m_Impl->postFXShader || !m_Impl->postFXShader->IsValid()) {
        SAGE_ERROR("Failed to create PostFX shader program");
        DestroyPostFXResources();
        return;
    }
    
    // Create fullscreen quad VAO/VBO
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_Impl->postFXVAO);
    LabelGlObject(GL_VERTEX_ARRAY, m_Impl->postFXVAO, "PostFX.FullscreenVAO");
    glGenBuffers(1, &m_Impl->postFXVBO);
    LabelGlObject(GL_BUFFER, m_Impl->postFXVBO, "PostFX.FullscreenVBO");
    glBindVertexArray(m_Impl->postFXVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_Impl->postFXVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    
    m_Impl->postFXResourcesReady = true;
    SAGE_INFO("PostFX resources created ({}x{})", width, height);
}

void OpenGLSceneRenderer::DestroyPostFXResources() {
    if (!m_Impl->postFXResourcesReady) {
        // Still attempt to delete any stray objects to be safe
        if (m_Impl->postFXVAO) { glDeleteVertexArrays(1, &m_Impl->postFXVAO); m_Impl->postFXVAO = 0; }
        if (m_Impl->postFXVBO) { glDeleteBuffers(1, &m_Impl->postFXVBO); m_Impl->postFXVBO = 0; }
        if (m_Impl->postFXColorTexture) { glDeleteTextures(1, &m_Impl->postFXColorTexture); m_Impl->postFXColorTexture = 0; }
        if (m_Impl->postFXDepthRenderbuffer) { glDeleteRenderbuffers(1, &m_Impl->postFXDepthRenderbuffer); m_Impl->postFXDepthRenderbuffer = 0; }
        if (m_Impl->postFXFramebuffer) { glDeleteFramebuffers(1, &m_Impl->postFXFramebuffer); m_Impl->postFXFramebuffer = 0; }
    m_Impl->postFXBlurTextures.clear();
    m_Impl->postFXShader.reset();
        return; // nothing else to do
    }
    if (m_Impl->postFXVAO) {
        glDeleteVertexArrays(1, &m_Impl->postFXVAO);
        m_Impl->postFXVAO = 0;
    }
    if (m_Impl->postFXVBO) {
        glDeleteBuffers(1, &m_Impl->postFXVBO);
        m_Impl->postFXVBO = 0;
    }
    m_Impl->postFXShader.reset();
    if (m_Impl->postFXColorTexture) {
        glDeleteTextures(1, &m_Impl->postFXColorTexture);
        m_Impl->postFXColorTexture = 0;
    }
    if (m_Impl->postFXDepthRenderbuffer) {
        glDeleteRenderbuffers(1, &m_Impl->postFXDepthRenderbuffer);
        m_Impl->postFXDepthRenderbuffer = 0;
    }
    if (m_Impl->postFXFramebuffer) {
        glDeleteFramebuffers(1, &m_Impl->postFXFramebuffer);
        m_Impl->postFXFramebuffer = 0;
    }
    
    m_Impl->postFXBlurTextures.clear();
    m_Impl->postFXResourcesReady = false;
    SAGE_INFO("PostFX resources destroyed");
}

void OpenGLSceneRenderer::ApplyPostFX() {
    if (!m_Impl->postFXResourcesReady) {
        return;
    }

    if (!m_Impl->postFXShader || !m_Impl->postFXShader->IsValid()) {
    m_Impl->postFXShader = m_Impl->AcquirePostFXShader();
        if (!m_Impl->postFXShader || !m_Impl->postFXShader->IsValid()) {
            SAGE_WARNING("PostFX shader unavailable; skipping effect");
            return;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    m_Impl->postFXShader->Bind();
    m_Impl->postFXShader->SetInt("u_SceneTexture", kPostFXSceneTextureSlot);

    glActiveTexture(GL_TEXTURE0 + kPostFXSceneTextureSlot);
    glBindTexture(GL_TEXTURE_2D, m_Impl->postFXColorTexture);

    const auto& settings = m_Impl->postFXSettings;
    m_Impl->postFXShader->SetFloat4("u_Tint", settings.tint.r, settings.tint.g, settings.tint.b, settings.tint.a);
    m_Impl->postFXShader->SetFloat("u_Intensity", settings.intensity);
    m_Impl->postFXShader->SetFloat("u_Gamma", settings.gamma);
    m_Impl->postFXShader->SetFloat("u_Exposure", settings.exposure);

    const float pulseTime = settings.pulseSpeed > 0.0f
        ? m_Impl->elapsedTime * settings.pulseSpeed
        : 0.0f;
    m_Impl->postFXShader->SetFloat("u_PulseTime", pulseTime);

    glBindVertexArray(m_Impl->postFXVAO);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    m_Impl->postFXShader->Unbind();
}

OpenGLSceneRenderer::Stats OpenGLSceneRenderer::GetStats() const {
    Stats s;
    s.drawCalls = m_Impl->drawCalls;
    s.vertices = m_Impl->vertices;
    s.triangles = m_Impl->triangles;
    s.requestedQuads = m_Impl->quadCountThisScene;
    s.requestedTextGlyphs = m_Impl->textQuadCountThisScene; // will be updated to glyph count later
    s.requestedTiles = m_Impl->tileQuadCountThisScene;
    return s;
}

void OpenGLSceneRenderer::RecomputeProjection() {
    float w = std::max(1.0f, m_Impl->camera.GetViewportWidth());
    float h = std::max(1.0f, m_Impl->camera.GetViewportHeight());

    m_Impl->camera.SetViewportSize(w, h);
    
    // Базовые матрицы (camera уже содержит все параметры)
    Matrix4 projection = m_Impl->camera.GetProjectionMatrix();
    Matrix4 view = m_Impl->camera.GetViewMatrix();

    // Добавляем screen shake (переводим после базового view)
    if (std::abs(m_Impl->shakeOffset.x) > 0.0001f || std::abs(m_Impl->shakeOffset.y) > 0.0001f) {
        view = Matrix4::Translate(-m_Impl->shakeOffset.x, -m_Impl->shakeOffset.y, 0.0f) * view;
    }

    m_Impl->view = view;
    m_Impl->projection = projection;
    m_Impl->viewProjection = projection * view;

    // Отдельная проекция для UI (экранное пространство, левый верх = (0,0))
    m_Impl->screenProjection = Matrix4::Orthographic(0.0f, w, h, 0.0f, -1.0f, 1.0f);

    m_Impl->projectionDirty = false;
}

} // namespace SAGE::Graphics
