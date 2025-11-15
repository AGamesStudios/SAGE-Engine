#pragma once

#include "Graphics/Rendering/Effects/Particles/ParticleSystem.h"
#include "Memory/Ref.h"
#include <memory>

namespace SAGE::ECS {

/// @brief Компонент для системы частиц
struct ParticleSystemComponent {
    std::unique_ptr<ParticleEmitter> emitter = nullptr;
    
    // Configuration
    ParticleEmitterConfig config;
    
    // Control
    bool playOnStart = true;
    bool autoDestroy = false;  // Destroy entity when particles finish
    bool hasStarted = false;   // Tracks if emission has begun
    
    ParticleSystemComponent() {
        // Default fire-like particles
        config.emissionRate = 50.0f;
        config.maxParticles = 500;
        config.looping = true;
        
        config.minLifetime = 0.5f;
        config.maxLifetime = 1.5f;
        
        config.position = Vector2(0.0f, 0.0f);
        config.positionVariance = Vector2(5.0f, 5.0f);
        config.spawnRadius = 2.0f;
        
        config.velocityMin = Vector2(-30.0f, -80.0f);
        config.velocityMax = Vector2(30.0f, -40.0f);
        
        config.acceleration = Vector2(0.0f, 20.0f);  // Slight upward
        
        config.startSize = 8.0f;
        config.endSize = 2.0f;
        config.sizeVariance = 2.0f;
        
        config.startColor = Color(1.0f, 0.8f, 0.2f, 1.0f);  // Orange
        config.endColor = Color(1.0f, 0.2f, 0.0f, 0.0f);    // Red fade
        
        config.rotationMin = 0.0f;
        config.rotationMax = 360.0f;
        config.angularVelocityMin = -90.0f;
        config.angularVelocityMax = 90.0f;
        
        // Create emitter with config
        emitter = std::make_unique<ParticleEmitter>(config);
    }
    
    explicit ParticleSystemComponent(const ParticleEmitterConfig& cfg)
        : config(cfg) {
        emitter = std::make_unique<ParticleEmitter>(config);
    }
    
    void Play() {
        playOnStart = true;
        if (!emitter) {
            emitter = std::make_unique<ParticleEmitter>(config);
        }
    }
    
    void Stop() {
        playOnStart = false;
        hasStarted = false;
        if (emitter) {
            emitter->Clear();
        }
    }
    
    void Reset() {
        playOnStart = false;
        hasStarted = false;
        if (emitter) {
            emitter->Clear();
        }
    }
    
    bool IsPlaying() const {
        return emitter && emitter->GetAliveCount() > 0;
    }
    
    void Update(float deltaTime) {
        if (emitter) {
            // Only update config if it changed (avoid clearing particles every frame)
            if (configDirty) {
                emitter->SetConfig(config);
                configDirty = false;
            }
            emitter->Update(deltaTime);
        }
    }
    
    void SetPosition(const Vector2& pos) {
        config.position = pos;
        if (emitter) {
            emitter->SetPosition(pos);
        }
    }
    
    void SetTexture(Ref<Texture> texture) {
        config.texture = texture;
        configDirty = true;
        if (emitter) {
            emitter->SetTexture(texture);
        }
    }
    
    void SetEmissionRate(float rate) {
        config.emissionRate = rate;
        configDirty = true;
    }
    
    void SetMaxParticles(size_t max) {
        config.maxParticles = max;
        configDirty = true;
    }
    
    void Render(IRenderBackend* backend) const {
        if (emitter && backend) {
            emitter->Render(backend, false);
        }
    }

private:
    bool configDirty = false;
};

} // namespace SAGE::ECS
