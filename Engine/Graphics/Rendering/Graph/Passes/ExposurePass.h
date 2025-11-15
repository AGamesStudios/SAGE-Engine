#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"

namespace SAGE {
namespace Graphics {
    class OpenGLSceneRenderer; // Forward declaration
}

/// ExposurePass: Applies gamma, exposure, and pulse time effects, renders final fullscreen quad to backbuffer
class ExposurePass : public IRenderPass {
public:
    explicit ExposurePass(Graphics::ISceneRenderer* scene) : m_Scene(scene) {}
    
    const char* GetName() const override { return "ExposurePass"; }
    
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
        
        // Temporarily delegate to existing ApplyPostFX for complete PostFX pipeline
        auto* glScene = dynamic_cast<Graphics::OpenGLSceneRenderer*>(m_Scene);
        if (glScene) {
            glScene->ApplyPostFX();
        }
        
        // TODO: Direct implementation without ApplyPostFX:
        // 1. Bind default framebuffer (screen)
        // 2. Sample composite texture from CompositePass
        // 3. Apply gamma correction
        // 4. Apply exposure adjustment
        // 5. Apply pulse time effect if enabled
        // 6. Draw fullscreen quad to backbuffer
        
        return true;
    }

private:
    Graphics::ISceneRenderer* m_Scene = nullptr; // non-owning
    IRenderBackend* m_Backend = nullptr; // non-owning
    bool m_Initialized = false;
};

} // namespace SAGE
