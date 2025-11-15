#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"

namespace SAGE {

/// CompositePass: Combines scene color with optional blur texture, applies tint and intensity
class CompositePass : public IRenderPass {
public:
    explicit CompositePass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    
    const char* GetName() const override { return "CompositePass"; }
    
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
        
        // TODO: Implement composite logic
        // For now, this is a stub
        // Future implementation:
        // 1. Bind intermediate FBO
        // 2. Sample scene texture
        // 3. Optionally blend with blur output
        // 4. Apply tint color and intensity
        // 5. Write to intermediate buffer for ExposurePass
        
        return true;
    }

private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    IRenderBackend* m_Backend = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
