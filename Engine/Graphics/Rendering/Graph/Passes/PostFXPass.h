#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"

namespace SAGE {

class PostFXPass : public IRenderPass {
public:
    explicit PostFXPass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    const char* GetName() const override { return "PostFXPass"; }
    void Initialize(IRenderBackend* backend) override { m_Backend = backend; m_Initialized = true; }
    void Shutdown() override { m_Backend = nullptr; m_Initialized = false; }
    bool IsInitialized() const override { return m_Initialized; }

    bool Execute(const FrameContext& ctx) override {
        if (!m_Scene) return true; // no-op if missing
        // For now, rely on scene's internal post-fx flag triggered during EndScene.
        (void)ctx; return true;
    }
private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    IRenderBackend* m_Backend = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
