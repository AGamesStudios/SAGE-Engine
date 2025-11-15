#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>

namespace SAGE {

// Explicit template specialization for Texture loading
template<>
Ref<Texture> ResourceManager::LoadResource<Texture>(const std::string& path) {
    if (!IsGpuLoadingEnabled()) {
        SAGE_WARNING("ResourceManager: GPU loading disabled, returning stub for '{}'", path);
        return GetStub<Texture>();
    }

    // Headless fallback: if no GL context, return stub.
    if (glfwGetCurrentContext() == nullptr) {
        SAGE_WARNING("ResourceManager: No active GL context; returning stub for '{}'", path);
        return GetStub<Texture>();
    }

    try {
        auto texture = CreateRef<Texture>(path);
        if (!texture || !texture->IsLoaded()) {
            SAGE_ERROR("ResourceManager: Failed to load texture from '{}'", path);
            return nullptr;
        }
        return texture;
    }
    catch (const std::exception& e) {
        SAGE_ERROR("ResourceManager: Exception loading texture '{}': {}", path, e.what());
        return nullptr;
    }
}

} // namespace SAGE
