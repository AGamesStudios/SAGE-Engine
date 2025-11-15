#pragma once

#include "Graphics/Core/Types/Color.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Memory/Ref.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"

#include <optional>
#include <random>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace SAGE {

    class Texture;
    class Material;
    class IRenderBackend;

    /**
     * @brief Single particle data (POD for cache efficiency and backend independence)
     */
    struct Particle {
        Vector2 position{ 0.0f, 0.0f };
        Vector2 velocity{ 0.0f, 0.0f };
        Vector2 acceleration{ 0.0f, 0.0f };
        
        Color colorStart{ 1.0f, 1.0f, 1.0f, 1.0f };
        Color colorEnd{ 1.0f, 1.0f, 1.0f, 0.0f };
        Color currentColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        
        float size = 1.0f;
        float sizeStart = 10.0f;
        float sizeEnd = 2.0f;
        
        float rotation = 0.0f;
        float angularVelocity = 0.0f;
        
        float lifetime = 1.0f;
        float age = 0.0f;
        float remainingLifetime = 1.0f;
        
        uint32_t textureIndex = 0; // For texture atlas
        bool active = false;
    };

    /**
     * @brief Particle emitter configuration (backend-agnostic)
     */
    struct ParticleEmitterConfig {
        // Emission
        float emissionRate = 100.0f;        // Particles per second
        std::size_t maxParticles = 1000;
        bool looping = true;
        float duration = 5.0f;              // For non-looping emitters
        
        // Lifetime
        float minLifetime = 0.8f;
        float maxLifetime = 1.4f;
        
        // Position
        Vector2 position{ 0.0f, 0.0f };
        Vector2 positionVariance{ 10.0f, 10.0f };   // Random offset range
        float spawnRadius = 4.0f;
        
        // Velocity
        Vector2 velocityMin{ -50.0f, -50.0f };
        Vector2 velocityMax{ 50.0f, 50.0f };
        
        // Acceleration (e.g., gravity)
        Vector2 acceleration{ 0.0f, 98.0f };     // Downward gravity
        
        // Size
        float startSize = 10.0f;
        float endSize = 2.0f;
        float sizeVariance = 2.0f;
        
        // Color
        Color startColor = Color::White();
        Color endColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
        
        // Rotation
        float rotationMin = 0.0f;
        float rotationMax = 360.0f;
        float angularVelocityMin = -180.0f;
        float angularVelocityMax = 180.0f;
        
        // Texture
        Ref<Texture> texture;
        Vector2 uvMin{ 0.0f, 0.0f };
        Vector2 uvMax{ 1.0f, 1.0f };
        uint32_t textureID = 0;
        uint32_t textureFrameCount = 1;    // Number of frames in atlas
        bool animateTexture = false;
        
        // Material (optional)
        std::string materialName;
        Ref<Material> material;
        
        // Rendering
        bool worldSpace = true;
        bool screenSpace = false;
        bool overrideLayer = false;
        float layer = 0.0f;
        QuadEffect quadEffect{};
        
        // Blend mode
        enum class BlendMode {
            Alpha,
            Additive,
            Multiply
        } blendMode = BlendMode::Alpha;
        
        std::optional<uint32_t> randomSeed;
    };

    /**
     * @brief Particle affector (modifies particles over time)
     * Backend-agnostic particle modification
     */
    class ParticleAffector {
    public:
        virtual ~ParticleAffector() = default;
        virtual void Apply(Particle& particle, float deltaTime) = 0;
    };

    /**
     * @brief Gravity affector
     */
    class GravityAffector : public ParticleAffector {
    public:
        explicit GravityAffector(const Vector2& gravity) : m_Gravity(gravity) {}
        
        void Apply(Particle& particle, float deltaTime) override {
            particle.velocity.x += m_Gravity.x * deltaTime;
            particle.velocity.y += m_Gravity.y * deltaTime;
        }
        
        void SetGravity(const Vector2& gravity) { m_Gravity = gravity; }
        const Vector2& GetGravity() const { return m_Gravity; }
        
    private:
        Vector2 m_Gravity;
    };

    /**
     * @brief Attractor affector (pulls particles toward point)
     */
    class AttractorAffector : public ParticleAffector {
    public:
        AttractorAffector(const Vector2& position, float strength) 
            : m_Position(position), m_Strength(strength) {}
        
        void Apply(Particle& particle, float deltaTime) override {
            Vector2 direction = {
                m_Position.x - particle.position.x,
                m_Position.y - particle.position.y
            };
            
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (distance > 0.0f) {
                direction.x /= distance;
                direction.y /= distance;
                
                float force = m_Strength / std::max(distance, 1.0f);
                particle.velocity.x += direction.x * force * deltaTime;
                particle.velocity.y += direction.y * force * deltaTime;
            }
        }
        
        void SetPosition(const Vector2& position) { m_Position = position; }
        void SetStrength(float strength) { m_Strength = strength; }
        
    private:
        Vector2 m_Position;
        float m_Strength;
    };

    /**
     * @brief Vortex affector (spins particles)
     */
    class VortexAffector : public ParticleAffector {
    public:
        VortexAffector(const Vector2& center, float strength)
            : m_Center(center), m_Strength(strength) {}
        
        void Apply(Particle& particle, float deltaTime) override {
            Vector2 offset = {
                particle.position.x - m_Center.x,
                particle.position.y - m_Center.y
            };
            
            float distance = std::sqrt(offset.x * offset.x + offset.y * offset.y);
            if (distance > 0.0f) {
                // Perpendicular vector for rotation
                Vector2 tangent = {-offset.y, offset.x};
                float force = m_Strength / std::max(distance, 1.0f);
                
                particle.velocity.x += tangent.x * force * deltaTime;
                particle.velocity.y += tangent.y * force * deltaTime;
            }
        }
        
        void SetCenter(const Vector2& center) { m_Center = center; }
        void SetStrength(float strength) { m_Strength = strength; }
        
    private:
        Vector2 m_Center;
        float m_Strength;
    };

    /**
     * @brief Particle Emitter (backend-agnostic)
     * Manages particle lifecycle and updates
     * Rendering delegated to IRenderBackend
     */
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

        // Affector management
        void AddAffector(std::unique_ptr<ParticleAffector> affector);
        void ClearAffectors();

        void Update(float deltaTime);
        
        // Backend-agnostic rendering via IRenderBackend
        void Render(IRenderBackend* backend, bool screenSpace = false) const;

        void Burst(std::size_t count);
        void Clear();

        std::size_t GetAliveCount() const;
        const std::vector<Particle>& GetParticles() const { return m_Particles; }

        // Statistics
        struct Stats {
            std::size_t activeParticles = 0;
            std::size_t maxParticles = 0;
            float emissionRate = 0.0f;
        };
        Stats GetStats() const;

    private:
        void InitializeRandom();
        void SpawnParticle();
        void UpdateParticle(Particle& particle, float deltaTime);
        float RandomFloat(float minValue, float maxValue);
        Vector2 RandomInCircle(float radius);
        Color LerpColor(const Color& a, const Color& b, float t) const;
        void ResolveMaterial();

        ParticleEmitterConfig m_Config{};
        Vector2 m_Position = Vector2::Zero();
        std::vector<Particle> m_Particles;
        std::vector<std::unique_ptr<ParticleAffector>> m_Affectors;
        std::mt19937 m_Rng;
        std::uniform_real_distribution<float> m_Distribution{ 0.0f, 1.0f };
        float m_EmissionAccumulator = 0.0f;
        float m_Age = 0.0f;
        bool m_Active = true;
        Ref<Material> m_Material;
        Ref<Texture> m_Texture;
    };

} // namespace SAGE
