#include "TestFramework.h"

#include "Engine/Graphics/API/RenderSystem.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>

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

        window = glfwCreateWindow(640, 480, "RenderPerformanceTests", nullptr, nullptr);
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

} // namespace

TEST_CASE(RenderPerformance_TenThousandSprites) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    SAGE::Graphics::RenderSystem renderSystem;
    SAGE::Graphics::RenderConfig config;
    config.initialCommandCapacity = 10000;
    REQUIRE(renderSystem.Initialize(config));

    renderSystem.BeginFrame();

    for (std::size_t i = 0; i < 10000; ++i) {
        SAGE::QuadDesc quad;
        quad.position = { static_cast<float>(i % 100), static_cast<float>(i / 100) };
        quad.size = { 1.0f, 1.0f };
        quad.color = SAGE::Color::White();
        quad.screenSpace = false;

        SAGE::Graphics::RenderCommand command;
        command.layer = SAGE::Graphics::RenderLayerHandle::Invalid();
        command.quad = quad;
        renderSystem.Submit(command);
    }

    renderSystem.EndFrame();

    const SAGE::Graphics::RenderStats stats = renderSystem.GetStats();
    CHECK(stats.submittedQuads == 10000);
    CHECK(stats.executedDrawCalls == 10000);
    CHECK(stats.frameTimeMs < 16.67f);

    renderSystem.Shutdown();
}
