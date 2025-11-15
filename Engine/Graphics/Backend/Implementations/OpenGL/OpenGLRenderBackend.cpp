#include "Graphics/Backend/Implementations/OpenGL/OpenGLRenderBackend.h"

#include "Graphics/Rendering/Batching/BatchRenderer.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"
#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/Rendering/StateManagement/RenderStateManager.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Core/Window.h"
#include "Math/Matrix4.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>
#include <optional>
#include <random>
#include <vector>

#include <glad/glad.h>

#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
#ifdef DrawTextW
#undef DrawTextW
#endif

namespace SAGE {
namespace {

// ========== Scene State ==========
struct ScreenShakeInstance {
    float amplitude = 0.0f;
    float frequency = 0.0f;
    float duration = 0.0f;
    float elapsed = 0.0f;
    float phaseX = 0.0f;
    float phaseY = 0.0f;
};

struct RendererState {
    Camera2D camera{};
    Vector2 cameraShakeOffset = Vector2::Zero();
    float totalTime = 0.0f;
    std::mt19937 rng{};
    std::vector<ScreenShakeInstance> shakeInstances;

    Matrix4 projection = Matrix4::Identity();
    Matrix4 view = Matrix4::Identity();
    Matrix4 viewProjection = Matrix4::Identity();
    Matrix4 screenProjection = Matrix4::Identity();
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;

    float currentLayer = 0.0f;
    QuadEffect currentEffect{};
    std::vector<float> layerStack;
    std::vector<QuadEffect> effectStack;

    Ref<Material> currentMaterial;
    Ref<Material> defaultMaterial;
    MaterialId currentMaterialId = 0;
    MaterialId defaultMaterialId = 0;

    PostFXSettings postFXSettings{};

    GraphicsResourceManager::TrackedFramebufferHandle postFXFramebuffer;
    GraphicsResourceManager::TrackedTextureHandle postFXColorAttachment;
    GraphicsResourceManager::TrackedRenderbufferHandle postFXDepthAttachment;
    GraphicsResourceManager::TrackedFramebufferHandle postFXPingPongFBO[2];
    GraphicsResourceManager::TrackedTextureHandle postFXPingPongColor[2];
    GraphicsResourceManager::TrackedVertexArrayHandle postFXQuadVAO;
    GraphicsResourceManager::TrackedBufferHandle postFXQuadVBO;
    Ref<Shader> postFXCompositeShader;
    Ref<Shader> postFXBlurShader;
    int postFXWidth = 0;
    int postFXHeight = 0;
    bool postFXFramebufferValid = false;
    bool renderingToPostFX = false;

    bool lastFlushSuccessful = true;
    float lastFlushDurationMs = 0.0f;
    std::chrono::high_resolution_clock::time_point frameStartTime{};
};

RendererState s_Data{};
Graphics::Batching::BatchRenderer s_BatchRenderer;
bool s_RendererInitialized = false;
std::size_t s_DrawCallsThisFrame = 0;
std::size_t s_VerticesThisFrame = 0;
Graphics::BatchConfig s_BatchConfig{};
std::optional<std::uint32_t> s_ScreenShakeSeedOverride{};
bool s_DebugLayerEnabled = true;

constexpr float kMinimumCameraZoom = 0.01f;
constexpr std::size_t kMaxActiveShakes = 8;
constexpr float kShakeDecayStrength = 3.0f;
constexpr float kTwoPi = 6.28318530717958647692f;

const ScreenShakeInstance* GetPrimaryShakeInstance() {
    if (s_Data.shakeInstances.empty()) {
        return nullptr;
    }

    return &*std::max_element(
        s_Data.shakeInstances.begin(),
        s_Data.shakeInstances.end(),
        [](const ScreenShakeInstance& a, const ScreenShakeInstance& b) {
            return a.amplitude < b.amplitude;
        });
}

bool HasActiveGLContext() {
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    return version != nullptr;
}

Graphics::BatchConfig SanitizeBatchConfig(const Graphics::BatchConfig& config) {
    Graphics::BatchConfig sanitized = config;
    if (sanitized.maxQuadCount == 0) {
        sanitized.maxQuadCount = Graphics::Batching::BatchRenderer::kDefaultMaxQuads;
    }
    return sanitized;
}

void ApplyScreenShakeSeed() {
    if (s_ScreenShakeSeedOverride.has_value()) {
        s_Data.rng.seed(*s_ScreenShakeSeedOverride);
    } else {
        std::random_device rd;
        s_Data.rng.seed(rd());
    }
}

class OpenGLSceneRenderer final : public Graphics::ISceneRenderer {
public:
    explicit OpenGLSceneRenderer(OpenGLRenderBackend& backend)
        : m_Backend(backend) {}

    void Initialize(IRenderDevice& /*device*/, IRenderContext& /*context*/) override {}
    void Shutdown() override {}

    void SetCamera(const CameraData& /*camera*/) override {}
    void RenderScene(const SceneRenderParams& /*params*/) override {}
    void SubmitMesh(const MeshSubmitInfo& /*meshInfo*/) override {}

    void SubmitCommands(const Rendering::Commands::RenderCommandQueue& queue) override {
        for (const auto& command : queue.Commands()) {
            switch (command.type) {
            case Rendering::Commands::CommandType::Quad:
                m_Backend.SubmitQuadInternal(command.quad);
                break;
            case Rendering::Commands::CommandType::Text:
                m_Backend.SubmitTextInternal(command.text);
                break;
            }
        }
    }

    void AddScreenShake(const Rendering::Commands::ScreenShakeCommand& command) override {
        m_Backend.PushScreenShakeInternal(command);
    }

    Float2 MeasureText(const std::string& text, const Ref<Font>& font, float scale) override {
        return m_Backend.MeasureTextInternal(text, font, scale);
    }

    void Flush() override {}

private:
    OpenGLRenderBackend& m_Backend;
};

void EnsurePostFXShaders() {
    const char* vertexSrc = R"(#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";

    if (!s_Data.postFXCompositeShader) {
        const char* compositeFragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_SceneTexture;
uniform vec4 u_Tint;
uniform float u_Intensity;
uniform float u_BloomStrength;
uniform float u_Time;
uniform float u_PulseSpeed;
uniform float u_Gamma;
uniform float u_Exposure;
uniform sampler2D u_BloomTexture;
uniform int u_BloomEnabled;

void main() {
    vec4 sceneSample = texture(u_SceneTexture, v_TexCoord);
    vec3 baseColor = sceneSample.rgb;
    float alpha = sceneSample.a;

    float clampedIntensity = clamp(u_Intensity, 0.0, 1.0);
    vec3 tinted = mix(baseColor, baseColor * u_Tint.rgb, clampedIntensity);

    float pulse = 1.0;
    if (u_PulseSpeed > 0.0) {
        pulse = sin(u_Time * u_PulseSpeed) * 0.5 + 0.5;
        tinted *= mix(1.0, pulse, clampedIntensity);
    }

    vec3 color = tinted;
    if (u_BloomEnabled == 1) {
        vec3 bloomSample = texture(u_BloomTexture, v_TexCoord).rgb;
        color += bloomSample * u_BloomStrength;
    }

    float exposure = max(u_Exposure, 0.0);
    if (exposure > 0.0) {
        color = vec3(1.0) - exp(-color * exposure);
    }

    float gamma = max(u_Gamma, 0.001);
    color = pow(color, vec3(1.0 / gamma));

    o_Color = vec4(color, alpha);
}
)";

        auto shader = ShaderManager::Load("Renderer_PostFXComposite", vertexSrc, compositeFragmentSrc);
        if (!shader) {
            SAGE_ERROR("Failed to create post-processing composite shader");
        } else {
            s_Data.postFXCompositeShader = shader;
            s_Data.postFXCompositeShader->Bind();
            s_Data.postFXCompositeShader->SetInt("u_SceneTexture", 0);
            s_Data.postFXCompositeShader->SetInt("u_BloomTexture", 1);
            s_Data.postFXCompositeShader->Unbind();
        }
    }

    if (!s_Data.postFXBlurShader) {
        const char* blurFragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_InputTexture;
uniform vec2 u_BlurDirection;
uniform float u_BloomThreshold;
uniform int u_ApplyThreshold;

vec3 SampleColor(vec2 uv) {
    vec3 color = texture(u_InputTexture, uv).rgb;
    if (u_ApplyThreshold == 1) {
        color = max(color - vec3(u_BloomThreshold), vec3(0.0));
    }
    return color;
}

void main() {
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec3 result = SampleColor(v_TexCoord) * weights[0];
    for (int i = 1; i < 5; ++i) {
        vec2 offset = u_BlurDirection * float(i);
        result += SampleColor(v_TexCoord + offset) * weights[i];
        result += SampleColor(v_TexCoord - offset) * weights[i];
    }
    o_Color = vec4(result, 1.0);
}
)";

        auto blurShader = ShaderManager::Load("Renderer_PostFXBlur", vertexSrc, blurFragmentSrc);
        if (!blurShader) {
            SAGE_ERROR("Failed to create post-processing blur shader");
        } else {
            s_Data.postFXBlurShader = blurShader;
            s_Data.postFXBlurShader->Bind();
            s_Data.postFXBlurShader->SetInt("u_InputTexture", 0);
            s_Data.postFXBlurShader->Unbind();
        }
    }
}

void EnsurePostFXQuad() {
    if (s_Data.postFXQuadVAO && s_Data.postFXQuadVBO) {
        return;
    }

    if (!s_Data.postFXQuadVAO) {
        s_Data.postFXQuadVAO.Create("Renderer_PostFXQuadVAO");
    }
    if (!s_Data.postFXQuadVBO) {
        s_Data.postFXQuadVBO.Create("Renderer_PostFXQuadVBO");
    }

    glBindVertexArray(s_Data.postFXQuadVAO.Get());
    glBindBuffer(GL_ARRAY_BUFFER, s_Data.postFXQuadVBO.Get());

    constexpr float quadVertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool EnsurePostFXFramebuffer(int width, int height) {
    if (width <= 0 || height <= 0) {
        return false;
    }

    const bool sizeChanged = width != s_Data.postFXWidth || height != s_Data.postFXHeight;
    if (sizeChanged ||
        !s_Data.postFXFramebuffer ||
        !s_Data.postFXColorAttachment ||
        !s_Data.postFXDepthAttachment) {
        s_Data.postFXFramebuffer.Reset();
        s_Data.postFXColorAttachment.Reset();
        s_Data.postFXDepthAttachment.Reset();
        for (auto& fbo : s_Data.postFXPingPongFBO) {
            fbo.Reset();
        }
        for (auto& tex : s_Data.postFXPingPongColor) {
            tex.Reset();
        }

        s_Data.postFXFramebuffer.Create("Renderer_PostFXFBO");
        s_Data.postFXColorAttachment.Create("Renderer_PostFXColor");
        s_Data.postFXDepthAttachment.Create("Renderer_PostFXDepth");

        glBindTexture(GL_TEXTURE_2D, s_Data.postFXColorAttachment.Get());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindRenderbuffer(GL_RENDERBUFFER, s_Data.postFXDepthAttachment.Get());
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.postFXFramebuffer.Get());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_Data.postFXColorAttachment.Get(), 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_Data.postFXDepthAttachment.Get());

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            SAGE_ERROR("PostFX framebuffer incomplete: 0x{0:X}", status);
            s_Data.postFXFramebufferValid = false;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create ping-pong framebuffers for blur
        for (int i = 0; i < 2; ++i) {
            s_Data.postFXPingPongFBO[i].Create(i == 0 ? "Renderer_PostFXPingPongFBO0" : "Renderer_PostFXPingPongFBO1");
            s_Data.postFXPingPongColor[i].Create(i == 0 ? "Renderer_PostFXPingPongColor0" : "Renderer_PostFXPingPongColor1");

            glBindTexture(GL_TEXTURE_2D, s_Data.postFXPingPongColor[i].Get());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, s_Data.postFXPingPongFBO[i].Get());
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_Data.postFXPingPongColor[i].Get(), 0);

            const GLenum blurStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (blurStatus != GL_FRAMEBUFFER_COMPLETE) {
                SAGE_ERROR("PostFX ping-pong framebuffer {} incomplete: 0x{:X}", i, blurStatus);
                s_Data.postFXFramebufferValid = false;
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return false;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        s_Data.postFXWidth = width;
        s_Data.postFXHeight = height;
        s_Data.postFXFramebufferValid = true;
    }

    return s_Data.postFXFramebufferValid;
}

bool EnsurePostFXResources(int width, int height) {
    EnsurePostFXShaders();
    EnsurePostFXQuad();
    if (!s_Data.postFXCompositeShader || !s_Data.postFXQuadVAO) {
        return false;
    }
    return EnsurePostFXFramebuffer(width, height);
}

void RenderPostFXPass(int screenWidth, int screenHeight) {
    if (!s_Data.postFXCompositeShader || !s_Data.postFXQuadVAO || !s_Data.postFXColorAttachment) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    const auto& fx = s_Data.postFXSettings;
    const bool bloomEnabled = fx.enabled &&
                              fx.blurIterations > 0 &&
                              fx.bloomStrength > 0.0f &&
                              s_Data.postFXPingPongFBO[0] &&
                              s_Data.postFXPingPongColor[0] &&
                              s_Data.postFXBlurShader;

    unsigned int sceneTexture = s_Data.postFXColorAttachment.Get();
    unsigned int bloomTexture = sceneTexture;

    if (bloomEnabled) {
        s_Data.postFXBlurShader->Bind();
        s_Data.postFXBlurShader->SetFloat("u_BloomThreshold", std::max(0.0f, fx.bloomThreshold));

        bool horizontal = true;
        bool firstPass = true;
        const int iterations = std::clamp(fx.blurIterations, 0, 10);
        int totalPasses = iterations * 2;
        unsigned int inputTexture = sceneTexture;

        for (int pass = 0; pass < totalPasses; ++pass) {
            const int targetIndex = horizontal ? 0 : 1;
            glBindFramebuffer(GL_FRAMEBUFFER, s_Data.postFXPingPongFBO[targetIndex].Get());
            glViewport(0, 0, std::max(1, s_Data.postFXWidth), std::max(1, s_Data.postFXHeight));

            const float invWidth = 1.0f / static_cast<float>(std::max(1, s_Data.postFXWidth));
            const float invHeight = 1.0f / static_cast<float>(std::max(1, s_Data.postFXHeight));
            Float2 direction = horizontal ? Float2{ invWidth, 0.0f } : Float2{ 0.0f, invHeight };
            s_Data.postFXBlurShader->SetFloat2("u_BlurDirection", direction);
            s_Data.postFXBlurShader->SetInt("u_ApplyThreshold", firstPass ? 1 : 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputTexture);

            glBindVertexArray(s_Data.postFXQuadVAO.Get());
            glDrawArrays(GL_TRIANGLES, 0, 6);

            inputTexture = s_Data.postFXPingPongColor[targetIndex].Get();
            horizontal = !horizontal;
            firstPass = false;
        }

        s_Data.postFXBlurShader->Unbind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        bloomTexture = inputTexture;
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
    }

    s_Data.postFXCompositeShader->Bind();
    s_Data.postFXCompositeShader->SetFloat("u_Intensity", std::clamp(fx.intensity, 0.0f, 1.0f));
    s_Data.postFXCompositeShader->SetFloat("u_BloomStrength", std::max(0.0f, fx.bloomStrength));
    s_Data.postFXCompositeShader->SetFloat("u_Time", s_Data.totalTime);
    s_Data.postFXCompositeShader->SetFloat("u_PulseSpeed", std::max(0.0f, fx.pulseSpeed));
    s_Data.postFXCompositeShader->SetFloat("u_Gamma", std::max(0.001f, fx.gamma));
    s_Data.postFXCompositeShader->SetFloat("u_Exposure", std::max(0.0f, fx.exposure));
    s_Data.postFXCompositeShader->SetInt("u_BloomEnabled", bloomEnabled ? 1 : 0);
    const Color& tint = fx.tint;
    s_Data.postFXCompositeShader->SetFloat4("u_Tint", tint.r, tint.g, tint.b, tint.a);

    glBindVertexArray(s_Data.postFXQuadVAO.Get());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    s_Data.postFXCompositeShader->Unbind();

    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void APIENTRY OnGLDebugMessage(GLenum /*source*/, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        SAGE_ERROR("OpenGL error [id={}]: {}", id, message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        SAGE_WARNING("OpenGL warning [id={}]: {}", id, message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        SAGE_INFO("OpenGL notice [id={}]: {}", id, message);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
    default:
        SAGE_INFO("OpenGL message [id={}]: {}", id, message);
        break;
    }
    (void)type;
}

void ConfigureGLDebugOutput() {
#if defined(SAGE_GL_DEBUG)
    if (!s_RendererInitialized) {
        return;
    }

#if defined(GLAD_GL_VERSION_4_3)
    const bool hasCoreDebug = GLAD_GL_VERSION_4_3 != 0;
#else
    const bool hasCoreDebug = false;
#endif

#if defined(GLAD_GL_KHR_debug)
    const bool hasDebugExtension = GLAD_GL_KHR_debug != 0;
#else
    const bool hasDebugExtension = false;
#endif

    if (!(hasCoreDebug || hasDebugExtension)) {
        return;
    }

    if (s_DebugLayerEnabled) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OnGLDebugMessage, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    } else {
        glDebugMessageCallback(nullptr, nullptr);
        glDisable(GL_DEBUG_OUTPUT);
    }
#endif
}

Matrix4 BuildViewMatrix(const Camera2D& camera, const Vector2& shakeOffset) {
    const float clampedZoom = std::max(kMinimumCameraZoom, camera.zoom);
    const Vector2 target = camera.position + shakeOffset;
    const Vector2 pivot = camera.rotationOrigin;

    Matrix4 translation = Matrix4::Translate(-target.x, -target.y, 0.0f);
    Matrix4 toPivot = Matrix4::Translate(-pivot.x, -pivot.y, 0.0f);
    Matrix4 fromPivot = Matrix4::Translate(pivot.x, pivot.y, 0.0f);
    Matrix4 rotation = Matrix4::RotateZ(-camera.rotation);
    Matrix4 scale = Matrix4::Scale(clampedZoom, clampedZoom, 1.0f);

    return fromPivot * scale * rotation * toPivot * translation;
}

// ========== Helper Functions ==========
bool FlushCommands() {
    if (!s_BatchRenderer.HasPendingCommands()) {
        s_Data.lastFlushDurationMs = 0.0f;
        return true;
    }

    Graphics::Batching::FlushContext context;
    context.totalTime = s_Data.totalTime;
    context.camera = &s_Data.camera;
    context.cameraShakeOffset = s_Data.cameraShakeOffset;
    context.drawCallCounter = &s_DrawCallsThisFrame;
    context.vertexCounter = &s_VerticesThisFrame;

    float width = 0.0f;
    float height = 0.0f;

    if (Application::HasInstance()) {
        auto& window = Application::Get().GetWindow();
        width = static_cast<float>(std::max<std::size_t>(1, window.GetWidth()));
        height = static_cast<float>(std::max<std::size_t>(1, window.GetHeight()));
    } else {
        width = 1280.0f;
        height = 720.0f;
    }

    context.viewportWidth = std::max(width, 1.0f);
    context.viewportHeight = std::max(height, 1.0f);

    s_Data.viewportWidth = context.viewportWidth;
    s_Data.viewportHeight = context.viewportHeight;

    const Matrix4 projection = Matrix4::Orthographic(0.0f,
                                                     context.viewportWidth,
                                                     context.viewportHeight,
                                                     0.0f,
                                                     -1.0f,
                                                     1.0f);
    const Matrix4 view = BuildViewMatrix(s_Data.camera, s_Data.cameraShakeOffset);
    s_Data.projection = projection;
    s_Data.view = view;
    s_Data.viewProjection = projection * view;
    s_Data.screenProjection = projection;

    context.projection = &s_Data.projection;
    context.view = &s_Data.view;
    context.viewProjection = &s_Data.viewProjection;
    context.screenProjection = &s_Data.screenProjection;

    const bool flushed = s_BatchRenderer.Flush(context);
    s_Data.lastFlushDurationMs = s_BatchRenderer.GetLastFlushDurationMs();
    return flushed;
}

void FlushPendingCommands(const char* reason) {
    if (!FlushCommands()) {
        SAGE_ERROR("OpenGLRenderBackend: flush failed during state change ({})", reason);
    }
}

void EnsureDefaultMaterial() {
    if (s_Data.defaultMaterial && s_Data.defaultMaterial->GetShader()) {
        if (!s_Data.currentMaterial) {
            s_Data.currentMaterial = s_Data.defaultMaterial;
            s_Data.currentMaterialId = s_Data.defaultMaterial->GetId();
        }
        return;
    }

    if (!HasActiveGLContext()) {
        SAGE_WARNING("EnsureDefaultMaterial skipped: OpenGL context is not active yet");
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

uniform mat4 u_ViewProjection;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_Pulse = a_Pulse;

    mat4 viewMatrix = u_View;
    mat4 projectionMatrix = u_Projection;
    gl_Position = projectionMatrix * viewMatrix * vec4(a_Position, 1.0);
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
    const Matrix4 identity = Matrix4::Identity();
    shader->SetMat4("u_ViewProjection", identity.Data());
    shader->SetMat4IfExists("u_View", identity.Data());
    shader->SetMat4IfExists("u_Projection", identity.Data());

    auto material = Material::Create("Renderer2D_Default", shader);
    if (!material) {
        SAGE_ERROR("Failed to create default material");
        return;
    }

    material->SetTint(Color::White());
    MaterialLibrary::RegisterMaterial(material);
    s_Data.defaultMaterial = material;
    s_Data.currentMaterial = material;
    s_Data.defaultMaterialId = material->GetId();
    s_Data.currentMaterialId = s_Data.defaultMaterialId;

    SAGE_INFO("Default material created successfully");
}

} // namespace

void OpenGLRenderBackend::Init() {
    if (s_RendererInitialized) {
        SAGE_WARNING("OpenGLRenderBackend::Init called more than once");
        return;
    }

    // Initialize managers
    ShaderManager::Init();
    MaterialLibrary::Init();
    StateManagement::RenderStateManager::Init();

    // Setup basic GL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize batch renderer
    s_BatchConfig = SanitizeBatchConfig(s_BatchConfig);
    s_BatchRenderer.Initialize(s_BatchConfig.maxQuadCount, s_BatchConfig.enableDynamicResizing);

    // Initialize random
    ApplyScreenShakeSeed();

    s_Data.camera = Camera2D{};
    s_Data.cameraShakeOffset = Vector2::Zero();
    s_Data.totalTime = 0.0f;

    EnsureDefaultMaterial();

    s_RendererInitialized = true;
    ConfigureGLDebugOutput();
    SAGE_INFO("OpenGLRenderBackend initialized successfully");
}

void OpenGLRenderBackend::Shutdown() {
    if (!s_RendererInitialized) {
        return;
    }

    s_BatchRenderer.Shutdown();

    s_Data.layerStack.clear();
    s_Data.effectStack.clear();
    s_Data.currentMaterial.reset();
    s_Data.defaultMaterial.reset();
    s_Data.postFXCompositeShader.reset();
    s_Data.postFXBlurShader.reset();
    s_Data.postFXFramebuffer.Reset();
    s_Data.postFXColorAttachment.Reset();
    s_Data.postFXDepthAttachment.Reset();
    for (auto& fbo : s_Data.postFXPingPongFBO) {
        fbo.Reset();
    }
    for (auto& tex : s_Data.postFXPingPongColor) {
        tex.Reset();
    }
    s_Data.postFXQuadVBO.Reset();
    s_Data.postFXQuadVAO.Reset();
    s_Data.postFXWidth = 0;
    s_Data.postFXHeight = 0;
    s_Data.postFXFramebufferValid = false;
    s_Data.renderingToPostFX = false;

    MaterialLibrary::Shutdown();
    ShaderManager::Shutdown();
    StateManagement::RenderStateManager::Shutdown();

    s_RendererInitialized = false;
    SAGE_INFO("OpenGLRenderBackend shutdown");
}

[[nodiscard]] bool OpenGLRenderBackend::IsInitialized() const {
    return s_RendererInitialized;
}

void OpenGLRenderBackend::Configure(const Graphics::RenderSystemConfig& config) {
    Graphics::BatchConfig sanitized = SanitizeBatchConfig(config.batching);
    const bool batchConfigChanged = sanitized.maxQuadCount != s_BatchConfig.maxQuadCount ||
                                    sanitized.enableDynamicResizing != s_BatchConfig.enableDynamicResizing;
    const bool seedChanged = config.screenShakeSeed != s_ScreenShakeSeedOverride;
    const bool debugChanged = config.enableDebugLayer != s_DebugLayerEnabled;

    s_BatchConfig = sanitized;
    s_ScreenShakeSeedOverride = config.screenShakeSeed;
    s_DebugLayerEnabled = config.enableDebugLayer;

    if (!s_RendererInitialized) {
        return;
    }

    if (seedChanged) {
        ApplyScreenShakeSeed();
    }

    if (batchConfigChanged) {
        FlushPendingCommands("Configure");
        s_BatchRenderer.Shutdown();
        s_BatchRenderer.Initialize(s_BatchConfig.maxQuadCount, s_BatchConfig.enableDynamicResizing);
        SAGE_INFO("OpenGLRenderBackend reconfigured batching: maxQuads={}, dynamicResize={}",
                  s_BatchConfig.maxQuadCount,
                  s_BatchConfig.enableDynamicResizing);
    }

    if (debugChanged || batchConfigChanged) {
        ConfigureGLDebugOutput();
    }
}

void OpenGLRenderBackend::Update(float deltaTime) {
    s_Data.totalTime += deltaTime;

    if (s_Data.shakeInstances.empty()) {
        s_Data.cameraShakeOffset = Vector2::Zero();
        return;
    }

    Vector2 accumulatedOffset = Vector2::Zero();
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);

    auto it = s_Data.shakeInstances.begin();
    while (it != s_Data.shakeInstances.end()) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration || it->duration <= 0.0f) {
            it = s_Data.shakeInstances.erase(it);
            continue;
        }

        const float progress = it->elapsed / it->duration;
        const float decay = std::exp(-kShakeDecayStrength * progress);
        const float amplitude = it->amplitude * decay;

        float offsetX = 0.0f;
        float offsetY = 0.0f;

        const float frequency = std::max(0.0f, it->frequency);
        if (frequency > 0.0f) {
            const float omega = frequency * kTwoPi;
            const float phase = omega * it->elapsed;
            offsetX = std::sin(phase + it->phaseX);
            offsetY = std::cos(phase + it->phaseY);
        }
        else {
            offsetX = noise(s_Data.rng);
            offsetY = noise(s_Data.rng);
        }

        accumulatedOffset += Vector2(offsetX, offsetY) * amplitude;
        ++it;
    }

    if (s_Data.shakeInstances.empty()) {
        s_Data.cameraShakeOffset = Vector2::Zero();
    }
    else {
        s_Data.cameraShakeOffset = accumulatedOffset;
    }
}

void OpenGLRenderBackend::SetCamera(const Camera2D& camera) {
    s_Data.camera.position = camera.position;
    s_Data.camera.zoom = std::max(kMinimumCameraZoom, camera.zoom);
    s_Data.camera.rotation = camera.rotation;
    s_Data.camera.rotationOrigin = camera.rotationOrigin;
}

[[nodiscard]] const Camera2D& OpenGLRenderBackend::GetCamera() const {
    return s_Data.camera;
}

void OpenGLRenderBackend::ResetCamera() {
    s_Data.camera = Camera2D{};
    s_Data.cameraShakeOffset = Vector2::Zero();
    s_Data.shakeInstances.clear();
}

void OpenGLRenderBackend::PushScreenShake(float amplitude, float frequency, float duration) {
    amplitude = std::max(0.0f, amplitude);
    frequency = std::max(0.0f, frequency);
    duration = std::max(0.0f, duration);

    if (amplitude <= 0.0f || duration <= 0.0f) {
        return;
    }

    if (s_Data.shakeInstances.size() >= kMaxActiveShakes) {
        s_Data.shakeInstances.erase(s_Data.shakeInstances.begin());
    }

    ScreenShakeInstance instance{};
    instance.amplitude = amplitude;
    instance.frequency = frequency;
    instance.duration = duration;
    instance.elapsed = 0.0f;

    std::uniform_real_distribution<float> phaseDist(0.0f, kTwoPi);
    instance.phaseX = phaseDist(s_Data.rng);
    instance.phaseY = phaseDist(s_Data.rng);

    s_Data.shakeInstances.push_back(instance);
}

#ifdef SAGE_ENGINE_TESTING
Vector2 OpenGLRenderBackend::GetCameraShakeOffsetForTesting() const {
    return s_Data.cameraShakeOffset;
}

float OpenGLRenderBackend::GetShakeStrengthForTesting() const {
    float totalStrength = 0.0f;
    for (const auto& instance : s_Data.shakeInstances) {
        if (instance.amplitude <= 0.0f) {
            continue;
        }

        float decay = 1.0f;
        if (instance.duration > 0.0f) {
            const float progress = std::clamp(instance.elapsed / instance.duration, 0.0f, 1.0f);
            decay = std::max(0.0f, 1.0f - progress * kShakeDecayStrength);
        }

        totalStrength += instance.amplitude * decay;
    }
    return totalStrength;
}

float OpenGLRenderBackend::GetShakeDurationForTesting() const {
    if (const ScreenShakeInstance* primary = GetPrimaryShakeInstance()) {
        return primary->duration;
    }
    return 0.0f;
}

float OpenGLRenderBackend::GetShakeTimerForTesting() const {
    if (const ScreenShakeInstance* primary = GetPrimaryShakeInstance()) {
        if (primary->duration > 0.0f) {
            return std::clamp(primary->elapsed, 0.0f, primary->duration);
        }
        return std::max(0.0f, primary->elapsed);
    }
    return 0.0f;
}
#endif

void OpenGLRenderBackend::BeginScene() {
    s_DrawCallsThisFrame = 0;
    s_VerticesThisFrame = 0;
    s_Data.frameStartTime = std::chrono::high_resolution_clock::now();
    s_Data.lastFlushDurationMs = 0.0f;

    EnsureDefaultMaterial();
    s_Data.currentMaterial = s_Data.defaultMaterial;
    s_Data.currentMaterialId = s_Data.defaultMaterialId;

    s_Data.layerStack.clear();
    s_Data.effectStack.clear();
    s_Data.currentLayer = 0.0f;
    s_Data.currentEffect = QuadEffect{};

    int targetWidth = 1280;
    int targetHeight = 720;
    if (Application::HasInstance()) {
        auto& window = Application::Get().GetWindow();
        targetWidth = static_cast<int>(std::max(1u, window.GetWidth()));
        targetHeight = static_cast<int>(std::max(1u, window.GetHeight()));
    }

    bool usePostFX = s_Data.postFXSettings.enabled;
    if (usePostFX) {
        usePostFX = EnsurePostFXResources(targetWidth, targetHeight);
    }
    s_Data.renderingToPostFX = usePostFX;

    glBindFramebuffer(GL_FRAMEBUFFER, usePostFX ? s_Data.postFXFramebuffer.Get() : 0);

    s_BatchRenderer.BeginFrame();
    s_Data.lastFlushSuccessful = true;

    // Apply any dirty states before rendering
    StateManagement::RenderStateManager::ApplyDirtyStates();

    glViewport(0, 0, static_cast<GLsizei>(targetWidth), static_cast<GLsizei>(targetHeight));

    // Clear screen
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

[[nodiscard]] bool OpenGLRenderBackend::EndScene() {
    // Apply any remaining state changes before flush
    StateManagement::RenderStateManager::ApplyDirtyStates();
    
    const bool flushed = FlushCommands();
    s_Data.lastFlushSuccessful = flushed;

    if (flushed && s_Data.renderingToPostFX && s_Data.postFXFramebufferValid) {
        RenderPostFXPass(
            s_Data.postFXWidth > 0 ? s_Data.postFXWidth : static_cast<int>(s_Data.viewportWidth),
            s_Data.postFXHeight > 0 ? s_Data.postFXHeight : static_cast<int>(s_Data.viewportHeight));
        StateManagement::RenderStateManager::ApplyDirtyStates();
    } else if (s_Data.renderingToPostFX) {
        // PostFX pass skipped (flush failure or invalid FBO); restore default framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    s_Data.renderingToPostFX = false;

    const auto frameEnd = std::chrono::high_resolution_clock::now();
    if (s_Data.frameStartTime.time_since_epoch().count() != 0) {
        const auto frameDuration = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(frameEnd - s_Data.frameStartTime);
        Profiler::RecordMetric("Renderer/FrameTimeMs", frameDuration.count());
    }

    Profiler::SetDrawCalls(s_DrawCallsThisFrame);
    Profiler::SetVertexCount(s_VerticesThisFrame);
    Profiler::SetTriangleCount(s_VerticesThisFrame / 2);

    if (!flushed) {
        SAGE_ERROR("EndScene: flush failed, commands retained");
    }

    return flushed;
}

void OpenGLRenderBackend::Clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderBackend::Clear() {
    Clear(0.1f, 0.1f, 0.15f, 1.0f);
}

void OpenGLRenderBackend::SetLayer(float layer) {
    s_Data.currentLayer = layer;
}

void OpenGLRenderBackend::PushLayer(float layer) {
    s_Data.layerStack.push_back(s_Data.currentLayer);
    s_Data.currentLayer = layer;
}

void OpenGLRenderBackend::PopLayer() {
    if (!s_Data.layerStack.empty()) {
        s_Data.currentLayer = s_Data.layerStack.back();
        s_Data.layerStack.pop_back();
    } else {
        s_Data.currentLayer = 0.0f;
    }
}

MaterialId OpenGLRenderBackend::SetMaterial(MaterialId materialId) {
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

void OpenGLRenderBackend::PushEffect(const QuadEffect& effect) {
    s_Data.effectStack.push_back(s_Data.currentEffect);
    s_Data.currentEffect = effect;
}

void OpenGLRenderBackend::PopEffect() {
    if (!s_Data.effectStack.empty()) {
        s_Data.currentEffect = s_Data.effectStack.back();
        s_Data.effectStack.pop_back();
    } else {
        s_Data.currentEffect = QuadEffect{};
    }
}

void OpenGLRenderBackend::ConfigurePostFX(const PostFXSettings& settings) {
    s_Data.postFXSettings = settings;
    
    // Clamp values to valid ranges
    s_Data.postFXSettings.intensity = std::clamp(settings.intensity, 0.0f, 1.0f);
    s_Data.postFXSettings.bloomThreshold = std::max(0.0f, settings.bloomThreshold);
    s_Data.postFXSettings.bloomStrength = std::max(0.0f, settings.bloomStrength);
    s_Data.postFXSettings.blurIterations = std::clamp(settings.blurIterations, 0, 10);
    s_Data.postFXSettings.gamma = std::max(0.001f, settings.gamma);
    s_Data.postFXSettings.exposure = std::max(0.0f, settings.exposure);
    s_Data.postFXSettings.pulseSpeed = std::max(0.0f, settings.pulseSpeed);
}

[[nodiscard]] const PostFXSettings& OpenGLRenderBackend::GetPostFXSettings() const {
    return s_Data.postFXSettings;
}

void OpenGLRenderBackend::EnablePostFX(bool enabled) {
    s_Data.postFXSettings.enabled = enabled;
    if (!enabled) {
        s_Data.renderingToPostFX = false;
    }
}

[[nodiscard]] bool OpenGLRenderBackend::DrawQuad(const QuadDesc& desc) {
    if (desc.size.x == 0.0f || desc.size.y == 0.0f) {
        return true;
    }

    EnsureDefaultMaterial();

    Graphics::Batching::QuadCommand command;
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
    // Reflect the active state stack so batching keys and GL state remain consistent
    command.blendMode = StateManagement::RenderStateManager::GetBlendMode();
    command.depthState = StateManagement::RenderStateManager::GetDepthState();

    auto flushDelegate = [&]() -> bool {
        const bool flushed = FlushCommands();
        s_Data.lastFlushSuccessful = flushed;
        return flushed;
    };

    const bool queued = s_BatchRenderer.QueueQuad(command, flushDelegate);
    if (!queued) {
        SAGE_ERROR("Failed to queue quad");
    }
    return queued;
}

[[nodiscard]] bool OpenGLRenderBackend::DrawText(const TextDesc& desc) {
    if (desc.text.empty() || !desc.font || !desc.font->IsLoaded()) {
        return true;
    }

    EnsureDefaultMaterial();

    Graphics::Batching::TextCommand command;
    command.text = desc.text;
    command.position = desc.position;
    command.font = desc.font;
    command.scale = desc.scale;
    command.color = desc.color;
    command.screenSpace = desc.screenSpace;
    command.material = s_Data.currentMaterial ? s_Data.currentMaterial : s_Data.defaultMaterial;
    command.effect = s_Data.currentEffect;
    command.layer = s_Data.currentLayer;
    command.blendMode = StateManagement::RenderStateManager::GetBlendMode();
    command.depthState = StateManagement::RenderStateManager::GetDepthState();

    auto flushDelegate = [&]() -> bool {
        const bool flushed = FlushCommands();
        s_Data.lastFlushSuccessful = flushed;
        return flushed;
    };

    const bool queued = s_BatchRenderer.QueueText(command, flushDelegate);
    if (!queued) {
        SAGE_ERROR("Failed to queue text");
    }
    return queued;
}

[[nodiscard]] Float2 OpenGLRenderBackend::MeasureText(const std::string& text, const Ref<Font>& font, float scale) {
    if (text.empty() || !font || !font->IsLoaded()) {
        return Float2::Zero();
    }

    float lineWidth = 0.0f;
    float maxWidth = 0.0f;
    std::size_t lineCount = 1;

    for (char c : text) {
        if (c == '\n') {
            maxWidth = std::max(maxWidth, lineWidth);
            lineWidth = 0.0f;
            ++lineCount;
            continue;
        }

        const Glyph& glyph = font->GetGlyph(static_cast<uint32_t>(c));
        lineWidth += glyph.advance * scale;
    }

    maxWidth = std::max(maxWidth, lineWidth);
    float height = static_cast<float>(lineCount) * font->GetLineHeight() * scale;
    return Float2(maxWidth, height);
}

void OpenGLRenderBackend::PushBlendMode(BlendMode mode) {
    FlushPendingCommands("PushBlendMode");
    StateManagement::RenderStateManager::PushBlendMode(mode);
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

void OpenGLRenderBackend::PopBlendMode() {
    FlushPendingCommands("PopBlendMode");
    StateManagement::RenderStateManager::PopBlendMode();
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

void OpenGLRenderBackend::SetBlendMode(BlendMode mode) {
    FlushPendingCommands("SetBlendMode");
    StateManagement::RenderStateManager::SetBlendMode(mode);
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

[[nodiscard]] BlendMode OpenGLRenderBackend::GetBlendMode() const {
    return StateManagement::RenderStateManager::GetBlendMode();
}

void OpenGLRenderBackend::PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                         float biasConstant, float biasSlope) {
    DepthSettings settings{};
    settings.testEnabled = enableTest;
    settings.writeEnabled = enableWrite;
    settings.function = function;
    settings.biasConstant = biasConstant;
    settings.biasSlope = biasSlope;
    FlushPendingCommands("PushDepthState");
    StateManagement::RenderStateManager::PushDepthState(settings);
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

void OpenGLRenderBackend::PopDepthState() {
    FlushPendingCommands("PopDepthState");
    StateManagement::RenderStateManager::PopDepthState();
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

void OpenGLRenderBackend::SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                        float biasConstant, float biasSlope) {
    DepthSettings settings{};
    settings.testEnabled = enableTest;
    settings.writeEnabled = enableWrite;
    settings.function = function;
    settings.biasConstant = biasConstant;
    settings.biasSlope = biasSlope;
    FlushPendingCommands("SetDepthState");
    StateManagement::RenderStateManager::SetDepthState(settings);
    StateManagement::RenderStateManager::ApplyDirtyStates();
}

[[nodiscard]] DepthSettings OpenGLRenderBackend::GetDepthState() const {
    return StateManagement::RenderStateManager::GetDepthState();
}

} // namespace SAGE


