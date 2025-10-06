#pragma once

#include "../Graphics/Color.h"
#include "../Graphics/MathTypes.h"
#include "../Memory/Ref.h"
#include "RendererTypes.h"

#include <optional>
#include <random>
#include <vector>

namespace SAGE {

    class Texture;
    class Material;

    struct ParticleEmitterConfig {
        std::size_t maxParticles = 512;
        float emissionRate = 0.0f; // particles per second
        float minLifetime = 0.8f;
        float maxLifetime = 1.4f;
        Vector2 velocityMin{ -40.0f, 40.0f };
        Vector2 velocityMax{ 40.0f, 120.0f };
        float startSize = 10.0f;
        float endSize = 2.0f;
        Color startColor = Color::White();
        Color endColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
        float spawnRadius = 4.0f;
        bool worldSpace = true;
        bool overrideLayer = false;
        float layer = 0.0f;
        bool screenSpace = false;
        QuadEffect quadEffect{};
        Ref<Texture> texture;
        Vector2 uvMin{ 0.0f, 0.0f };
        Vector2 uvMax{ 1.0f, 1.0f };
        std::string materialName;
        Ref<Material> material;
        std::optional<uint32_t> randomSeed;
    };

    struct Particle {
        Vector2 position{ 0.0f, 0.0f };
        Vector2 velocity{ 0.0f, 0.0f };
        float lifetime = 1.0f;
        float remainingLifetime = 1.0f;
        float age = 0.0f;
        float size = 1.0f;
        Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
    };

    class ParticleEmitter {
    public:
        ParticleEmitter();
        explicit ParticleEmitter(ParticleEmitterConfig config, const Vector2& position = Vector2::Zero());

        void SetPosition(const Vector2& position) { m_Position = position; }
        const Vector2& GetPosition() const { return m_Position; }

        void SetEmissionRate(float rate) { m_Config.emissionRate = rate; }
        void SetConfig(const ParticleEmitterConfig& config);
        const ParticleEmitterConfig& GetConfig() const { return m_Config; }

        void SetMaterial(const Ref<Material>& material);
        void SetMaterial(const std::string& materialName);
        void SetTexture(const Ref<Texture>& texture, const Vector2& uvMin = Vector2(0.0f, 0.0f), const Vector2& uvMax = Vector2(1.0f, 1.0f));
        const Ref<Material>& GetMaterial() const { return m_Material; }
        const Ref<Texture>& GetTexture() const { return m_Texture; }

        void Update(float deltaTime);
        void Render(bool screenSpace = false) const;

        void Burst(std::size_t count);
        void Clear();

        std::size_t GetAliveCount() const { return m_Particles.size(); }
        const std::vector<Particle>& GetParticles() const { return m_Particles; }

    private:
        void InitializeRandom();
        void SpawnParticle();
        float RandomFloat(float minValue, float maxValue);
        Vector2 RandomInCircle(float radius);
        Color LerpColor(const Color& a, const Color& b, float t) const;
        void ResolveMaterial();

        ParticleEmitterConfig m_Config{};
        Vector2 m_Position = Vector2::Zero();
        std::vector<Particle> m_Particles;
        std::mt19937 m_Rng;
        std::uniform_real_distribution<float> m_Distribution{ 0.0f, 1.0f };
        float m_EmissionAccumulator = 0.0f;
        Ref<Material> m_Material;
        Ref<Texture> m_Texture;
    };

} // namespace SAGE
