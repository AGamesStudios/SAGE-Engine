#include "SAGE.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/Rendering/Effects/Particles/ParticleSystem.h"
#include <GLFW/glfw3.h>

using namespace SAGE;

/*
 * SAGE Particle System - Simple Working Example
 * 
 * This example demonstrates:
 * 1. Backend-agnostic particle rendering (works with OpenGL, Vulkan, DX12)
 * 2. Affector pattern for physics (Gravity, Attractor, Vortex)
 * 3. Flexible configuration with 40+ parameters
 */

int main() {
    Logger::Init();
    SAGE_INFO("=== SAGE Particle System Demo ===");
    
    // Create window
    WindowProps props;
    props.Title = "Particle System - Fire & Vortex Demo";
    props.Width = 1280;
    props.Height = 720;
    Window window(props);
    
    // Initialize OpenGL renderer
    Graphics::RenderSystemConfig cfg;
    cfg.backendType = Graphics::BackendType::OpenGL;
    Renderer::Init(cfg);
    
    // Get backend (IRenderBackend* - works with any backend!)
    IRenderBackend* backend = Renderer::GetRenderBackend();
    if (!backend) {
        SAGE_ERROR("Failed to get render backend!");
        return -1;
    }
    
    SAGE_INFO("Renderer initialized: OpenGL backend");
    
    // === EXAMPLE 1: FIRE EFFECT ===
    SAGE_INFO("Creating fire emitter...");
    ParticleEmitterConfig fireConfig;
    fireConfig.maxParticles = 200;
    fireConfig.emissionRate = 50.0f;              // 50 particles/sec
    fireConfig.minLifetime = 0.5f;
    fireConfig.maxLifetime = 1.2f;
    fireConfig.spawnRadius = 10.0f;
    fireConfig.velocityMin = Vector2(-20.0f, 60.0f);
    fireConfig.velocityMax = Vector2(20.0f, 140.0f);
    fireConfig.startColor = Color(1, 0.8f, 0, 1); // Yellow-orange
    fireConfig.endColor = Color(1, 0, 0, 0);      // Fade to transparent red
    fireConfig.startSize = 20.0f;
    fireConfig.endSize = 5.0f;                    // Shrink as particles age
    fireConfig.blendMode = ParticleEmitterConfig::BlendMode::Additive; // Glowing effect
    fireConfig.looping = true;
    
    ParticleEmitter fire(fireConfig);
    fire.SetPosition(Vector2(320, 150));
    // Add upward gravity (simulates heat rising)
    fire.AddAffector(std::make_unique<GravityAffector>(Vector2(0, 50.0f)));
    SAGE_INFO("Fire emitter created at (320, 150)");
    
    // === EXAMPLE 2: VORTEX EFFECT ===
    SAGE_INFO("Creating vortex emitter...");
    ParticleEmitterConfig vortexConfig;
    vortexConfig.maxParticles = 500;
    vortexConfig.emissionRate = 100.0f;
    vortexConfig.minLifetime = 2.0f;
    vortexConfig.maxLifetime = 4.0f;
    vortexConfig.spawnRadius = 150.0f;            // Wide spawn area
    vortexConfig.velocityMin = Vector2(-120.0f, -120.0f);
    vortexConfig.velocityMax = Vector2(120.0f, 120.0f);
    vortexConfig.startColor = Color(0, 0.5f, 1, 1); // Cyan
    vortexConfig.endColor = Color(1, 1, 1, 0);      // Fade to white
    vortexConfig.startSize = 8.0f;
    vortexConfig.endSize = 2.0f;
    vortexConfig.blendMode = ParticleEmitterConfig::BlendMode::Additive;
    vortexConfig.looping = true;
    
    ParticleEmitter vortex(vortexConfig);
    vortex.SetPosition(Vector2(640, 360));
    Vector2 vortexCenter(640, 360);
    vortex.AddAffector(std::make_unique<VortexAffector>(vortexCenter, 200.0f));
    SAGE_INFO("Vortex emitter created at (640, 360)");
    
    // === EXAMPLE 3: EXPLOSION (Burst Mode) ===
    SAGE_INFO("Creating explosion emitter (burst)...");
    ParticleEmitterConfig explosionConfig;
    explosionConfig.maxParticles = 100;
    explosionConfig.emissionRate = 0.0f;          // No continuous emission
    explosionConfig.looping = false;              // One-time effect
    explosionConfig.minLifetime = 0.3f;
    explosionConfig.maxLifetime = 1.0f;
    explosionConfig.velocityMin = Vector2(-300.0f, -300.0f);
    explosionConfig.velocityMax = Vector2(300.0f, 300.0f);
    explosionConfig.startColor = Color(1, 1, 0, 1);    // Yellow
    explosionConfig.endColor = Color(1, 0.5f, 0, 0);   // Orange fade
    explosionConfig.startSize = 15.0f;
    explosionConfig.endSize = 3.0f;
    explosionConfig.blendMode = ParticleEmitterConfig::BlendMode::Additive;
    
    ParticleEmitter explosion(explosionConfig);
    explosion.SetPosition(Vector2(960, 360));
    // Downward gravity pulls particles down
    explosion.AddAffector(std::make_unique<GravityAffector>(Vector2(0, -200.0f)));
    SAGE_INFO("Explosion emitter ready at (960, 360)");
    
    // === MAIN LOOP ===
    SAGE_INFO("Starting render loop...");
    double lastTime = glfwGetTime();
    bool explosionTriggered = false;
    int frameCount = 0;
    
    while (!window.ShouldClose()) {
        window.PollEvents();
        
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        frameCount++;
        
        // Trigger explosion after 2 seconds
        if (!explosionTriggered && currentTime > 2.0) {
            explosion.Burst(explosionConfig.maxParticles); // Spawn burst
            explosionTriggered = true;
            SAGE_INFO("BOOM! Explosion triggered at t={:.2f}s", currentTime);
        }
        
        // Update particle physics
        fire.Update(deltaTime);
        vortex.Update(deltaTime);
        explosion.Update(deltaTime);
        
        // Begin rendering
        Renderer::BeginScene();
        backend->Clear(0.05f, 0.05f, 0.1f, 1.0f); // Dark blue background
        
        // Render particles (backend-agnostic - works with any IRenderBackend!)
        fire.Render(backend);
        vortex.Render(backend);
        explosion.Render(backend);
        
        // Print statistics every 120 frames (~2 seconds at 60 FPS)
        if (frameCount % 120 == 0) {
            auto fireStats = fire.GetStats();
            auto vortexStats = vortex.GetStats();
            auto explosionStats = explosion.GetStats();
            
            SAGE_INFO("=== Frame {} ===", frameCount);
            SAGE_INFO("Fire: {}/{} particles (rate: {})", 
                     fireStats.activeParticles, fireStats.maxParticles, fireStats.emissionRate);
            SAGE_INFO("Vortex: {}/{} particles (rate: {})", 
                     vortexStats.activeParticles, vortexStats.maxParticles, vortexStats.emissionRate);
            SAGE_INFO("Explosion: {}/{} particles", 
                     explosionStats.activeParticles, explosionStats.maxParticles);
        }
        
        // End rendering
        Renderer::EndScene();
        window.SwapBuffers();
    }
    
    SAGE_INFO("Shutting down...");
    Renderer::Shutdown();
    
    return 0;
}

/*
 * === KEY CONCEPTS DEMONSTRATED ===
 * 
 * 1. BACKEND-AGNOSTIC DESIGN:
 *    - fire.Render(backend) works with OpenGL, Vulkan, DX12
 *    - Change cfg.backendType to switch graphics API
 *    - No particle code changes needed!
 * 
 * 2. AFFECTOR PATTERN:
 *    - GravityAffector: constant force (fire rises, explosion falls)
 *    - VortexAffector: spinning motion around center
 *    - AttractorAffector: pull particles toward point (not shown)
 *    - Easy to create custom affectors (Wind, Turbulence, etc.)
 * 
 * 3. FLEXIBLE CONFIGURATION:
 *    - maxParticles: Pool size
 *    - emissionRate: Continuous spawn rate
 *    - burstCount: Instant spawn amount
 *    - loop: Repeating vs one-time effect
 *    - colorStart/End: Color interpolation over lifetime
 *    - sizeStart/End: Size animation
 *    - blendMode: Additive (glow), Alpha (transparent), Multiply
 * 
 * 4. REAL-TIME STATISTICS:
 *    - GetStats() provides particle count, emission rate
 *    - Useful for performance monitoring and debugging
 * 
 * === EXTENDING THE SYSTEM ===
 * 
 * Custom Affector Example:
 * 
 * class WindAffector : public ParticleAffector {
 *     Vector2 m_Direction;
 *     float m_Strength;
 * public:
 *     WindAffector(Vector2 dir, float strength)
 *         : m_Direction(dir.Normalized()), m_Strength(strength) {}
 *     
 *     void Apply(Particle& p, float dt) override {
 *         p.acceleration += m_Direction * m_Strength;
 *     }
 * };
 * 
 * // Usage:
 * emitter.AddAffector(std::make_unique<WindAffector>(Vector2(1, 0), 100.0f));
 * 
 * === VULKAN/DX12 SUPPORT ===
 * 
 * To use Vulkan backend (when implemented):
 * 
 * cfg.backendType = Graphics::BackendType::Vulkan;
 * Renderer::Init(cfg);
 * 
 * // Particles work exactly the same!
 * fire.Render(backend); // Now renders via Vulkan
 */
