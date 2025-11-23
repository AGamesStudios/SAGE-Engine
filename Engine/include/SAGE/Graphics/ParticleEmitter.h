#pragma once

#include "SAGE/Graphics/ParticleSystem.h"
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include <functional>
#include <memory>

namespace SAGE {

/// Shape types for particle emitters
enum class EmitterShape {
    Point,      // Emit from a single point
    Circle,     // Emit from circle edge
    Box,        // Emit from box area
    Cone        // Emit in a cone direction
};

/// Configuration for particle emitter
struct ParticleEmitterConfig {
    // Shape
    EmitterShape shape = EmitterShape::Point;
    Vector2 position{0.0f, 0.0f};
    
    // Shape-specific parameters
    float radius = 10.0f;           // For Circle
    Vector2 boxSize{10.0f, 10.0f};  // For Box
    float coneAngle = 45.0f;        // For Cone (degrees)
    Vector2 direction{0.0f, -1.0f}; // For Cone
    
    // Emission
    float emissionRate = 10.0f;     // Particles per second
    int burstCount = 0;             // 0 = continuous emission
    float burstInterval = 1.0f;     // Time between bursts
    
    // Particle properties
    float lifetimeMin = 1.0f;
    float lifetimeMax = 2.0f;
    
    Vector2 velocityMin{-50.0f, -50.0f};
    Vector2 velocityMax{50.0f, 50.0f};
    
    Vector2 acceleration{0.0f, 100.0f}; // Gravity
    
    float sizeMin = 5.0f;
    float sizeMax = 10.0f;
    float sizeOverLifetime = 1.0f; // Multiplier
    
    Color startColor{1.0f, 1.0f, 1.0f, 1.0f};
    Color endColor{1.0f, 1.0f, 1.0f, 0.0f};
    
    float rotationMin = 0.0f;
    float rotationMax = 6.28318f; // 2π
    float angularVelocityMin = -2.0f;
    float angularVelocityMax = 2.0f;
    
    // Behavior
    bool autoEmit = true;
    bool loop = true;
    float duration = 5.0f; // Total duration if not looping
};

/// Advanced particle emitter with various shapes and behaviors
class ParticleEmitter {
public:
    explicit ParticleEmitter(size_t maxParticles = 1000);
    ~ParticleEmitter() = default;

    void SetConfig(const ParticleEmitterConfig& config);
    const ParticleEmitterConfig& GetConfig() const { return m_Config; }

    void Start();
    void Stop();
    void Pause();
    void Resume();
    void Burst(int count);
    
    void Update(float deltaTime);
    
    bool IsActive() const { return m_Active; }
    bool IsPaused() const { return m_Paused; }
    
    void SetTexture(std::shared_ptr<Texture> texture) { m_Texture = texture; }
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }
    
    const std::vector<Particle>& GetParticles() const { return m_Particles; }
    size_t GetActiveParticleCount() const;
    
    // Rendering
    void Render() const;
    
    // Custom update callback
    using ParticleUpdateCallback = std::function<void(Particle&, float)>;
    void SetCustomUpdate(ParticleUpdateCallback callback) { m_CustomUpdate = callback; }

    // Preset emitter configurations
    static ParticleEmitterConfig CreateFireEmitter();
    static ParticleEmitterConfig CreateSmokeEmitter();
    static ParticleEmitterConfig CreateExplosionEmitter();
    static ParticleEmitterConfig CreateRainEmitter();
    static ParticleEmitterConfig CreateSnowEmitter();

private:
    void EmitParticle();
    void UpdateParticle(Particle& particle, float deltaTime);
    Vector2 GetEmissionPosition() const;
    Vector2 GetEmissionVelocity() const;
    float RandomRange(float min, float max) const;
    Color LerpColor(const Color& a, const Color& b, float t) const;

    ParticleEmitterConfig m_Config;
    std::vector<Particle> m_Particles;
    std::shared_ptr<Texture> m_Texture;
    
    bool m_Active = false;
    bool m_Paused = false;
    
    float m_EmissionTimer = 0.0f;
    float m_BurstTimer = 0.0f;
    float m_DurationTimer = 0.0f;
    
    ParticleUpdateCallback m_CustomUpdate;
};

} // namespace SAGE
