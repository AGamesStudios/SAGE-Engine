#include "SAGE.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/Rendering/Effects/Particles/ParticleSystem.h"
#include <GLFW/glfw3.h>

using namespace SAGE;

// Particle System Demo - Backend-Agnostic Design
// Works with OpenGL, Vulkan, DirectX12, or any IRenderBackend implementation

int main() {
    Logger::Init();
    
    // Create window
    WindowProps props;
    props.Title = "SAGE Engine - Particle System Demo";
    props.Width = 1280;
    props.Height = 720;
    Window window(props);
    
    // Initialize renderer (defaults to OpenGL)
    Graphics::RenderSystemConfig cfg;
    cfg.backendType = Graphics::BackendType::OpenGL;
    Renderer::Init(cfg);
    
    // Get backend pointer (works with ANY IRenderBackend)
    IRenderBackend* backend = Renderer::GetRenderBackend();
    
    // Example 1: Fire Effect
    ParticleEmitterConfig fireConfig;
    fireConfig.maxParticles = 200;
    fireConfig.emissionRate = 50.0f;
    fireConfig.particleLifetimeMin = 0.5f;
    fireConfig.particleLifetimeMax = 1.2f;
    fireConfig.spawnRadius = 10.0f;
    fireConfig.velocityDir = Vector2(0, 1); // Upward
    fireConfig.velocitySpeed = 100.0f;
    fireConfig.velocitySpeedVariance = 50.0f;
    fireConfig.colorStart = Color(1, 0.8f, 0, 1);  // Yellow-orange
    fireConfig.colorEnd = Color(1, 0, 0, 0);       // Transparent red
    fireConfig.sizeStart = 20.0f;
    fireConfig.sizeEnd = 5.0f;
    fireConfig.blendMode = ParticleBlendMode::Additive;
    
    ParticleEmitter fire(fireConfig);
    fire.SetPosition(Vector2(320, 100));
    fire.AddAffector(std::make_unique<GravityAffector>(Vector2(0, 50.0f))); // Upward gravity
    
    // Example 2: Vortex Effect
    ParticleEmitterConfig vortexConfig;
    vortexConfig.maxParticles = 500;
    vortexConfig.emissionRate = 100.0f;
    vortexConfig.particleLifetimeMin = 2.0f;
    vortexConfig.particleLifetimeMax = 4.0f;
    vortexConfig.velocitySpeed = 150.0f;
    vortexConfig.velocitySpeedVariance = 50.0f;
    vortexConfig.colorStart = Color(0, 0.5f, 1, 1);  // Cyan
    vortexConfig.colorEnd = Color(1, 1, 1, 0);        // White fade
    vortexConfig.sizeStart = 8.0f;
    vortexConfig.sizeEnd = 2.0f;
    vortexConfig.blendMode = ParticleBlendMode::Additive;
    
    ParticleEmitter vortex(vortexConfig);
    vortex.SetPosition(Vector2(640, 360));
    Vector2 vortexCenter(640, 360);
    vortex.AddAffector(std::make_unique<VortexAffector>(vortexCenter, 200.0f, 0.5f));
    
    // Example 3: Explosion (burst mode)
    ParticleEmitterConfig explosionConfig;
    explosionConfig.maxParticles = 100;
    explosionConfig.emissionRate = 0.0f;  // No continuous emission
    explosionConfig.burstCount = 100;     // Spawn all at once
    explosionConfig.burstInterval = 0.0f;
    explosionConfig.loop = false;         // One-time effect
    explosionConfig.particleLifetimeMin = 0.3f;
    explosionConfig.particleLifetimeMax = 1.0f;
    explosionConfig.velocitySpeed = 300.0f;
    explosionConfig.velocitySpeedVariance = 150.0f;
    explosionConfig.colorStart = Color(1, 1, 0, 1);    // Yellow
    explosionConfig.colorEnd = Color(1, 0.5f, 0, 0);   // Orange fade
    explosionConfig.sizeStart = 15.0f;
    explosionConfig.sizeEnd = 3.0f;
    explosionConfig.blendMode = ParticleBlendMode::Additive;
    
    ParticleEmitter explosion(explosionConfig);
    explosion.SetPosition(Vector2(960, 360));
    explosion.AddAffector(std::make_unique<GravityAffector>(Vector2(0, -200.0f))); // Downward gravity
    
    // Main loop
    double lastTime = glfwGetTime();
    bool explosionTriggered = false;
    
    while (!window.ShouldClose()) {
        window.PollEvents();
        
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        // Trigger explosion after 2 seconds
        if (!explosionTriggered && currentTime > 2.0) {
            explosion.Emit();
            explosionTriggered = true;
            SAGE_INFO("Explosion triggered!");
        }
        
        // Update particle systems
        fire.Update(deltaTime);
        vortex.Update(deltaTime);
        explosion.Update(deltaTime);
        
        // Begin frame
        Renderer::BeginScene();
        backend->Clear(0.05f, 0.05f, 0.1f, 1.0f);
        
        // Render particles (backend-agnostic!)
        fire.Render(backend);
        vortex.Render(backend);
        explosion.Render(backend);
        
        // Display statistics
        if (static_cast<int>(currentTime) % 2 == 0) { // Every 2 seconds
            auto fireStats = fire.GetStats();
            auto vortexStats = vortex.GetStats();
            auto explosionStats = explosion.GetStats();
            
            SAGE_INFO("Fire: {}/{} particles", fireStats.activeParticles, fireStats.maxParticles);
            SAGE_INFO("Vortex: {}/{} particles", vortexStats.activeParticles, vortexStats.maxParticles);
            SAGE_INFO("Explosion: {}/{} particles", explosionStats.activeParticles, explosionStats.maxParticles);
        }
        
        // End frame
        Renderer::EndScene();
        window.SwapBuffers();
    }
    
    Renderer::Shutdown();
    Logger::Shutdown();
    
    return 0;
}

/*
 * KEY DESIGN POINTS:
 * 
 * 1. Backend-Agnostic Rendering:
 *    - fire.Render(backend) works with OpenGL, Vulkan, DX12, etc.
 *    - No hardcoded graphics API calls in particle code
 *    - IRenderBackend interface ensures extensibility
 * 
 * 2. Affector Pattern:
 *    - GravityAffector: constant force (fire, explosion)
 *    - VortexAffector: spinning motion (vortex effect)
 *    - AttractorAffector: pull toward point (not shown, but available)
 *    - Easy to add custom affectors (WindAffector, TurbulenceAffector, etc.)
 * 
 * 3. Flexible Configuration:
 *    - 40+ parameters per emitter
 *    - Burst vs continuous emission
 *    - Color/size interpolation
 *    - Blend modes (Alpha, Additive, Multiply)
 * 
 * 4. Real-Time Statistics:
 *    - GetStats() provides active particle count
 *    - Useful for performance monitoring
 * 
 * 5. Future Extensibility:
 *    - Switch to Vulkan: Just change cfg.backendType = BackendType::Vulkan
 *    - Particles continue working without code changes!
 */
