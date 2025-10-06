#include "TestFramework.h"

#include <SAGE.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    CHECK(configuredFx.pulseSpeed == TestFramework::Approx(0.0f));

    SAGE::Renderer::ConfigurePostFX(SAGE::PostFXSettings{});
}
