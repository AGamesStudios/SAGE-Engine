#pragma once

#include "ECS/System.h"
#include "ECS/Components/Effects/ScreenEffectsComponent.h"
#include "Graphics/API/Renderer.h"

namespace SAGE::ECS {

/// @brief Система экранных эффектов
/// Применяет post-processing эффекты
class ScreenEffectsSystem : public ISystem {
public:
    ScreenEffectsSystem() {
        SetPriority(950); // Почти последним
    }

    void Update(Registry& registry, float deltaTime) override {
        auto effects = registry.GetAllWith<ScreenEffectsComponent>();

        for (auto& view : effects) {
            if (!view.component) {
                continue;
            }

            auto& effect = *view.component;
            if (effect.enableShake && effect.shake.IsActive()) {
                if (effect.shake.timer == 0.0f) {
                    ::SAGE::Renderer::PushScreenShake(
                        effect.shake.intensity,
                        effect.shake.frequency,
                        effect.shake.duration);
                }
                if (deltaTime > 0.0f) {
                    effect.shake.Update(deltaTime);
                }
            }

            if (effect.enableFlash && effect.flash.IsActive() && deltaTime > 0.0f) {
                effect.flash.Update(deltaTime);
            }

            if (effect.enableTransition && effect.transition.IsActive() && deltaTime > 0.0f) {
                effect.transition.Update(deltaTime);
            }

            // Post-processing hooks will map other effect fields when renderer exposes them.
        }
    }

    std::string GetName() const override {
        return "ScreenEffectsSystem";
    }
};

} // namespace SAGE::ECS
