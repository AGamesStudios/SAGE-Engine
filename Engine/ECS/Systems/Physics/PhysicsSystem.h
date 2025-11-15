#pragma once

#include "ECS/System.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "ECS/Components/Physics/ColliderComponent.h"
#include "Physics/IPhysicsBackend.h"
#include "Physics/Box2DBackend.h"
#include <memory>

namespace SAGE::ECS {

/// @brief Система физики
/// Интегрирует Box2D с ECS через IPhysicsBackend
class PhysicsSystem : public ISystem {
public:
    PhysicsSystem() {
        SetPriority(20); // Физика выполняется рано
        
        // Создаём Box2D backend
        m_Backend = std::make_unique<Physics::Box2DBackend>();
        
        Physics::PhysicsSettings settings;
        settings.gravity = Vector2(0.0f, 980.0f); // 9.8 м/с² в пикселях
        m_Backend->Initialize(settings);
    }

    void Init() override {
        // Инициализация при необходимости
    }

    void Update(Registry&, float) override {
        // Обновление физики выполняется в FixedUpdate
    }

    void FixedUpdate(Registry& registry, float fixedDeltaTime) override {
        // Создаём тела для новых физических компонентов
        auto physicsViews = registry.GetAllWith<PhysicsComponent>();

        for (auto& view : physicsViews) {
            if (!view.component) {
                continue;
            }

            auto entity = view.entity;
            auto& physics = *view.component;

            if (!physics.bodyCreated) {
                m_Backend->CreateBody(entity, registry);
                physics.bodyCreated = true;
            }
        }

        // Выполняем шаг симуляции
        m_Backend->Step(registry, fixedDeltaTime);

        // Синхронизируем transform с физикой
        m_Backend->SyncTransforms(registry);
    }

    void Shutdown() override {
        m_Backend->Clear();
        m_Backend.reset();
    }

    std::string GetName() const override {
        return "PhysicsSystem";
    }

    Physics::IPhysicsBackend* GetBackend() { return m_Backend.get(); }

private:
    std::unique_ptr<Physics::IPhysicsBackend> m_Backend;
};

} // namespace SAGE::ECS
