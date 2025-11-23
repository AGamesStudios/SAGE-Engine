#include "SAGE/Graphics/ParticleEmitter.h"
#include "SAGE/Log.h"
#include <random>
#include <algorithm>
#include <cmath>

namespace SAGE {

namespace {
    std::random_device s_RandomDevice;
    std::mt19937 s_RandomEngine(s_RandomDevice());
}

ParticleEmitter::ParticleEmitter(size_t maxParticles) {
    m_Particles.resize(maxParticles);
}

void ParticleEmitter::SetConfig(const ParticleEmitterConfig& config) {
    m_Config = config;
    
    if (m_Config.emissionRate <= 0.0f) {
        m_Config.emissionRate = 1.0f;
        static bool warningShown = false;
        if (!warningShown) {
            SAGE_WARN("ParticleEmitter: emissionRate must be > 0, using 1.0");
            warningShown = true;
        }
    }
}

void ParticleEmitter::Start() {
    m_Active = true;
    m_Paused = false;
    m_EmissionTimer = 0.0f;
    m_BurstTimer = 0.0f;
    m_DurationTimer = 0.0f;
}

void ParticleEmitter::Stop() {
    m_Active = false;
    m_Paused = false;
    
    // Clear all particles
    for (auto& particle : m_Particles) {
        particle.active = false;
    }
}

void ParticleEmitter::Pause() {
    m_Paused = true;
}

void ParticleEmitter::Resume() {
    m_Paused = false;
}

void ParticleEmitter::Burst(int count) {
    for (int i = 0; i < count; ++i) {
        EmitParticle();
    }
}

void ParticleEmitter::Update(float deltaTime) {
    if (!m_Active || m_Paused) {
        return;
    }

    // Update duration timer
    if (!m_Config.loop) {
        m_DurationTimer += deltaTime;
        if (m_DurationTimer >= m_Config.duration) {
            Stop();
            return;
        }
    }

    // Handle emission
    if (m_Config.autoEmit) {
        if (m_Config.burstCount > 0) {
            // Burst mode
            m_BurstTimer += deltaTime;
            if (m_BurstTimer >= m_Config.burstInterval) {
                Burst(m_Config.burstCount);
                m_BurstTimer -= m_Config.burstInterval;
            }
        } else {
            // Continuous mode
            m_EmissionTimer += deltaTime;
            float emissionInterval = 1.0f / m_Config.emissionRate;
            
            while (m_EmissionTimer >= emissionInterval) {
                EmitParticle();
                m_EmissionTimer -= emissionInterval;
            }
        }
    }

    // Update active particles
    for (auto& particle : m_Particles) {
        if (particle.active) {
            UpdateParticle(particle, deltaTime);
        }
    }
}

size_t ParticleEmitter::GetActiveParticleCount() const {
    return std::count_if(m_Particles.begin(), m_Particles.end(),
        [](const Particle& p) { return p.active; });
}

void ParticleEmitter::EmitParticle() {
    // Find inactive particle
    auto it = std::find_if(m_Particles.begin(), m_Particles.end(),
        [](const Particle& p) { return !p.active; });
    
    if (it == m_Particles.end()) {
        return; // No free particles
    }

    Particle& p = *it;
    
    // Initialize particle
    p.position = GetEmissionPosition();
    p.velocity = GetEmissionVelocity();
    p.acceleration = m_Config.acceleration;
    p.color = m_Config.startColor;
    p.lifetime = RandomRange(m_Config.lifetimeMin, m_Config.lifetimeMax);
    p.age = 0.0f;
    p.size = RandomRange(m_Config.sizeMin, m_Config.sizeMax);
    p.rotation = RandomRange(m_Config.rotationMin, m_Config.rotationMax);
    p.angularVelocity = RandomRange(m_Config.angularVelocityMin, m_Config.angularVelocityMax);
    p.active = true;
}

void ParticleEmitter::UpdateParticle(Particle& particle, float deltaTime) {
    particle.age += deltaTime;
    
    // Kill old particles
    if (particle.age >= particle.lifetime) {
        particle.active = false;
        return;
    }

    // Physics
    particle.velocity = particle.velocity + particle.acceleration * deltaTime;
    particle.position = particle.position + particle.velocity * deltaTime;
    particle.rotation += particle.angularVelocity * deltaTime;

    // Life progress
    float t = particle.lifetime > 0.0f ? (particle.age / particle.lifetime) : 0.0f;
    
    // Color interpolation
    particle.color = LerpColor(m_Config.startColor, m_Config.endColor, t);
    
    // Size over lifetime
    particle.size *= m_Config.sizeOverLifetime;

    // Custom update
    if (m_CustomUpdate) {
        m_CustomUpdate(particle, deltaTime);
    }
}

Vector2 ParticleEmitter::GetEmissionPosition() const {
    switch (m_Config.shape) {
        case EmitterShape::Point:
            return m_Config.position;
            
        case EmitterShape::Circle: {
            float angle = RandomRange(0.0f, 6.28318f); // 0-2π
            float r = m_Config.radius;
            return {
                m_Config.position.x + std::cos(angle) * r,
                m_Config.position.y + std::sin(angle) * r
            };
        }
            
        case EmitterShape::Box: {
            return {
                m_Config.position.x + RandomRange(-m_Config.boxSize.x / 2, m_Config.boxSize.x / 2),
                m_Config.position.y + RandomRange(-m_Config.boxSize.y / 2, m_Config.boxSize.y / 2)
            };
        }
            
        case EmitterShape::Cone:
            return m_Config.position;
            
        default:
            return m_Config.position;
    }
}

Vector2 ParticleEmitter::GetEmissionVelocity() const {
    if (m_Config.shape == EmitterShape::Cone) {
        // Emit in cone direction with spread
        float angleRad = m_Config.coneAngle * 3.14159f / 180.0f;
        float spread = RandomRange(-angleRad / 2, angleRad / 2);
        
        float baseAngle = std::atan2(m_Config.direction.y, m_Config.direction.x);
        float finalAngle = baseAngle + spread;
        
        float speed = RandomRange(
            m_Config.velocityMin.Length(),
            m_Config.velocityMax.Length()
        );
        
        return {
            std::cos(finalAngle) * speed,
            std::sin(finalAngle) * speed
        };
    }
    
    return {
        RandomRange(m_Config.velocityMin.x, m_Config.velocityMax.x),
        RandomRange(m_Config.velocityMin.y, m_Config.velocityMax.y)
    };
}

float ParticleEmitter::RandomRange(float min, float max) const {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_RandomEngine);
}

Color ParticleEmitter::LerpColor(const Color& a, const Color& b, float t) const {
    t = std::max(0.0f, std::min(1.0f, t));
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

// Preset configurations
ParticleEmitterConfig ParticleEmitter::CreateFireEmitter() {
    ParticleEmitterConfig config;
    config.shape = EmitterShape::Point;
    config.emissionRate = 50.0f;
    config.lifetimeMin = 0.5f;
    config.lifetimeMax = 1.5f;
    config.velocityMin = {-20.0f, -100.0f};
    config.velocityMax = {20.0f, -50.0f};
    config.acceleration = {0.0f, -20.0f};
    config.sizeMin = 5.0f;
    config.sizeMax = 15.0f;
    config.sizeOverLifetime = 0.95f;
    config.startColor = {1.0f, 0.8f, 0.2f, 1.0f}; // Orange
    config.endColor = {1.0f, 0.0f, 0.0f, 0.0f};   // Red fade
    return config;
}

ParticleEmitterConfig ParticleEmitter::CreateSmokeEmitter() {
    ParticleEmitterConfig config;
    config.shape = EmitterShape::Circle;
    config.radius = 5.0f;
    config.emissionRate = 20.0f;
    config.lifetimeMin = 2.0f;
    config.lifetimeMax = 4.0f;
    config.velocityMin = {-15.0f, -50.0f};
    config.velocityMax = {15.0f, -30.0f};
    config.acceleration = {0.0f, -10.0f};
    config.sizeMin = 10.0f;
    config.sizeMax = 20.0f;
    config.sizeOverLifetime = 1.05f; // Grow
    config.startColor = {0.5f, 0.5f, 0.5f, 0.7f};
    config.endColor = {0.3f, 0.3f, 0.3f, 0.0f};
    return config;
}

ParticleEmitterConfig ParticleEmitter::CreateExplosionEmitter() {
    ParticleEmitterConfig config;
    config.shape = EmitterShape::Circle;
    config.radius = 0.0f;
    config.autoEmit = false; // Use manual burst
    config.burstCount = 100;
    config.lifetimeMin = 0.5f;
    config.lifetimeMax = 1.5f;
    config.velocityMin = {-200.0f, -200.0f};
    config.velocityMax = {200.0f, 200.0f};
    config.acceleration = {0.0f, 50.0f};
    config.sizeMin = 3.0f;
    config.sizeMax = 8.0f;
    config.sizeOverLifetime = 0.9f;
    config.startColor = {1.0f, 0.5f, 0.0f, 1.0f};
    config.endColor = {0.5f, 0.5f, 0.5f, 0.0f};
    config.loop = false;
    return config;
}

ParticleEmitterConfig ParticleEmitter::CreateRainEmitter() {
    ParticleEmitterConfig config;
    config.shape = EmitterShape::Box;
    config.boxSize = {800.0f, 10.0f};
    config.emissionRate = 100.0f;
    config.lifetimeMin = 2.0f;
    config.lifetimeMax = 3.0f;
    config.velocityMin = {-5.0f, 300.0f};
    config.velocityMax = {5.0f, 400.0f};
    config.acceleration = {0.0f, 50.0f};
    config.sizeMin = 2.0f;
    config.sizeMax = 4.0f;
    config.startColor = {0.5f, 0.5f, 1.0f, 0.6f};
    config.endColor = {0.5f, 0.5f, 1.0f, 0.3f};
    return config;
}

ParticleEmitterConfig ParticleEmitter::CreateSnowEmitter() {
    ParticleEmitterConfig config;
    config.shape = EmitterShape::Box;
    config.boxSize = {800.0f, 10.0f};
    config.emissionRate = 50.0f;
    config.lifetimeMin = 5.0f;
    config.lifetimeMax = 10.0f;
    config.velocityMin = {-20.0f, 30.0f};
    config.velocityMax = {20.0f, 60.0f};
    config.acceleration = {0.0f, 5.0f};
    config.sizeMin = 3.0f;
    config.sizeMax = 6.0f;
    config.angularVelocityMin = -1.0f;
    config.angularVelocityMax = 1.0f;
    config.startColor = {1.0f, 1.0f, 1.0f, 0.8f};
    config.endColor = {1.0f, 1.0f, 1.0f, 0.4f};
    return config;
}

void ParticleEmitter::Render() const {
    // Forward declaration - will be implemented with Renderer
    for (const auto& particle : m_Particles) {
        if (particle.active) {
            // Renderer::DrawParticle will be called from outside
            // This method is kept for future batched rendering optimization
        }
    }
}

} // namespace SAGE
