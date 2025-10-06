#include "ParticleSystem.h"

#include "Renderer.h"

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
        m_Particles.clear();
        m_Particles.reserve(m_Config.maxParticles);
        m_EmissionAccumulator = 0.0f;
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

        if (m_Config.emissionRate > 0.0f && m_Particles.size() < m_Config.maxParticles) {
            m_EmissionAccumulator += m_Config.emissionRate * deltaTime;
            int particlesToEmit = static_cast<int>(std::floor(m_EmissionAccumulator));
            if (particlesToEmit > 0) {
                m_EmissionAccumulator -= static_cast<float>(particlesToEmit);
                for (int i = 0; i < particlesToEmit && m_Particles.size() < m_Config.maxParticles; ++i) {
                    SpawnParticle();
                }
            }
        }

        auto it = m_Particles.begin();
        while (it != m_Particles.end()) {
            Particle& particle = *it;
            particle.remainingLifetime -= deltaTime;
            if (particle.remainingLifetime <= 0.0f) {
                it = m_Particles.erase(it);
                continue;
            }

            particle.age = particle.lifetime - particle.remainingLifetime;
            particle.position += particle.velocity * deltaTime;
            float normalizedAge = 1.0f - (particle.remainingLifetime / particle.lifetime);
            normalizedAge = std::clamp(normalizedAge, 0.0f, 1.0f);
            particle.size = Lerp(m_Config.startSize, m_Config.endSize, normalizedAge);
            particle.color = LerpColor(m_Config.startColor, m_Config.endColor, normalizedAge);
            ++it;
        }
    }

    void ParticleEmitter::Render(bool screenSpace) const {
        if (m_Particles.empty()) {
            return;
        }

        bool finalScreenSpace = screenSpace || m_Config.screenSpace;

        bool pushedLayer = false;
        if (m_Config.overrideLayer) {
            Renderer::PushLayer(m_Config.layer);
            pushedLayer = true;
        }

        MaterialId previousMaterial = 0;
        bool changedMaterial = false;
        if (m_Material) {
            previousMaterial = Renderer::SetMaterial(m_Material->GetId());
            changedMaterial = true;
        }

        Renderer::PushEffect(m_Config.quadEffect);

        for (const Particle& particle : m_Particles) {
            Float2 sizeVec(particle.size, particle.size);
            Float2 drawPos = m_Config.worldSpace ? particle.position : (m_Position + particle.position);

            QuadDesc desc;
            desc.position = drawPos;
            desc.size = sizeVec;
            desc.color = particle.color;
            desc.screenSpace = finalScreenSpace;

            if (m_Texture) {
                desc.texture = m_Texture;
                desc.uvMin = m_Config.uvMin;
                desc.uvMax = m_Config.uvMax;
            }

            Renderer::DrawQuad(desc);
        }

        Renderer::PopEffect();
        if (changedMaterial) {
            Renderer::SetMaterial(previousMaterial);
        }
        if (pushedLayer) {
            Renderer::PopLayer();
        }
    }

    void ParticleEmitter::Burst(std::size_t count) {
        for (std::size_t i = 0; i < count && m_Particles.size() < m_Config.maxParticles; ++i) {
            SpawnParticle();
        }
    }

    void ParticleEmitter::Clear() {
        m_Particles.clear();
        m_EmissionAccumulator = 0.0f;
    }

    void ParticleEmitter::SpawnParticle() {
        float lifetime = RandomFloat(m_Config.minLifetime, m_Config.maxLifetime);
        if (lifetime <= 0.0f) {
            lifetime = 0.1f;
        }

        Particle particle;
        particle.lifetime = lifetime;
        particle.remainingLifetime = lifetime;
        particle.age = 0.0f;
        particle.size = m_Config.startSize;
        particle.color = m_Config.startColor;
        particle.velocity = Vector2(
            RandomFloat(m_Config.velocityMin.x, m_Config.velocityMax.x),
            RandomFloat(m_Config.velocityMin.y, m_Config.velocityMax.y)
        );
        Vector2 offset = RandomInCircle(m_Config.spawnRadius);
        particle.position = m_Config.worldSpace ? (m_Position + offset) : offset;

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
