#include "ParticleSystem.h"

#include "Graphics/API/Renderer.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"

#include <algorithm>
#include <cmath>

namespace SAGE {

    namespace {
        constexpr float TwoPi = 6.28318530718f;

        float Lerp(float a, float b, float t) {
            return a + (b - a) * t;
        }
    }

    ParticleEmitter::ParticleEmitter()
        : ParticleEmitter(ParticleEmitterConfig{}, Vector2::Zero()) {
    }

    ParticleEmitter::ParticleEmitter(ParticleEmitterConfig config, const Vector2& position)
        : m_Config(std::move(config)), m_Position(position) {
        m_Particles.reserve(m_Config.maxParticles);
        InitializeRandom();
        m_Texture = m_Config.texture;
        ResolveMaterial();
    }

    void ParticleEmitter::SetConfig(const ParticleEmitterConfig& config) {
        m_Config = config;
        // Don't clear particles on config update - allows runtime parameter changes
        // m_Particles.clear(); // REMOVED - was destroying all active particles
        m_Particles.reserve(m_Config.maxParticles);
        // Don't reset emission state unless explicitly requested
        // m_EmissionAccumulator = 0.0f;
        // m_Age = 0.0f;
        // m_Active = true;
        InitializeRandom();
        m_Texture = m_Config.texture;
        ResolveMaterial();
    }

    void ParticleEmitter::SetMaterial(const Ref<Material>& material) {
        m_Config.material = material;
        m_Config.materialName = material ? material->GetName() : std::string{};
        ResolveMaterial();
    }

    void ParticleEmitter::SetMaterial(const std::string& materialName) {
        m_Config.materialName = materialName;
        m_Config.material.reset();
        if (!materialName.empty() && MaterialLibrary::Exists(materialName)) {
            m_Config.material = MaterialLibrary::Get(materialName);
        }
        ResolveMaterial();
    }

    void ParticleEmitter::SetTexture(const Ref<Texture>& texture, const Vector2& uvMin, const Vector2& uvMax) {
        m_Config.texture = texture;
        m_Config.uvMin = uvMin;
        m_Config.uvMax = uvMax;
        m_Texture = texture;
    }

    void ParticleEmitter::AddAffector(std::unique_ptr<ParticleAffector> affector) {
        if (affector) {
            m_Affectors.push_back(std::move(affector));
        }
    }

    void ParticleEmitter::ClearAffectors() {
        m_Affectors.clear();
    }

    void ParticleEmitter::InitializeRandom() {
        if (m_Config.randomSeed.has_value()) {
            m_Rng.seed(*m_Config.randomSeed);
        }
        else {
            std::random_device rd;
            m_Rng.seed(rd());
        }
    }

    float ParticleEmitter::RandomFloat(float minValue, float maxValue) {
        float t = m_Distribution(m_Rng);
        return Lerp(minValue, maxValue, t);
    }

    Vector2 ParticleEmitter::RandomInCircle(float radius) {
        if (radius <= 0.0f) {
            return Vector2::Zero();
        }

        float angle = RandomFloat(0.0f, TwoPi);
        float distance = std::sqrt(RandomFloat(0.0f, 1.0f)) * radius;
        return Vector2(std::cos(angle) * distance, std::sin(angle) * distance);
    }

    Color ParticleEmitter::LerpColor(const Color& a, const Color& b, float t) const {
        return Color(
            Lerp(a.r, b.r, t),
            Lerp(a.g, b.g, t),
            Lerp(a.b, b.b, t),
            Lerp(a.a, b.a, t)
        );
    }

    void ParticleEmitter::Update(float deltaTime) {
        if (deltaTime <= 0.0f) {
            return;
        }

        // Update emitter age
        m_Age += deltaTime;

        // Check if non-looping emitter expired
        if (!m_Config.looping && m_Age >= m_Config.duration) {
            m_Active = false;
        }

        // Emit new particles
        if (m_Active && m_Config.emissionRate > 0.0f) {
            m_EmissionAccumulator += m_Config.emissionRate * deltaTime;
            while (m_EmissionAccumulator >= 1.0f && m_Particles.size() < m_Config.maxParticles) {
                SpawnParticle();
                m_EmissionAccumulator -= 1.0f;
            }
        }

        // Update existing particles
        auto removeIt = std::remove_if(m_Particles.begin(), m_Particles.end(),
            [this, deltaTime](Particle& particle) {
                UpdateParticle(particle, deltaTime);
                return !particle.active;
            });
        
        m_Particles.erase(removeIt, m_Particles.end());
    }

    void ParticleEmitter::UpdateParticle(Particle& particle, float deltaTime) {
        // Age particle
        particle.age += deltaTime;
        particle.remainingLifetime -= deltaTime;

        if (particle.age >= particle.lifetime || particle.remainingLifetime <= 0.0f) {
            particle.active = false;
            return;
        }

        // Apply affectors
        for (auto& affector : m_Affectors) {
            affector->Apply(particle, deltaTime);
        }

        // Apply velocity and acceleration
        particle.velocity.x += particle.acceleration.x * deltaTime;
        particle.velocity.y += particle.acceleration.y * deltaTime;

        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;

        // Update rotation
        particle.rotation += particle.angularVelocity * deltaTime;

        // Interpolate properties
        float t = particle.age / particle.lifetime;

        // Size
        particle.size = particle.sizeStart + (particle.sizeEnd - particle.sizeStart) * t;

        // Color
        particle.currentColor = LerpColor(particle.colorStart, particle.colorEnd, t);

        // Texture animation
        if (m_Config.animateTexture && m_Config.textureFrameCount > 1) {
            particle.textureIndex = static_cast<uint32_t>(t * m_Config.textureFrameCount) 
                                  % m_Config.textureFrameCount;
        }
    }

    std::size_t ParticleEmitter::GetAliveCount() const {
        return std::count_if(m_Particles.begin(), m_Particles.end(),
            [](const Particle& p) { return p.active; });
    }

    ParticleEmitter::Stats ParticleEmitter::GetStats() const {
        Stats stats;
        stats.activeParticles = GetAliveCount();
        stats.maxParticles = m_Config.maxParticles;
        stats.emissionRate = m_Config.emissionRate;
        return stats;
    }

    void ParticleEmitter::Render(IRenderBackend* backend, bool screenSpace) const {
        // NOTE: IRenderBackend no longer exposes high-level DrawQuad helpers.
        // We delegate sprite/quad submission to Renderer::DrawQuad using QuadDesc.
        // Backend pointer kept for potential future low-level state hooks (currently unused).
        if (m_Particles.empty()) {
            return;
        }

        // Apply blend mode based on config
        BlendMode blendMode = BlendMode::Alpha; // Default
        switch (m_Config.blendMode) {
            case ParticleEmitterConfig::BlendMode::Alpha:
                blendMode = BlendMode::Alpha;
                break;
            case ParticleEmitterConfig::BlendMode::Additive:
                blendMode = BlendMode::Additive;
                break;
            case ParticleEmitterConfig::BlendMode::Multiply:
                blendMode = BlendMode::Multiply;
                break;
        }
        
        Renderer::PushBlendMode(blendMode);

        for (const Particle& particle : m_Particles) {
            if (!particle.active) continue;

            QuadDesc desc;
            desc.position = m_Config.worldSpace ? Float2(particle.position.x, particle.position.y)
                                                : Float2(m_Position.x + particle.position.x, m_Position.y + particle.position.y);
            desc.size = Float2(particle.size, particle.size);
            desc.color = particle.currentColor;
            desc.rotation = particle.rotation; // degrees expected
            desc.screenSpace = screenSpace || m_Config.screenSpace;
            desc.source = QuadDesc::QuadSource::Generic;

            if (m_Texture) {
                desc.texture = m_Texture;
            }

            Renderer::DrawQuad(desc);
        }
        
        Renderer::PopBlendMode();
    }

    void ParticleEmitter::Burst(std::size_t count) {
        for (std::size_t i = 0; i < count && m_Particles.size() < m_Config.maxParticles; ++i) {
            SpawnParticle();
        }
    }

    void ParticleEmitter::Clear() {
        m_Particles.clear();
        m_EmissionAccumulator = 0.0f;
        m_Age = 0.0f;
        m_Active = true;
    }

    void ParticleEmitter::SpawnParticle() {
        float lifetime = RandomFloat(m_Config.minLifetime, m_Config.maxLifetime);
        if (lifetime <= 0.0f) {
            lifetime = 0.1f;
        }

        Particle particle;
        particle.active = true;
        particle.lifetime = lifetime;
        particle.remainingLifetime = lifetime;
        particle.age = 0.0f;
        
        // Size
        particle.sizeStart = m_Config.startSize + RandomFloat(-m_Config.sizeVariance, m_Config.sizeVariance);
        particle.sizeEnd = m_Config.endSize + RandomFloat(-m_Config.sizeVariance, m_Config.sizeVariance);
        particle.size = particle.sizeStart;
        
        // Color
        particle.colorStart = m_Config.startColor;
        particle.colorEnd = m_Config.endColor;
        particle.currentColor = m_Config.startColor;
        
        // Velocity
        particle.velocity = Vector2(
            RandomFloat(m_Config.velocityMin.x, m_Config.velocityMax.x),
            RandomFloat(m_Config.velocityMin.y, m_Config.velocityMax.y)
        );
        
        // Acceleration
        particle.acceleration = m_Config.acceleration;
        
        // Position
        Vector2 offset = RandomInCircle(m_Config.spawnRadius);
        // Apply position variance (rectangular offset on top of circular spawn)
        offset.x += RandomFloat(-m_Config.positionVariance.x, m_Config.positionVariance.x);
        offset.y += RandomFloat(-m_Config.positionVariance.y, m_Config.positionVariance.y);
        particle.position = m_Config.worldSpace ? (m_Position + offset) : offset;
        
        // Rotation
        particle.rotation = RandomFloat(m_Config.rotationMin, m_Config.rotationMax);
        particle.angularVelocity = RandomFloat(m_Config.angularVelocityMin, m_Config.angularVelocityMax);
        
        // Texture
        particle.textureIndex = 0;

        m_Particles.push_back(particle);
    }

    void ParticleEmitter::ResolveMaterial() {
        m_Material.reset();
        if (m_Config.material) {
            m_Material = m_Config.material;
            return;
        }

        if (!m_Config.materialName.empty() && MaterialLibrary::Exists(m_Config.materialName)) {
            m_Material = MaterialLibrary::Get(m_Config.materialName);
            m_Config.material = m_Material;
        }
    }

} // namespace SAGE
