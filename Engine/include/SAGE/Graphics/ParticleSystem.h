#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Graphics/Texture.h"
#include <memory>
#include <vector>
#include <functional>

namespace SAGE {

// ============================================
// Simple Particle System for 2D Effects
// ============================================

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Color color;
    float lifetime = 1.0f;      // Total lifetime
    float age = 0.0f;           // Current age
    float size = 10.0f;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;
    float fadeOut = 1.0f;
    bool active = false;
};

using ParticleUpdateFunc = std::function<void(Particle&, float)>;

class ParticleSystem {
public:
    struct EmitterConfig {
        Vector2 position = {0.0f, 0.0f};
        Vector2 emissionArea = {10.0f, 10.0f};  // Random spawn area
        
        // Particle properties (ranges)
        Vector2 velocityMin = {-50.0f, -50.0f};
        Vector2 velocityMax = {50.0f, 50.0f};
        Vector2 acceleration = {0.0f, 0.0f};  // Gravity/Global acceleration
        
        float radialAcceleration = 0.0f;
        float tangentialAcceleration = 0.0f;

        Color startColor = Color::White();
        Color endColor = Color::White();
        
        float lifetimeMin = 1.0f;
        float lifetimeMax = 2.0f;
        
        float sizeStart = 10.0f;
        float sizeEnd = 0.0f;
        float sizeVariation = 0.0f; // Random variation added to start size
        
        float rotationStart = 0.0f;
        float rotationEnd = 0.0f;
        float rotationVariation = 0.0f;

        float emissionRate = 10.0f;  // Particles per second
        int maxParticles = 1000;
        
        bool autoEmit = true;
    };

    explicit ParticleSystem(size_t maxParticles = 1000);

    // Configuration
    void SetEmitterConfig(const EmitterConfig& config);
    const EmitterConfig& GetEmitterConfig() const { return m_Config; }
    
    void SetTexture(std::shared_ptr<Texture> texture) { m_Texture = texture; }
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }

    // Control
    void Start();
    void Stop();
    void Pause();
    void Resume();
    void Clear();

    // Manual emission
    void Emit(int count = 1);
    void Burst(int count);

    // Update
    void Update(float deltaTime);

    // Queries
    bool IsActive() const { return m_Active; }
    bool IsPaused() const { return m_Paused; }
    
    const std::vector<Particle>& GetParticles() const { return m_Particles; }
    size_t GetActiveCount() const { return m_ActiveCount; }

    void SetCustomUpdate(ParticleUpdateFunc func) { m_CustomUpdate = func; }

private:
    void EmitParticle();
    void UpdateParticle(Particle& particle, float deltaTime);

    std::vector<Particle> m_Particles;
    size_t m_ActiveCount = 0; // Optimization: Keep active particles at the front
    
    EmitterConfig m_Config;
    std::shared_ptr<Texture> m_Texture;
    
    float m_EmissionTimer = 0.0f;
    bool m_Active = false;
    bool m_Paused = false;
    
    ParticleUpdateFunc m_CustomUpdate;
};

// Particle renderer helper
class ParticleRenderer {
public:
    static void Render(const ParticleSystem& system);
};

} // namespace SAGE
