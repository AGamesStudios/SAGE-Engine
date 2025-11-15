#pragma once

#include <SAGE/IEngine.h>

namespace SAGE {
namespace Internal {

// Bridges public IRenderer interface to the internal Graphics::Renderer static API.
class RendererAdapter : public IRenderer {
public:
    RendererAdapter();
    ~RendererAdapter() override;

    // IRenderer interface
    void Clear(const Color& color) override;
    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;

    void Initialize();
    void Shutdown();

private:
    bool m_Initialized = false;
};

} // namespace Internal
} // namespace SAGE
