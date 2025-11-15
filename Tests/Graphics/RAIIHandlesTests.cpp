#include "TestFramework.h"

#include "Engine/Graphics/GraphicsResourceManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
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

        window = glfwCreateWindow(64, 64, "RAIIHandlesTests", nullptr, nullptr);
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

void ResetGraphicsResourceManager() {
    SAGE::GraphicsResourceManager::Shutdown();
    SAGE::GraphicsResourceManager::Init();
}

} // namespace

TEST_CASE(RAIIHandles_TextureDefaultConstruction) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
    CHECK(texture.Get() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_TextureCreateAndDestroy) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    {
        SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
        texture.Create("TestTexture");
        CHECK(texture.Get() != 0u);
        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 1u);
    }

    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_TextureMoveSemantics) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    unsigned int originalID = 0;
    {
        SAGE::GraphicsResourceManager::TrackedTextureHandle texture1;
        texture1.Create("Texture1");
        originalID = texture1.Get();
        CHECK(texture1.Get() != 0u);
        CHECK(originalID != 0u);
        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 1u);

        SAGE::GraphicsResourceManager::TrackedTextureHandle texture2 = std::move(texture1);
        CHECK(texture1.Get() == 0u);
        CHECK(texture2.Get() != 0u);
        CHECK(texture2.Get() == originalID);
        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 1u);
    }

    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_MultipleResourceTypes) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    {
        SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
        texture.Create("MultiTexture");

        SAGE::GraphicsResourceManager::TrackedBufferHandle buffer;
        buffer.Create("MultiBuffer");

        SAGE::GraphicsResourceManager::TrackedVertexArrayHandle vao;
        vao.Create("MultiVAO");

        SAGE::GraphicsResourceManager::TrackedFramebufferHandle framebuffer;
        framebuffer.Create("MultiFBO");

        SAGE::GraphicsResourceManager::TrackedRenderbufferHandle renderbuffer;
        renderbuffer.Create("MultiRBO");

        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 1u);
        CHECK(SAGE::GraphicsResourceManager::ActiveBufferCount() == 1u);
        CHECK(SAGE::GraphicsResourceManager::ActiveVertexArrayCount() == 1u);
        CHECK(SAGE::GraphicsResourceManager::ActiveFramebufferCount() == 1u);
        CHECK(SAGE::GraphicsResourceManager::ActiveRenderbufferCount() == 1u);
        CHECK(SAGE::GraphicsResourceManager::TotalTrackedHandleCount() == 5u);
    }

    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());
    CHECK(SAGE::GraphicsResourceManager::TotalTrackedHandleCount() == 0u);

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_TextureStressCreateDestroy) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    {
        std::vector<SAGE::GraphicsResourceManager::TrackedTextureHandle> textures;
        textures.reserve(50);

        for (int i = 0; i < 50; ++i) {
            textures.emplace_back();
            textures.back().Create("Texture_" + std::to_string(i));
            CHECK(textures.back().Get() != 0u);
        }

        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 50u);
    }

    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_TextureDoubleDeleteSafe) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    unsigned int releasedID = 0;
    {
        SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
        texture.Create("DoubleDelete");
        CHECK(texture.Get() != 0u);
        releasedID = texture.Get();
    }

    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    if (releasedID != 0u) {
        GLuint externalID = releasedID;
        glDeleteTextures(1, &externalID);
    }

    CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == 0u);
    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_DefaultHandleOperations) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
    SAGE::GraphicsResourceManager::TrackedBufferHandle buffer;
    SAGE::GraphicsResourceManager::TrackedVertexArrayHandle vao;

    CHECK(texture.Get() == 0u);
    CHECK(buffer.Get() == 0u);
    CHECK(vao.Get() == 0u);

    CHECK(texture.Release() == 0u);
    CHECK(buffer.Release() == 0u);
    CHECK(vao.Release() == 0u);

    texture.Reset();
    buffer.Reset();
    vao.Reset();

    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_TextureBindingValidation) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    {
        SAGE::GraphicsResourceManager::TrackedTextureHandle texture;
        texture.Create("BindingValidation");
        const unsigned int id = texture.Get();
        CHECK(id != 0u);

        glBindTexture(GL_TEXTURE_2D, id);

        GLint boundTexture = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
        CHECK(boundTexture == static_cast<GLint>(id));

        glBindTexture(GL_TEXTURE_2D, 0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
        CHECK(boundTexture == 0);
    }

    CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());

    ResetGraphicsResourceManager();
}

TEST_CASE(RAIIHandles_MassResourceLifecycle) {
    GlfwContext context;
    REQUIRE(context.IsReady());

    ResetGraphicsResourceManager();

    constexpr int iterations = 5;
    constexpr int resourcesPerIteration = 40;

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<SAGE::GraphicsResourceManager::TrackedTextureHandle> textures;
        std::vector<SAGE::GraphicsResourceManager::TrackedBufferHandle> buffers;
        std::vector<SAGE::GraphicsResourceManager::TrackedVertexArrayHandle> vaos;

        textures.reserve(resourcesPerIteration);
        buffers.reserve(resourcesPerIteration);
        vaos.reserve(resourcesPerIteration);

        for (int i = 0; i < resourcesPerIteration; ++i) {
            textures.emplace_back();
            textures.back().Create("MassTex_" + std::to_string(iter * resourcesPerIteration + i));

            buffers.emplace_back();
            buffers.back().Create("MassBuf_" + std::to_string(iter * resourcesPerIteration + i));

            vaos.emplace_back();
            vaos.back().Create("MassVAO_" + std::to_string(iter * resourcesPerIteration + i));
        }

        CHECK(SAGE::GraphicsResourceManager::ActiveTextureCount() == static_cast<std::size_t>(resourcesPerIteration));
        CHECK(SAGE::GraphicsResourceManager::ActiveBufferCount() == static_cast<std::size_t>(resourcesPerIteration));
        CHECK(SAGE::GraphicsResourceManager::ActiveVertexArrayCount() == static_cast<std::size_t>(resourcesPerIteration));

        textures.clear();
        buffers.clear();
        vaos.clear();

        CHECK(SAGE::GraphicsResourceManager::ValidateNoLeaks());
        CHECK(SAGE::GraphicsResourceManager::TotalTrackedHandleCount() == 0u);
    }

    ResetGraphicsResourceManager();
}
