#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"

namespace SAGE {

/// BlurPass: Applies blur effect to scene texture
/// Initial implementation is a stub; future iterations can implement ping-pong multi-pass blur
class BlurPass : public IRenderPass {
public:
    explicit BlurPass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    
    const char* GetName() const override { return "BlurPass"; }
    
    void Initialize(IRenderBackend* backend) override { 
        m_Backend = backend; 
        m_Initialized = true; 
    }
    
    void Shutdown() override { 
        m_Backend = nullptr; 
        m_Initialized = false; 
    }
    
    bool IsInitialized() const override { return m_Initialized; }

    bool Execute(const FrameContext& ctx) override {
        // Only process during PostFX domain
        if (ctx.pass.domain != RenderDomain::PostFX) return true;
        
        if (!m_Scene) return true;
        
        // TODO: Implement blur logic
        // For now, this is a stub that does nothing
        // Future implementation:
        // 1. Bind blur FBO
        // 2. Apply horizontal blur pass
        // 3. Bind second FBO
        // 4. Apply vertical blur pass (ping-pong)
        // 5. Store result for CompositePass
        
        return true;
    }

private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    IRenderBackend* m_Backend = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
