#include "SAGE/Graphics/ParticleSystem.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Sprite.h"
#include <random>
#include <algorithm>

namespace SAGE {

namespace {
    std::random_device s_RandomDevice;
    std::mt19937 s_RandomEngine(s_RandomDevice());

    float RandomRange(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(s_RandomEngine);
    }

    Color LerpColor(const Color& a, const Color& b, float t) {
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }
}

// ============================================
// ParticleSystem Implementation
// ============================================

ParticleSystem::ParticleSystem(size_t maxParticles) {
    m_Particles.resize(maxParticles);
}

void ParticleSystem::SetEmitterConfig(const EmitterConfig& config) {
    m_Config = config;
    
    // Resize particle pool if needed
    if (m_Config.maxParticles > static_cast<int>(m_Particles.size())) {
        m_Particles.resize(m_Config.maxParticles);
    }
}

void ParticleSystem::Start() {
    m_Active = true;
    m_Paused = false;
    m_EmissionTimer = 0.0f;
}

void ParticleSystem::Stop() {
    m_Active = false;
    m_Paused = false;
    Clear();
}

void ParticleSystem::Pause() {
    m_Paused = true;
}

void ParticleSystem::Resume() {
    m_Paused = false;
}

void ParticleSystem::Clear() {
    for (auto& particle : m_Particles) {
        particle.active = false;
    }
}

void ParticleSystem::Emit(int count) {
    for (int i = 0; i < count; ++i) {
        EmitParticle();
    }
}

void ParticleSystem::Burst(int count) {
    Emit(count);
}

void ParticleSystem::Update(float deltaTime) {
    if (!m_Active || m_Paused) {
        return;
    }

    // Auto-emit particles
    if (m_Config.autoEmit && m_Config.emissionRate > 0.0f) {
        m_EmissionTimer += deltaTime;
        float emissionInterval = 1.0f / m_Config.emissionRate;
        
        if (emissionInterval > 0.0f) {
            while (m_EmissionTimer >= emissionInterval) {
                EmitParticle();
                m_EmissionTimer -= emissionInterval;
            }
        }
    }

    // Update active particles
    for (size_t i = 0; i < m_ActiveCount; ++i) {
        Particle& p = m_Particles[i];
        UpdateParticle(p, deltaTime);
        
        if (!p.active) {
            // Swap with last active
            if (i != m_ActiveCount - 1) {
                m_Particles[i] = m_Particles[m_ActiveCount - 1];
            }
            m_ActiveCount--;
            i--; // Re-process this index
        }
    }
}

void ParticleSystem::EmitParticle() {
    if (m_ActiveCount >= m_Particles.size()) {
        return; // Pool full
    }

    Particle& p = m_Particles[m_ActiveCount];
    m_ActiveCount++;
    
    // Initialize particle
    p.position = m_Config.position + Vector2{
        RandomRange(-m_Config.emissionArea.x, m_Config.emissionArea.x),
        RandomRange(-m_Config.emissionArea.y, m_Config.emissionArea.y)
    };
    
    p.velocity = {
        RandomRange(m_Config.velocityMin.x, m_Config.velocityMax.x),
        RandomRange(m_Config.velocityMin.y, m_Config.velocityMax.y)
    };
    
    p.acceleration = m_Config.acceleration;
    p.color = m_Config.startColor;
    p.lifetime = RandomRange(m_Config.lifetimeMin, m_Config.lifetimeMax);
    p.age = 0.0f;
    
    float startSize = m_Config.sizeStart + RandomRange(-m_Config.sizeVariation, m_Config.sizeVariation);
    p.size = std::max(0.0f, startSize);
    
    p.rotation = m_Config.rotationStart + RandomRange(-m_Config.rotationVariation, m_Config.rotationVariation);
    p.angularVelocity = 0.0f; // TODO: Add angular velocity config
    p.active = true;
}

void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime) {
    particle.age += deltaTime;
    
    // Kill old particles
    if (particle.age >= particle.lifetime) {
        particle.active = false;
        return;
    }

    float t = particle.age / particle.lifetime;

    // Physics
    // Radial/Tangential acceleration
    if (m_Config.radialAcceleration != 0.0f || m_Config.tangentialAcceleration != 0.0f) {
        Vector2 diff = particle.position - m_Config.position;
        Vector2 radial = diff.Normalized();
        Vector2 tangential = {-radial.y, radial.x};
        
        particle.velocity += radial * (m_Config.radialAcceleration * deltaTime);
        particle.velocity += tangential * (m_Config.tangentialAcceleration * deltaTime);
    }

    particle.velocity += particle.acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;
    
    // Rotation
    float rotT = t; // Linear interpolation for rotation
    particle.rotation = m_Config.rotationStart + (m_Config.rotationEnd - m_Config.rotationStart) * rotT;

    // Size interpolation
    particle.size = m_Config.sizeStart + (m_Config.sizeEnd - m_Config.sizeStart) * t;

    // Color interpolation
    particle.color = LerpColor(m_Config.startColor, m_Config.endColor, t);
}

// ============================================
// ParticleRenderer Implementation
// ============================================

void ParticleRenderer::Render(const ParticleSystem& system) {
    auto texture = system.GetTexture();
    if (!texture) {
        return; // No texture to render
    }

    for (const auto& particle : system.GetParticles()) {
        if (!particle.active) {
            continue;
        }

        Sprite sprite;
        sprite.SetTexture(texture);
        sprite.transform.position = particle.position;
        sprite.transform.rotation = particle.rotation;
        
        // Avoid division by zero
        float texWidth = static_cast<float>(texture->GetWidth());
        float texHeight = static_cast<float>(texture->GetHeight());
        if (texWidth > 0.0f && texHeight > 0.0f) {
            sprite.transform.scale = Vector2(particle.size / texWidth, particle.size / texHeight);
        } else {
            sprite.transform.scale = Vector2(1.0f, 1.0f);
        }
        sprite.tint = particle.color;
        sprite.visible = true;

        Renderer::SubmitSprite(sprite);
    }
}

} // namespace SAGE
