#pragma once

#include "ECS/System.h"
#include "ECS/Components/Effects/ParticleSystemComponent.h"
#include "ECS/Components/Core/TransformComponent.h"
#include <vector>

namespace SAGE::ECS {

/// @brief Система обновления частиц
/// Обновляет все ParticleSystemComponent
class ParticleUpdateSystem : public ISystem {
public:
    ParticleUpdateSystem() {
        SetPriority(30);
    }

    void Update(Registry& registry, float deltaTime) override {
        if (deltaTime <= 0.0f) {
            return;
        }

        auto particleSystems = registry.GetAllWith<ParticleSystemComponent>();
        std::vector<Entity> toDestroy;

        for (auto& view : particleSystems) {
            if (!view.component) {
                continue;
            }

            auto& ps = *view.component;

            if (!ps.emitter) {
                ps.emitter = std::make_unique<ParticleEmitter>(ps.config);
            }

            if (auto* transform = registry.GetComponent<TransformComponent>(view.entity)) {
                ps.SetPosition(transform->position);
            }

            const bool hasActiveParticles = ps.IsPlaying();
            const bool shouldSimulate = ps.playOnStart || hasActiveParticles;
            if (!shouldSimulate) {
                continue;
            }

            ps.hasStarted = ps.hasStarted || ps.playOnStart || hasActiveParticles;

            float originalEmissionRate = ps.config.emissionRate;
            bool emissionSuppressed = false;
            if (!ps.playOnStart && hasActiveParticles) {
                ps.emitter->SetEmissionRate(0.0f);
                emissionSuppressed = true;
            }

            ps.Update(deltaTime);

            if (emissionSuppressed) {
                ps.config.emissionRate = originalEmissionRate;
                ps.emitter->SetEmissionRate(originalEmissionRate);
            }

            if (ps.autoDestroy && ps.hasStarted && !ps.config.looping && !ps.IsPlaying()) {
                toDestroy.push_back(view.entity);
            }
        }

        for (Entity entity : toDestroy) {
            registry.DestroyEntity(entity);
        }
    }

    std::string GetName() const override {
        return "ParticleUpdateSystem";
    }
};

} // namespace SAGE::ECS
