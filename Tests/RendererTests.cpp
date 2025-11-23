/**
 * @file RendererTests.cpp
 * @brief Unit tests for graphics color system
 */

#include "catch2.hpp"
#include "SAGE/Math/Color.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Core/CommandLine.h"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <string>
#include <system_error>

using namespace SAGE;

namespace {

class StubRenderBackend final : public RenderBackend {
public:
    void Initialize(const RendererConfig& config) override {
        initialized = true;
        receivedConfig = config;
    }

    void Shutdown() override { shutdown = true; }

    void BeginFrame() override {}
    void EndFrame() override {}
    void Clear(const Color&) override {}
    void SetViewport(int, int, int, int) override {}
    void SetRenderMode(RenderMode mode) override { modeState = mode; }
    RenderMode GetRenderMode() const override { return modeState; }

    void EnableBlending(bool) override {}
    void SetBlendFunc(uint32_t, uint32_t) override {}
    void DrawQuad(const Vector2&, const Vector2&, const Color&) override {}
    void DrawQuad(const Vector2&, const Vector2&, Texture*) override {}
    void DrawQuadTinted(const Vector2&, const Vector2&, const Color&, Texture*) override {}
    void DrawQuad(const Vector2&, const Vector2&, const Color&, Shader*) override {}
    void DrawQuadGradient(const Vector2&, const Vector2&, const Color&, const Color&, const Color&, const Color&) override {}
    void DrawLine(const Vector2&, const Vector2&, const Color&, float) override {}
    void DrawTriangle(const Vector2&, const Vector2&, const Vector2&, const Color&) override {}
    void DrawCircle(const Vector2&, float, const Color&) override {}

    void DrawSprite(const Sprite&) override {}
    void DrawSprite(const Sprite&, const Camera2D&) override {}

    void SetScissor(int x, int y, int width, int height) override {}
    void DisableScissor() override {}
    void PushScissor(int x, int y, int width, int height) override {}
    void PopScissor() override {}

    void BeginSpriteBatch(const Camera2D*) override {}
    void SubmitSprite(const Sprite&) override {}
    void FlushSpriteBatch() override {}

    void DrawParticle(const Vector2&, float, const Color&, float) override {}

    void SetProjectionMatrix(const Matrix3& projection) override { projectionMatrix = projection; }
    void SetViewMatrix(const Matrix3& view) override { viewMatrix = view; }
    void SetCamera(const Camera2D&) override {}

    const Matrix3& GetProjectionMatrix() const override { return projectionMatrix; }
    const Matrix3& GetViewMatrix() const override { return viewMatrix; }
    Matrix3 GetViewProjectionMatrix() const override { return projectionMatrix; }

    const RenderStats& GetStats() const override { return stats; }
    void ResetStats() override { stats = {}; }

    RendererConfig receivedConfig{};
    bool initialized = false;
    bool shutdown = false;
    RenderMode modeState = RenderMode::Solid;
    Matrix3 projectionMatrix = Matrix3::Identity();
    Matrix3 viewMatrix = Matrix3::Identity();
    RenderStats stats{};
};

void SetEnvVar(const char* name, const char* value) {
#if defined(_WIN32)
    if (value) {
        _putenv_s(name, value);
    } else {
        _putenv_s(name, "");
    }
#else
    if (value) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
#endif
}

std::filesystem::path WriteTempRendererConfig(const std::string& backendName) {
    const auto uniqueId = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto path = std::filesystem::temp_directory_path() /
        std::filesystem::path("sage_renderer_test_" + std::to_string(uniqueId) + ".json");
    std::ofstream out(path, std::ios::trunc);
    out << "{\n  \"backend\": \"" << backendName << "\"\n}";
    out.flush();
    return path;
}

struct TempFile {
    explicit TempFile(std::filesystem::path p) : path(std::move(p)) {}
    ~TempFile() {
        if (!path.empty()) {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    }

    std::filesystem::path path;
};

struct RendererTestCleanup {
    ~RendererTestCleanup() {
        Renderer::Shutdown();
        Renderer::SetBackendFactory(nullptr);
        CommandLine::ResetForTesting();
        SetEnvVar("SAGE_RENDERER_BACKEND", nullptr);
        SetEnvVar("SAGE_RENDERER_CONFIG", nullptr);
    }
};

} // namespace

TEST_CASE("Color - Predefined Colors", "[Graphics][Color]") {
    SECTION("Red color") {
        auto red = Color::Red();
        REQUIRE(red.r == 1.0f);
        REQUIRE(red.g == 0.0f);
        REQUIRE(red.b == 0.0f);
        REQUIRE(red.a == 1.0f);
    }

    SECTION("Green color") {
        auto green = Color::Green();
        REQUIRE(green.r == 0.0f);
        REQUIRE(green.g == 1.0f);
        REQUIRE(green.b == 0.0f);
        REQUIRE(green.a == 1.0f);
    }

    SECTION("Blue color") {
        auto blue = Color::Blue();
        REQUIRE(blue.r == 0.0f);
        REQUIRE(blue.g == 0.0f);
        REQUIRE(blue.b == 1.0f);
        REQUIRE(blue.a == 1.0f);
    }

    SECTION("White color") {
        auto white = Color::White();
        REQUIRE(white.r == 1.0f);
        REQUIRE(white.g == 1.0f);
        REQUIRE(white.b == 1.0f);
        REQUIRE(white.a == 1.0f);
    }

    SECTION("Black color") {
        auto black = Color::Black();
        REQUIRE(black.r == 0.0f);
        REQUIRE(black.g == 0.0f);
        REQUIRE(black.b == 0.0f);
        REQUIRE(black.a == 1.0f);
    }

    SECTION("Transparent color") {
        auto transparent = Color::Transparent();
        REQUIRE(transparent.a == 0.0f);
    }
}

TEST_CASE("Color - Custom Colors", "[Graphics][Color]") {
    SECTION("Create custom color") {
        Color custom{0.5f, 0.3f, 0.8f, 0.9f};
        
        REQUIRE(custom.r == 0.5f);
        REQUIRE(custom.g == 0.3f);
        REQUIRE(custom.b == 0.8f);
        REQUIRE(custom.a == 0.9f);
    }

    SECTION("Default alpha is 1.0") {
        Color color{0.2f, 0.4f, 0.6f};
        REQUIRE(color.a == 1.0f);
    }
}

TEST_CASE("Color - Operations", "[Graphics][Color]") {
    SECTION("Color equality") {
        Color c1{1.0f, 0.5f, 0.0f, 1.0f};
        Color c2{1.0f, 0.5f, 0.0f, 1.0f};
        Color c3{0.0f, 0.5f, 1.0f, 1.0f};
        
        REQUIRE(c1.r == c2.r);
        REQUIRE(c1.g == c2.g);
        REQUIRE(c1.r != c3.r);
    }

    SECTION("Color modification") {
        Color color = Color::Red();
        
        color.g = 0.5f;
        color.b = 0.25f;
        
        REQUIRE(color.r == 1.0f);
        REQUIRE(color.g == 0.5f);
        REQUIRE(color.b == 0.25f);
    }
}

TEST_CASE("Renderer honors explicit backend when overrides disabled", "[Renderer][Config]") {
    RendererTestCleanup cleanup;
    CommandLine::ResetForTesting();

    StubRenderBackend* backendInstance = nullptr;
    RenderBackendType factoryType = RenderBackendType::OpenGL;
    Renderer::SetBackendFactory([&](RenderBackendType type) {
        factoryType = type;
        auto backend = std::make_unique<StubRenderBackend>();
        backendInstance = backend.get();
        return backend;
    });

    RendererConfig config{};
    config.backend = RenderBackendType::Vulkan;
    config.enableRuntimeOverrides = false;

    Renderer::Init(config);

    REQUIRE(backendInstance != nullptr);
    REQUIRE(factoryType == RenderBackendType::Vulkan);
    REQUIRE(backendInstance->receivedConfig.backend == RenderBackendType::Vulkan);
    REQUIRE(Renderer::GetConfig().backend == RenderBackendType::Vulkan);
}

TEST_CASE("Renderer prefers CLI override over environment", "[Renderer][Config]") {
    RendererTestCleanup cleanup;

    StubRenderBackend* backendInstance = nullptr;
    Renderer::SetBackendFactory([&](RenderBackendType) {
        auto backend = std::make_unique<StubRenderBackend>();
        backendInstance = backend.get();
        return backend;
    });

    CommandLine::OverrideForTesting({"SAGE_Tests", "--renderer-backend=vulkan"});
    SetEnvVar("SAGE_RENDERER_BACKEND", "opengl");

    RendererConfig config{};
    config.backend = RenderBackendType::OpenGL;
    config.enableRuntimeOverrides = true;

    Renderer::Init(config);

    REQUIRE(backendInstance != nullptr);
    REQUIRE(backendInstance->receivedConfig.backend == RenderBackendType::Vulkan);
    REQUIRE(Renderer::GetConfig().backend == RenderBackendType::Vulkan);
}

TEST_CASE("Renderer loads backend from config file when enabled", "[Renderer][Config]") {
    RendererTestCleanup cleanup;

    StubRenderBackend* backendInstance = nullptr;
    Renderer::SetBackendFactory([&](RenderBackendType) {
        auto backend = std::make_unique<StubRenderBackend>();
        backendInstance = backend.get();
        return backend;
    });

    TempFile configFile{WriteTempRendererConfig("vulkan")};

    RendererConfig config{};
    config.backend = RenderBackendType::OpenGL;
    config.configFile = configFile.path;
    config.enableRuntimeOverrides = true;

    Renderer::Init(config);

    REQUIRE(backendInstance != nullptr);
    REQUIRE(backendInstance->receivedConfig.backend == RenderBackendType::Vulkan);
    REQUIRE(Renderer::GetConfig().backend == RenderBackendType::Vulkan);
}

TEST_CASE("Renderer can disable runtime overrides", "[Renderer][Config]") {
    RendererTestCleanup cleanup;

    StubRenderBackend* backendInstance = nullptr;
    Renderer::SetBackendFactory([&](RenderBackendType) {
        auto backend = std::make_unique<StubRenderBackend>();
        backendInstance = backend.get();
        return backend;
    });

    CommandLine::OverrideForTesting({"SAGE_Tests", "--renderer-backend=vulkan"});
    SetEnvVar("SAGE_RENDERER_BACKEND", "opengl");

    RendererConfig config{};
    config.backend = RenderBackendType::OpenGL;
    config.enableRuntimeOverrides = false;

    Renderer::Init(config);

    REQUIRE(backendInstance != nullptr);
    REQUIRE(backendInstance->receivedConfig.backend == RenderBackendType::OpenGL);
    REQUIRE(Renderer::GetConfig().backend == RenderBackendType::OpenGL);
}
