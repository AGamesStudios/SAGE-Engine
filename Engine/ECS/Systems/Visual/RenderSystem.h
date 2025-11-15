#pragma once

#include "ECS/System.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "Graphics/API/Renderer.h"

namespace SAGE::ECS {

/// @brief Система рендеринга спрайтов
/// Отрисовывает все сущности с TransformComponent и SpriteComponent
class RenderSystem : public ISystem {
public:
    RenderSystem() {
        SetPriority(1000); // Рендеринг должен быть последним
    }

    void Update(Registry& registry, float deltaTime) override {
        (void)deltaTime;
        // Начало рендера сцены
        Renderer::BeginScene();

        auto spriteViews = registry.GetAllWith<SpriteComponent>();

        for (auto& view : spriteViews) {
            if (!view.component) {
                continue;
            }

            const auto entity = view.entity;
            auto* transform = registry.GetComponent<TransformComponent>(entity);
            if (!transform) {
                continue;
            }

            auto& sprite = *view.component;

            // Here we would push sprite draw data into the renderer.
            (void)sprite;
            (void)transform;
        }

        // Завершение рендера
        Renderer::EndScene();
    }

    std::string GetName() const override {
        return "RenderSystem";
    }
};

} // namespace SAGE::ECS
