#include "GraphicsResourceManager.h"

#include "Core/Logger.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <glad/glad.h>

namespace SAGE {

namespace {

struct ResourceStats {
    bool initialized = false;
    std::size_t textures = 0;
    std::size_t buffers = 0;
    std::size_t vertexArrays = 0;
    std::size_t framebuffers = 0;
    std::size_t renderbuffers = 0;
    std::size_t shaderPrograms = 0;
    std::vector<std::weak_ptr<Texture>> trackedTextures;
    std::vector<std::weak_ptr<Shader>> trackedShaders;
    std::vector<std::weak_ptr<Font>> trackedFonts;
};

ResourceStats& Stats() {
    static ResourceStats stats{};
    return stats;
}

void CompactTrackedTextures(ResourceStats& stats) {
    std::erase_if(stats.trackedTextures,
                  [](const std::weak_ptr<Texture>& entry) { return entry.expired(); });
}

void CompactTrackedShaders(ResourceStats& stats) {
    std::erase_if(stats.trackedShaders,
                  [](const std::weak_ptr<Shader>& entry) { return entry.expired(); });
}

void CompactTrackedFonts(ResourceStats& stats) {
    std::erase_if(stats.trackedFonts,
                  [](const std::weak_ptr<Font>& entry) { return entry.expired(); });
}

} // namespace

void GraphicsResourceManager::EnsureInitialized() {
    auto& stats = Stats();
    if (!stats.initialized) {
        Init();
    }
}

void GraphicsResourceManager::Init() {
    auto& stats = Stats();
    if (stats.initialized) {
        if (!ValidateNoLeaks()) {
            SAGE_WARNING("GraphicsResourceManager::Init called while resources are still tracked");
        }
    }

    stats.textures = 0;
    stats.buffers = 0;
    stats.vertexArrays = 0;
    stats.framebuffers = 0;
    stats.renderbuffers = 0;
    stats.shaderPrograms = 0;
    stats.trackedTextures.clear();
    stats.trackedShaders.clear();
    stats.trackedFonts.clear();
    stats.initialized = true;
}

void GraphicsResourceManager::Shutdown() {
    auto& stats = Stats();
    if (!stats.initialized) {
        return;
    }

    if (!ValidateNoLeaks()) {
        SAGE_WARNING("GraphicsResourceManager::Shutdown detected active graphics resources");
    }

    stats.trackedTextures.clear();
    stats.trackedShaders.clear();
    stats.trackedFonts.clear();
    stats.initialized = false;
}

void GraphicsResourceManager::TrackTexture(const Ref<Texture>& texture) {
    EnsureInitialized();
    if (!texture) {
        return;
    }

    auto& stats = Stats();
    CompactTrackedTextures(stats);
    stats.trackedTextures.emplace_back(texture);
}

void GraphicsResourceManager::TrackShader(const Ref<Shader>& shader) {
    EnsureInitialized();
    if (!shader) {
        return;
    }

    auto& stats = Stats();
    CompactTrackedShaders(stats);
    stats.trackedShaders.emplace_back(shader);
}

void GraphicsResourceManager::TrackFont(const Ref<Font>& font) {
    EnsureInitialized();
    if (!font) {
        return;
    }

    auto& stats = Stats();
    CompactTrackedFonts(stats);
    stats.trackedFonts.emplace_back(font);
}

std::size_t GraphicsResourceManager::ActiveTextureCount() {
    return Stats().textures;
}

std::size_t GraphicsResourceManager::ActiveBufferCount() {
    return Stats().buffers;
}

std::size_t GraphicsResourceManager::ActiveVertexArrayCount() {
    return Stats().vertexArrays;
}

std::size_t GraphicsResourceManager::ActiveFramebufferCount() {
    return Stats().framebuffers;
}

std::size_t GraphicsResourceManager::ActiveRenderbufferCount() {
    return Stats().renderbuffers;
}

std::size_t GraphicsResourceManager::ActiveShaderProgramCount() {
    return Stats().shaderPrograms;
}

std::size_t GraphicsResourceManager::TotalTrackedHandleCount() {
    const auto& stats = Stats();
    return stats.textures + stats.buffers + stats.vertexArrays + stats.framebuffers +
           stats.renderbuffers + stats.shaderPrograms;
}

bool GraphicsResourceManager::ValidateNoLeaks() {
    auto& stats = Stats();
    CompactTrackedTextures(stats);
    CompactTrackedShaders(stats);
    CompactTrackedFonts(stats);

    const bool handlesClean = stats.textures == 0 && stats.buffers == 0 &&
                              stats.vertexArrays == 0 && stats.framebuffers == 0 &&
                              stats.renderbuffers == 0 && stats.shaderPrograms == 0;

    const bool trackedTexturesReleased = std::none_of(
        stats.trackedTextures.begin(),
        stats.trackedTextures.end(),
        [](const std::weak_ptr<Texture>& entry) { return !entry.expired(); });

    const bool trackedShadersReleased = std::none_of(
        stats.trackedShaders.begin(),
        stats.trackedShaders.end(),
        [](const std::weak_ptr<Shader>& entry) { return !entry.expired(); });

    const bool trackedFontsReleased = std::none_of(
        stats.trackedFonts.begin(),
        stats.trackedFonts.end(),
        [](const std::weak_ptr<Font>& entry) { return !entry.expired(); });

    return handlesClean && trackedTexturesReleased && trackedShadersReleased && trackedFontsReleased;
}

void GraphicsResourceManager::Increment(ResourceKind kind) {
    EnsureInitialized();
    auto& stats = Stats();
    switch (kind) {
    case ResourceKind::Texture:
        ++stats.textures;
        break;
    case ResourceKind::Buffer:
        ++stats.buffers;
        break;
    case ResourceKind::VertexArray:
        ++stats.vertexArrays;
        break;
    case ResourceKind::Framebuffer:
        ++stats.framebuffers;
        break;
    case ResourceKind::Renderbuffer:
        ++stats.renderbuffers;
        break;
    case ResourceKind::ShaderProgram:
        ++stats.shaderPrograms;
        break;
    }
}

void GraphicsResourceManager::Decrement(ResourceKind kind) {
    auto& stats = Stats();
    switch (kind) {
    case ResourceKind::Texture:
        if (stats.textures > 0) {
            --stats.textures;
        }
        break;
    case ResourceKind::Buffer:
        if (stats.buffers > 0) {
            --stats.buffers;
        }
        break;
    case ResourceKind::VertexArray:
        if (stats.vertexArrays > 0) {
            --stats.vertexArrays;
        }
        break;
    case ResourceKind::Framebuffer:
        if (stats.framebuffers > 0) {
            --stats.framebuffers;
        }
        break;
    case ResourceKind::Renderbuffer:
        if (stats.renderbuffers > 0) {
            --stats.renderbuffers;
        }
        break;
    case ResourceKind::ShaderProgram:
        if (stats.shaderPrograms > 0) {
            --stats.shaderPrograms;
        }
        break;
    }
}

} // namespace SAGE


