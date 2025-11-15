#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/API/Renderer.h" // for fallback color maybe

namespace SAGE {

class ClearPass : public IRenderPass {
public:
    explicit ClearPass(float r=0.05f,float g=0.05f,float b=0.08f,float a=1.0f)
        : m_R(r), m_G(g), m_B(b), m_A(a) {}

    const char* GetName() const override { return "ClearPass"; }
    void Initialize(IRenderBackend* backend) override { m_Backend = backend; m_Initialized = true; }
    void Shutdown() override { m_Backend = nullptr; m_Initialized = false; }
    bool IsInitialized() const override { return m_Initialized; }

    bool Execute(const FrameContext& ctx) override {
        if (!m_Backend) {
            return false;
        }

        // Only clear once for the main world domain; UI/PostFX passes should preserve
        // whatever the world domain rendered.
        if (ctx.pass.domain != RenderDomain::World) {
            return true;
        }

        m_Backend->Clear(m_R, m_G, m_B, m_A);
        return true;
    }
private:
    IRenderBackend* m_Backend = nullptr;
    bool m_Initialized = false;
    float m_R, m_G, m_B, m_A;
};

} // namespace SAGE
