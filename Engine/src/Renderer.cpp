#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Core/CommandLine.h"
#include "SAGE/Log.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <nlohmann/json.hpp>

#include "Graphics/OpenGL/OpenGLRenderBackend.h"

namespace SAGE {

using json = nlohmann::json;

namespace {

std::unique_ptr<RenderBackend> CreateBackend(RenderBackendType type) {
    switch (type) {
        case RenderBackendType::OpenGL:
            return std::make_unique<OpenGLRenderBackend>();
        case RenderBackendType::Vulkan:
            SAGE_ERROR("Vulkan backend not implemented yet");
            return nullptr;
        default:
            SAGE_ERROR("Unknown render backend type");
            return nullptr;
    }
}

struct RendererState {
    RendererConfig requestedConfig{};
    RendererConfig resolvedConfig{};
    std::unique_ptr<RenderBackend> backend;
    Renderer::BackendFactory backendFactory;
    bool autoProjectionActive = true;
    bool originTopLeft = true;
    int viewportWidth = 0;
    int viewportHeight = 0;
};

RendererState& GetState() {
    static RendererState state;
    if (!state.backendFactory) {
        state.backendFactory = CreateBackend;
    }
    return state;
}

void ApplyAutoProjection(RendererState& state, int width, int height) {
    if (!state.backend || !state.autoProjectionActive) {
        return;
    }
    if (width <= 0 || height <= 0) {
        return;
    }

    const float left = 0.0f;
    const float right = static_cast<float>(width);
    const float bottom = state.originTopLeft ? static_cast<float>(height) : 0.0f;
    const float top = state.originTopLeft ? 0.0f : static_cast<float>(height);

    state.backend->SetProjectionMatrix(Matrix3::Ortho(left, right, bottom, top));
    state.backend->SetViewMatrix(Matrix3::Identity());
}

RenderBackend* RequireBackend(const char* functionName) {
    auto* backend = GetState().backend.get();
    if (!backend) {
        SAGE_ERROR("Renderer::{} called before Renderer::Init", functionName);
    }
    return backend;
}

std::string ToLower(std::string_view value) {
    std::string lowered;
    lowered.reserve(value.size());
    for (char ch : value) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return lowered;
}

std::optional<RenderBackendType> ParseBackendString(std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }
    const auto lowered = ToLower(value);
    if (lowered == "opengl" || lowered == "gl") {
        return RenderBackendType::OpenGL;
    }
    if (lowered == "vulkan" || lowered == "vk") {
        return RenderBackendType::Vulkan;
    }
    return std::nullopt;
}

std::optional<json> LoadJsonFile(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return std::nullopt;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        SAGE_WARN("Failed to open renderer config file: {}", path.string());
        return std::nullopt;
    }

    try {
        json data;
        file >> data;
        return data;
    } catch (const std::exception& e) {
        SAGE_WARN("Failed to parse renderer config {}: {}", path.string(), e.what());
        return std::nullopt;
    }
}

std::optional<std::filesystem::path> ResolveConfigPath(const std::filesystem::path& preferred) {
    if (preferred.empty()) {
        return std::nullopt;
    }

    std::error_code ec;
    if (std::filesystem::exists(preferred, ec)) {
        return preferred;
    }

    const auto candidate = std::filesystem::current_path() / preferred;
    if (std::filesystem::exists(candidate, ec)) {
        return candidate;
    }

    return std::nullopt;
}

std::optional<RenderBackendType> BackendFromJson(const json& data) {
    if (data.contains("renderer")) {
        const auto& renderer = data["renderer"];
        if (renderer.contains("backend") && renderer["backend"].is_string()) {
            return ParseBackendString(renderer["backend"].get<std::string>());
        }
    } else if (data.contains("backend") && data["backend"].is_string()) {
        return ParseBackendString(data["backend"].get<std::string>());
    }
    return std::nullopt;
}

RendererConfig ApplyRuntimeOverrides(const RendererConfig& base) {
    RendererConfig resolved = base;

    const char* configEnv = std::getenv("SAGE_RENDERER_CONFIG");
    if (configEnv && *configEnv != '\0') {
        resolved.configFile = configEnv;
    }

    if (!resolved.configFile.empty()) {
        if (auto path = ResolveConfigPath(resolved.configFile)) {
            if (auto jsonData = LoadJsonFile(*path)) {
                if (auto backend = BackendFromJson(*jsonData)) {
                    resolved.backend = *backend;
                    SAGE_INFO("Renderer backend overridden by config file: {}", ToString(resolved.backend));
                }
            }
        }
    }

    if (const char* envBackend = std::getenv("SAGE_RENDERER_BACKEND")) {
        if (auto backend = ParseBackendString(envBackend)) {
            resolved.backend = *backend;
            SAGE_INFO("Renderer backend overridden by environment: {}", ToString(resolved.backend));
        }
    }

    if (auto option = CommandLine::GetOption("renderer-backend")) {
        if (auto backend = ParseBackendString(*option)) {
            resolved.backend = *backend;
            SAGE_INFO("Renderer backend overridden by CLI: {}", ToString(resolved.backend));
        }
    }

    return resolved;
}

} // namespace

const char* ToString(RenderBackendType type) {
    switch (type) {
        case RenderBackendType::OpenGL:
            return "OpenGL";
        case RenderBackendType::Vulkan:
            return "Vulkan";
        default:
            return "Unknown";
    }
}

RenderBackendType RenderBackendTypeFromString(const std::string& name, RenderBackendType fallback) {
    if (auto parsed = ParseBackendString(name)) {
        return *parsed;
    }
    return fallback;
}

void Renderer::Init(const RendererConfig& config) {
    auto& state = GetState();
    if (state.backend) {
        SAGE_WARN("Renderer already initialized");
        return;
    }

    state.requestedConfig = config;
    state.resolvedConfig = config;
    if (state.resolvedConfig.enableRuntimeOverrides) {
        state.resolvedConfig = ApplyRuntimeOverrides(state.requestedConfig);
    }
    state.autoProjectionActive = state.resolvedConfig.autoConfigurePixelProjection;
    state.originTopLeft = state.resolvedConfig.pixelOriginTopLeft;

    auto factory = state.backendFactory;
    if (!factory) {
        factory = CreateBackend;
    }

    state.backend = factory(state.resolvedConfig.backend);
    if (!state.backend) {
        SAGE_ERROR("Failed to create renderer backend");
        return;
    }

    SAGE_INFO("Renderer initializing {} backend", ToString(state.resolvedConfig.backend));
    state.backend->Initialize(state.resolvedConfig);

    TextRenderer::Init();
}

void Renderer::Shutdown() {
    TextRenderer::Shutdown();

    auto& state = GetState();
    if (!state.backend) {
        return;
    }

    state.backend->Shutdown();
    state.backend.reset();
    state.viewportWidth = 0;
    state.viewportHeight = 0;
    state.autoProjectionActive = true;
}

RendererConfig Renderer::GetConfig() {
    return GetState().resolvedConfig;
}

void Renderer::SetBackendFactory(BackendFactory factory) {
    auto& state = GetState();
    if (state.backend) {
        SAGE_WARN("Renderer::SetBackendFactory called after initialization");
        return;
    }
    if (factory) {
        state.backendFactory = std::move(factory);
    } else {
        state.backendFactory = CreateBackend;
    }
}

void Renderer::BeginFrame() {
    if (auto* backend = RequireBackend("BeginFrame")) {
        backend->BeginFrame();
    }
}

void Renderer::EndFrame() {
    if (auto* backend = RequireBackend("EndFrame")) {
        backend->EndFrame();
    }
}

void Renderer::Clear(const Color& color) {
    if (auto* backend = RequireBackend("Clear")) {
        backend->Clear(color);
    }
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    if (auto* backend = RequireBackend("SetViewport")) {
        backend->SetViewport(x, y, width, height);
        auto& state = GetState();
        state.viewportWidth = width;
        state.viewportHeight = height;
        if (state.autoProjectionActive) {
            ApplyAutoProjection(state, width, height);
        }
    }
}

void Renderer::SetScissor(int x, int y, int width, int height) {
    if (auto* backend = RequireBackend("SetScissor")) {
        backend->SetScissor(x, y, width, height);
    }
}

void Renderer::PushScissor(int x, int y, int width, int height) {
    if (auto* backend = RequireBackend("PushScissor")) {
        backend->PushScissor(x, y, width, height);
    }
}

void Renderer::PopScissor() {
    if (auto* backend = RequireBackend("PopScissor")) {
        backend->PopScissor();
    }
}

void Renderer::DisableScissor() {
    if (auto* backend = RequireBackend("DisableScissor")) {
        backend->DisableScissor();
    }
}

void Renderer::SetRenderMode(RenderMode mode) {
    if (auto* backend = RequireBackend("SetRenderMode")) {
        backend->SetRenderMode(mode);
    }
}

RenderMode Renderer::GetRenderMode() {
    if (auto* backend = RequireBackend("GetRenderMode")) {
        return backend->GetRenderMode();
    }
    return RenderMode::Solid;
}

void Renderer::EnableBlending(bool enabled) {
    if (auto* backend = RequireBackend("EnableBlending")) {
        backend->EnableBlending(enabled);
    }
}

void Renderer::SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor) {
    if (auto* backend = RequireBackend("SetBlendFunc")) {
        backend->SetBlendFunc(srcFactor, dstFactor);
    }
}

void Renderer::ConfigureAutoProjection(bool enabled, bool originTopLeft) {
    auto& state = GetState();
    state.autoProjectionActive = enabled;
    state.originTopLeft = originTopLeft;
    if (enabled) {
        ApplyAutoProjection(state, state.viewportWidth, state.viewportHeight);
    }
}

void Renderer::DrawQuad(const Vector2& position, const Vector2& size, const Color& color) {
    if (auto* backend = RequireBackend("DrawQuad")) {
        backend->DrawQuad(position, size, color);
    }
}

void Renderer::DrawQuad(const Vector2& position, const Vector2& size, Texture* texture) {
    if (auto* backend = RequireBackend("DrawQuadTexture")) {
        backend->DrawQuad(position, size, texture);
    }
}

void Renderer::DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Texture* texture) {
    if (auto* backend = RequireBackend("DrawQuadTextureColor")) {
        backend->DrawQuadTinted(position, size, color, texture);
    }
}

void Renderer::DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Shader* shader) {
    if (auto* backend = RequireBackend("DrawQuadCustomShader")) {
        backend->DrawQuad(position, size, color, shader);
    }
}

void Renderer::DrawRect(const Vector2& position, const Vector2& size, const Color& fillColor, float outlineThickness, const Color& outlineColor) {
    // Draw filled rect
    if (fillColor.a > 0.0f) {
        DrawQuad(position, size, fillColor);
    }

    // Draw outline
    if (outlineThickness > 0.0f && outlineColor.a > 0.0f) {
        Vector2 halfSize = size * 0.5f;
        // Corners
        Vector2 c1 = position - halfSize;
        Vector2 c2 = { position.x + halfSize.x, position.y - halfSize.y };
        Vector2 c3 = position + halfSize;
        Vector2 c4 = { position.x - halfSize.x, position.y + halfSize.y };

        DrawLine(c1, c2, outlineColor, outlineThickness);
        DrawLine(c2, c3, outlineColor, outlineThickness);
        DrawLine(c3, c4, outlineColor, outlineThickness);
        DrawLine(c4, c1, outlineColor, outlineThickness);
    }
}

void Renderer::DrawRect(const Rect& rect, const Color& fillColor, float outlineThickness, const Color& outlineColor) {
    DrawRect(rect.GetCenter(), rect.GetSize(), fillColor, outlineThickness, outlineColor);
}

void Renderer::DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness) {
    if (auto* backend = RequireBackend("DrawLine")) {
        backend->DrawLine(start, end, color, thickness);
    }
}

void Renderer::DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) {
    // TODO: Add backend support for triangles
    // For now, draw wireframe
    DrawLine(p1, p2, color);
    DrawLine(p2, p3, color);
    DrawLine(p3, p1, color);
}

void Renderer::DrawCircle(const Vector2& center, float radius, const Color& color) {
    DrawCircle(center, radius, color, 0.0f, Color::Transparent());
}

void Renderer::DrawCircle(const Vector2& center, float radius, const Color& fillColor, float outlineThickness, const Color& outlineColor) {
    const int segments = 32;
    const float angleStep = 6.283185f / segments;

    // Filled (Approximation with lines for now as we lack backend support for generic polys)
    // Ideally we should add DrawTriangle to backend and use a fan.
    // Or use a quad with a circle shader.
    if (fillColor.a > 0.0f) {
        // Fallback: Draw a quad with the circle color (square circle!) or just outline if we can't fill.
        // Better: Draw a quad with a circle shader if we had one.
        // For now, let's just draw the outline if fill is requested, or maybe a dense web of lines? No.
        // Let's just warn or do nothing for fill if we can't.
        // Actually, let's try to use DrawQuad with a "Circle" texture if available? No.
        
        // Temporary: Draw a quad as a placeholder for filled circle
        // DrawQuad(center, {radius*2, radius*2}, fillColor); 
    }

    // Outline
    Color strokeColor = (outlineThickness > 0.0f) ? outlineColor : fillColor;
    float thickness = (outlineThickness > 0.0f) ? outlineThickness : 1.0f;
    
    if (outlineThickness > 0.0f || fillColor.a <= 0.0f) {
        for (int i = 0; i < segments; ++i) {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;
            Vector2 p1 = center + Vector2(std::cos(angle1), std::sin(angle1)) * radius;
            Vector2 p2 = center + Vector2(std::cos(angle2), std::sin(angle2)) * radius;
            DrawLine(p1, p2, strokeColor, thickness);
        }
    }
}

void Renderer::DrawSprite(const Sprite& sprite) {
    if (auto* backend = RequireBackend("DrawSprite")) {
        backend->DrawSprite(sprite);
    }
}

void Renderer::DrawSprite(const Sprite& sprite, const Camera2D& camera) {
    if (auto* backend = RequireBackend("DrawSpriteCamera")) {
        backend->DrawSprite(sprite, camera);
    }
}

void Renderer::BeginSpriteBatch(const Camera2D* camera) {
    if (auto* backend = RequireBackend("BeginSpriteBatch")) {
        backend->BeginSpriteBatch(camera);
    }
}

void Renderer::SubmitSprite(const Sprite& sprite) {
    if (auto* backend = RequireBackend("SubmitSprite")) {
        backend->SubmitSprite(sprite);
    }
}

void Renderer::FlushSpriteBatch() {
    if (auto* backend = RequireBackend("FlushSpriteBatch")) {
        backend->FlushSpriteBatch();
    }
}

void Renderer::DrawParticle(const Vector2& position, float size, const Color& color, float rotation) {
    if (auto* backend = RequireBackend("DrawParticle")) {
        backend->DrawParticle(position, size, color, rotation);
    }
}

void Renderer::DrawText(const std::string& text, const Vector2& position, const Color& color, std::shared_ptr<Font> font) {
    TextRenderer::DrawText(text, position, color, font);
}

void Renderer::DrawTextAligned(const std::string& text, const Vector2& position, TextAlign align, const Color& color, std::shared_ptr<Font> font) {
    TextRenderer::DrawTextAligned(text, position, align, color, font);
}

void Renderer::SetProjectionMatrix(const Matrix3& projection) {
    if (auto* backend = RequireBackend("SetProjectionMatrix")) {
        GetState().autoProjectionActive = false;
        backend->SetProjectionMatrix(projection);
    }
}

void Renderer::SetViewMatrix(const Matrix3& view) {
    if (auto* backend = RequireBackend("SetViewMatrix")) {
        GetState().autoProjectionActive = false;
        backend->SetViewMatrix(view);
    }
}

void Renderer::SetCamera(const Camera2D& camera) {
    if (auto* backend = RequireBackend("SetCamera")) {
        GetState().autoProjectionActive = false;
        backend->SetCamera(camera);
    }
}

const Matrix3& Renderer::GetProjectionMatrix() {
    if (auto* backend = RequireBackend("GetProjectionMatrix")) {
        return backend->GetProjectionMatrix();
    }

    static Matrix3 identity = Matrix3::Identity();
    return identity;
}

const Matrix3& Renderer::GetViewMatrix() {
    if (auto* backend = RequireBackend("GetViewMatrix")) {
        return backend->GetViewMatrix();
    }

    static Matrix3 identity = Matrix3::Identity();
    return identity;
}

Matrix3 Renderer::GetViewProjectionMatrix() {
    if (auto* backend = RequireBackend("GetViewProjectionMatrix")) {
        return backend->GetViewProjectionMatrix();
    }
    return Matrix3::Identity();
}

const RenderStats& Renderer::GetStats() {
    return GetState().backend->GetStats();
}

void Renderer::ResetStats() {
    GetState().backend->ResetStats();
}

RenderBackend* Renderer::GetBackend() {
    return GetState().backend.get();
}

} // namespace SAGE
