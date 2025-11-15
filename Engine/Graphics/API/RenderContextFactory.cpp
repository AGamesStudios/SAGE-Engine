#include "Graphics/API/RenderContext.hpp"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/API/RenderSystemRegistry.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLRenderBackend.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Core/Logger.h"

#include <memory>

namespace SAGE::Graphics {

// Create and initialize a core RenderContext along with backend and register subsystems.
std::unique_ptr<RenderContext> CreateDefaultRenderContext(const RenderSystemConfig& config,
                                                          RenderSystemRegistry& registry) {
    // Backend must already be created via registry.CreateBackend in Renderer::Init.
    auto backend = registry.GetActiveBackend();
    if (!backend) {
        SAGE_WARNING("CreateDefaultRenderContext: backend not active; creating shared OpenGL fallback backend");
        auto fallbackOwned = std::make_shared<OpenGLRenderBackend>();
        fallbackOwned->Init();
        fallbackOwned->Configure(config);
        registry.SetActiveBackendShared(fallbackOwned);
        backend = fallbackOwned.get();
    }

    auto context = std::make_unique<RenderContext>();

    // Renderer will finish initialization (calling Init) after creation.
    return context;
}

} // namespace SAGE::Graphics
