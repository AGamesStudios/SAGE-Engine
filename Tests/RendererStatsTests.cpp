// Unit tests for Renderer statistics including tile source classification
#include "TestFramework.h"
#include <SAGE.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace {
    class GlfwContext {
    public:
        GlfwContext() {
            initialized = glfwInit() == GLFW_TRUE;
            if (!initialized) return;
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            window = glfwCreateWindow(320, 240, "RendererStatsTests", nullptr, nullptr);
            if (!window) return;
            glfwMakeContextCurrent(window);
            gladLoaded = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0;
        }
        ~GlfwContext() {
            if (window) { glfwDestroyWindow(window); window = nullptr; }
            if (initialized) { glfwTerminate(); initialized = false; }
        }
        bool IsReady() const { return initialized && window && gladLoaded; }
    private:
        bool initialized = false;
        bool gladLoaded = false;
        GLFWwindow* window = nullptr;
    };

    class RendererGuard {
    public:
        RendererGuard() { SAGE::Renderer::Init(); initialized = true; }
        ~RendererGuard() { if (initialized) { SAGE::Renderer::Shutdown(); initialized = false; } }
    private:
        bool initialized = false;
    };
} // namespace

TEST_CASE(Renderer_RenderStats_QuadAndTextCounts) {
    GlfwContext glfw; ASSERT_TRUE(glfw.IsReady(), "GLFW init failed"); RendererGuard renderer;
    SAGE::Renderer::BeginScene();
    SAGE::QuadDesc quad{}; quad.position = {10,10}; quad.size = {50,40}; quad.color = SAGE::Color::Red(); quad.screenSpace = true;
    ASSERT_TRUE(SAGE::Renderer::DrawQuad(quad), "DrawQuad quad failed");
    SAGE::QuadDesc quad2{}; quad2.position = {100,15}; quad2.size = {20,20}; quad2.color = SAGE::Color::Green(); quad2.screenSpace = true;
    ASSERT_TRUE(SAGE::Renderer::DrawQuad(quad2), "DrawQuad quad2 failed");
    ASSERT_TRUE(SAGE::Renderer::EndScene(), "EndScene failed");
    auto stats = SAGE::Renderer::GetRenderStats();
    ASSERT_TRUE(stats.requestedQuads >= 2, "requestedQuads < 2");
}

TEST_CASE(Renderer_TileSource_RequestedTilesCount) {
    GlfwContext glfw; ASSERT_TRUE(glfw.IsReady(), "GLFW init failed"); RendererGuard renderer;
    auto* scene = SAGE::Renderer::GetSceneRenderer(); ASSERT_NOT_NULL(scene, "Scene renderer null");
    SAGE::Renderer::BeginScene();
    SAGE::QuadDesc tileQuad{}; tileQuad.position = {25,25}; tileQuad.size = {16,16}; tileQuad.screenSpace = true; tileQuad.source = SAGE::QuadDesc::QuadSource::Tile;
    ASSERT_TRUE(SAGE::Renderer::DrawQuad(tileQuad), "DrawQuad tileQuad failed");
    SAGE::Renderer::EndScene();
    auto stats = SAGE::Renderer::GetRenderStats();
    ASSERT_TRUE(stats.requestedQuads >= 1, "requestedQuads <1");
    ASSERT_EQ(1, stats.requestedTiles, "requestedTiles !=1");
}

// Force linker to include this translation unit (pattern similar to Matrix4Tests.cpp)
namespace { [[maybe_unused]] int __force_renderer_stats_tests = [](){ return 0; }(); }
