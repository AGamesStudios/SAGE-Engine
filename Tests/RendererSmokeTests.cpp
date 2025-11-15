#include "TestFramework.h"

#include <SAGE.h>

#include <Engine/Graphics/Backend/Interfaces/IRenderDevice.h>
#include <Engine/Graphics/Backend/Interfaces/IResourceManager.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
using namespace TestFramework;

namespace {

class GlfwContext {
public:
    GlfwContext() {
        initialized = glfwInit() == GLFW_TRUE;
        if (!initialized) {
            return;
        }

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 480, "RendererSmokeTests", nullptr, nullptr);
        if (!window) {
            return;
        }

        glfwMakeContextCurrent(window);
        gladLoaded = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0;
    }

    ~GlfwContext() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        if (initialized) {
            glfwTerminate();
            initialized = false;
        }
    }

    [[nodiscard]] bool IsReady() const {
        return initialized && window && gladLoaded;
    }

private:
    bool initialized = false;
    bool gladLoaded = false;
    GLFWwindow* window = nullptr;
};

class RendererGuard {
public:
    RendererGuard() {
        SAGE::Renderer::Init();
        initialized = true;
    }

    ~RendererGuard() {
        if (initialized) {
            SAGE::Renderer::Shutdown();
            initialized = false;
        }
    }

private:
    bool initialized = false;
};

} // namespace

TEST_CASE(Renderer_CameraAndPostFXSmoke) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    RendererGuard renderer;

    SAGE::Renderer::ResetCamera();
    const SAGE::Camera2D& defaultCamera = SAGE::Renderer::GetCamera();
    CHECK(defaultCamera.position.x == 0.0f);
    CHECK(defaultCamera.position.y == 0.0f);
    CHECK(defaultCamera.zoom == TestFramework::Approx(1.0f));

    SAGE::Camera2D camera;
    camera.position = { 42.0f, -13.5f };
    camera.zoom = 2.5f;
    SAGE::Renderer::SetCamera(camera);

    const SAGE::Camera2D& configuredCamera = SAGE::Renderer::GetCamera();
    CHECK(configuredCamera.position.x == TestFramework::Approx(42.0f));
    CHECK(configuredCamera.position.y == TestFramework::Approx(-13.5f));
    CHECK(configuredCamera.zoom == TestFramework::Approx(2.5f));

    SAGE::Renderer::ResetCamera();
    const SAGE::Camera2D& resetCamera = SAGE::Renderer::GetCamera();
    CHECK(resetCamera.position.x == 0.0f);
    CHECK(resetCamera.position.y == 0.0f);
    CHECK(resetCamera.zoom == TestFramework::Approx(1.0f));

    SAGE::PostFXSettings settings;
    settings.enabled = false;
    const SAGE::Color expectedTint{ 1.0f, 0.0f, 1.0f, 1.0f };
    settings.tint = expectedTint;
    settings.intensity = 1.5f;
    settings.bloomThreshold = -0.25f;
    settings.bloomStrength = -1.0f;
    settings.blurIterations = -3;
    settings.gamma = -0.5f;
    settings.exposure = -1.0f;
    settings.pulseSpeed = -2.0f;
    SAGE::Renderer::ConfigurePostFX(settings);

    const SAGE::PostFXSettings& configuredFx = SAGE::Renderer::GetPostFXSettings();
    CHECK(configuredFx.enabled == false);
    CHECK(configuredFx.tint.r == TestFramework::Approx(expectedTint.r));
    CHECK(configuredFx.tint.g == TestFramework::Approx(expectedTint.g));
    CHECK(configuredFx.tint.b == TestFramework::Approx(expectedTint.b));
    CHECK(configuredFx.tint.a == TestFramework::Approx(expectedTint.a));
    CHECK(configuredFx.intensity == TestFramework::Approx(1.0f));
    CHECK(configuredFx.bloomThreshold == TestFramework::Approx(0.0f));
    CHECK(configuredFx.bloomStrength == TestFramework::Approx(0.0f));
    CHECK(configuredFx.blurIterations == 0);
    CHECK(configuredFx.gamma == TestFramework::Approx(0.001f));
    CHECK(configuredFx.exposure == TestFramework::Approx(0.0f));
    CHECK(configuredFx.pulseSpeed == TestFramework::Approx(0.0f));

    SAGE::Renderer::ConfigurePostFX(SAGE::PostFXSettings{});
}

TEST_CASE(Renderer_PostFXToggleSmoke) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    RendererGuard renderer;

    SAGE::PostFXSettings settings;
    settings.enabled = true;
    settings.intensity = 0.4f;
    settings.bloomThreshold = 0.25f;
    settings.bloomStrength = 0.9f;
    settings.blurIterations = 3;
    settings.gamma = 2.0f;
    settings.exposure = 1.25f;
    settings.pulseSpeed = 1.2f;
    settings.tint = SAGE::Color{ 0.2f, 0.6f, 1.0f, 1.0f };

    SAGE::Renderer::ConfigurePostFX(settings);
    SAGE::Renderer::EnablePostFX(true);

    SAGE::Renderer::BeginScene();
    CHECK(SAGE::Renderer::EndScene());
    const SAGE::PostFXSettings& configured = SAGE::Renderer::GetPostFXSettings();
    CHECK(configured.blurIterations == 3);
    CHECK(configured.bloomStrength == TestFramework::Approx(0.9f));
    CHECK(configured.gamma == TestFramework::Approx(2.0f));
    CHECK(configured.exposure == TestFramework::Approx(1.25f));

    SAGE::Renderer::EnablePostFX(false);
    SAGE::Renderer::BeginScene();
    CHECK(SAGE::Renderer::EndScene());
}

TEST_CASE(Renderer_PostFXBlurSmoke) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    RendererGuard renderer;

    SAGE::PostFXSettings settings;
    settings.enabled = true;
    settings.intensity = 0.35f;
    settings.tint = SAGE::Color{ 0.9f, 0.7f, 0.3f, 1.0f };
    settings.bloomThreshold = 0.6f;
    settings.bloomStrength = 1.1f;
    settings.blurIterations = 4;
    settings.gamma = 2.4f;
    settings.exposure = 1.4f;
    settings.pulseSpeed = 0.5f;

    SAGE::Renderer::ConfigurePostFX(settings);
    SAGE::Renderer::EnablePostFX(true);

    SAGE::Renderer::BeginScene();
    CHECK(SAGE::Renderer::EndScene());

    const auto& fx = SAGE::Renderer::GetPostFXSettings();
    CHECK(fx.blurIterations == 4);
    CHECK(fx.enabled == true);

    SAGE::Renderer::EnablePostFX(false);
}

TEST_CASE(Renderer_DeviceAdapterSmoke) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    RendererGuard renderer;

    auto* device = SAGE::Renderer::GetDevice();
    REQUIRE(device != nullptr);
    CHECK(device->IsInitialized());

    auto* resources = SAGE::Renderer::GetResourceManager();
    REQUIRE(resources != nullptr);

    const char* vertexSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Position;
void main() {
    gl_Position = vec4(a_Position, 1.0);
}
)";

    const char* fragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 o_Color;
void main() {
    o_Color = vec4(1.0, 0.5, 0.25, 1.0);
}
)";

    SAGE::Graphics::ShaderSource shaderSource{
        vertexSrc,
        fragmentSrc,
        "RendererDeviceSmoke"
    };

    auto shaderHandle = resources->LoadShader("renderer_device_smoke_shader", shaderSource);
    CHECK(shaderHandle != 0);
    CHECK(resources->TryGetShader("renderer_device_smoke_shader").has_value());

    std::vector<std::uint8_t> texels(4 * 4 * 4, 255);
    SAGE::Graphics::TextureDesc textureDesc;
    textureDesc.width = 4;
    textureDesc.height = 4;
    textureDesc.generateMipmaps = false;

    SAGE::Graphics::TextureDataView textureData{
        texels.data(),
        texels.size()
    };

    auto textureHandle = resources->LoadTexture("renderer_device_smoke_texture", textureDesc, textureData);
    CHECK(textureHandle != 0);
    CHECK(resources->TryGetTexture("renderer_device_smoke_texture").has_value());

    SAGE::Graphics::MaterialDesc materialDesc{};
    materialDesc.shader = shaderHandle;
    materialDesc.diffuseTexture = textureHandle;
    auto materialHandle = resources->CreateMaterial("renderer_device_smoke_material", materialDesc);
    CHECK(materialHandle != 0);
    CHECK(resources->TryGetMaterial("renderer_device_smoke_material").has_value());

    resources->DestroyMaterial(materialHandle);
    CHECK_FALSE(resources->TryGetMaterial("renderer_device_smoke_material").has_value());

    resources->DestroyTexture(textureHandle);
    CHECK_FALSE(resources->TryGetTexture("renderer_device_smoke_texture").has_value());

    resources->DestroyShader(shaderHandle);
    CHECK_FALSE(resources->TryGetShader("renderer_device_smoke_shader").has_value());
}
