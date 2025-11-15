#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"
#include <memory>

namespace SAGE {

// Bridges existing scene renderer batching as a pass
class GeometryPass : public IRenderPass {
public:
    explicit GeometryPass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    const char* GetName() const override { return "GeometryPass"; }
    void Initialize(IRenderBackend* /*backend*/) override { m_Initialized = true; }
    void Shutdown() override { m_Initialized = false; }
    bool IsInitialized() const override { return m_Initialized; }

    bool Execute(const FrameContext& ctx) override {
        if (!m_Scene) return false;
        // Scene renderer already began; we ensure EndScene occurs externally.
        // Here we could later inject camera, culling, etc.
        (void)ctx; // currently unused
        return true;
    }
private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
