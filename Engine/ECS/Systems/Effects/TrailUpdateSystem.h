#pragma once

#include "ECS/System.h"
#include "ECS/Components/Effects/TrailComponent.h"
#include "ECS/Components/Core/TransformComponent.h"

namespace SAGE::ECS {

/// @brief Система обновления следов (trails)
/// Создаёт эффект следа за движущимися объектами
class TrailUpdateSystem : public ISystem {
public:
    TrailUpdateSystem() {
        SetPriority(40);
    }

    void Update(Registry& registry, float deltaTime) override {
        auto trails = registry.GetAllWith<TrailComponent>();

        for (auto& view : trails) {
            if (!view.component) {
                continue;
            }

            const auto entity = view.entity;
            auto* transform = registry.GetComponent<TransformComponent>(entity);
            if (!transform) {
                continue;
            }

            auto& trailComponent = *view.component;

            if (trailComponent.enableTrail) {
                trailComponent.trail.SetActive(true);
                trailComponent.trail.Update(deltaTime, transform->position);
            } else if (trailComponent.trail.IsActive()) {
                trailComponent.trail.SetActive(false);
            }

            if (trailComponent.enableDashEffect) {
                if (!trailComponent.dashEffect.IsActive()) {
                    trailComponent.dashEffect.Start();
                }

                trailComponent.dashEffect.Update(
                    deltaTime,
                    transform->position,
                    transform->rotation,
                    transform->scale
                );
            } else if (trailComponent.dashEffect.IsActive()) {
                trailComponent.dashEffect.Stop();
            }
        }
    }

    std::string GetName() const override {
        return "TrailUpdateSystem";
    }
};

} // namespace SAGE::ECS
