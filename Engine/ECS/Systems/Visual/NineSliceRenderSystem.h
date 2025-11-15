#pragma once

#include "ECS/System.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/UI/NineSliceComponent.h"
#include "Graphics/API/Renderer.h"

namespace SAGE::ECS {

/// @brief Система рендеринга NineSlice UI элементов
class NineSliceRenderSystem : public ISystem {
public:
    NineSliceRenderSystem() {
        SetPriority(900); // UI рендерится после основной геометрии но до оверлеев
    }

    void Update(Registry& registry, float /*deltaTime*/) override {
        auto nineSliceViews = registry.GetAllWith<NineSliceComponent>();

        for (auto& view : nineSliceViews) {
            if (!view.component) {
                continue;
            }

            const auto entity = view.entity;
            auto* transform = registry.GetComponent<TransformComponent>(entity);
            if (!transform) {
                continue;
            }

            auto& nineSlice = *view.component;

            // TODO: Integrate real rendering path once Renderer exposes nine-slice helpers.
            (void)nineSlice;
            (void)transform;
        }
    }

    std::string GetName() const override {
        return "NineSliceRenderSystem";
    }
};

} // namespace SAGE::ECS
