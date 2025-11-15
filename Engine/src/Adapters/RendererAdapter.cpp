#include "RendererAdapter.h"

#include <Graphics/API/Renderer.h>
#include <Graphics/API/RenderSystemConfig.h>
#include <Core/Logger.h>

namespace SAGE {
namespace Internal {

RendererAdapter::RendererAdapter() {
    Initialize();
}

RendererAdapter::~RendererAdapter() {
    Shutdown();
}

void RendererAdapter::Initialize() {
    if (m_Initialized) {
        return;
    }

    if (!Graphics::Renderer::IsInitialized()) {
        Graphics::Renderer::Init();
        SAGE_INFO("RendererAdapter initialized Graphics::Renderer");
    }

    m_Initialized = true;
}

void RendererAdapter::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    if (Graphics::Renderer::IsInitialized()) {
        Graphics::Renderer::Shutdown();
    }

    m_Initialized = false;
}

void RendererAdapter::Clear(const Color& color) {
    Graphics::Renderer::Clear(color.r, color.g, color.b, color.a);
}

void RendererAdapter::BeginFrame() {
    Graphics::Renderer::BeginScene();
}

void RendererAdapter::EndFrame() {
    Graphics::Renderer::EndScene();
}

void RendererAdapter::Present() {
    // The Renderer handles presentation implicitly inside EndScene().
    // Method retained for interface completeness.
}

} // namespace Internal
} // namespace SAGE
