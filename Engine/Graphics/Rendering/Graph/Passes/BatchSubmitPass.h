#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"

namespace SAGE {

class BatchSubmitPass : public IRenderPass {
public:
    explicit BatchSubmitPass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    const char* GetName() const override { return "BatchSubmitPass"; }
    void Initialize(IRenderBackend* /*backend*/) override { m_Initialized = true; }
    void Shutdown() override { m_Initialized = false; }
    bool IsInitialized() const override { return m_Initialized; }
    bool Execute(const FrameContext& ctx) override {
        if (!m_Initialized || !m_Scene) return false;
        // Only execute during World domain currently
        if (ctx.pass.domain != RenderDomain::World) return true; // skip silently
        return m_Scene->EndScene();
    }
private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
