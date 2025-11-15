#pragma once

#include "Math/Vector2.h"
#include "ECS/Components/Core/TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace SAGE::ECS {

/// \brief Тип физического тела
enum class PhysicsBodyType {
    Static,     ///< Статическое тело - нулевая масса, неподвижно
    Kinematic,  ///< Кинематическое - нулевая масса, управляется кодом
    Dynamic     ///< Динамическое - есть масса, подчиняется физике
};

/// \brief Компонент физики для 2D движка
/// Хранит массу, скорость, силы, и параметры физического тела
struct PhysicsComponent {
    PhysicsBodyType type = PhysicsBodyType::Dynamic;  ///< Тип физического тела
    
    // Линейная физика
    Vector2 velocity = Vector2::Zero();               ///< Линейная скорость (пиксели/с)
    Vector2 forces = Vector2::Zero();                 ///< Накопленные силы (Ньютон)
    float mass = 1.0f;                                ///< Масса (кг)
    float inverseMass = 1.0f;                         ///< Обратная масса (1/масса)
    
    // Угловая физика
    float angularVelocity = 0.0f;                     ///< Угловая скорость (рад/с)
    float torque = 0.0f;                              ///< Крутящий момент
    float inertia = 1.0f;                             ///< Момент инерции
    float inverseInertia = 1.0f;                      ///< Обратный момент инерции
    
    // Зависимости компонента
    static constexpr bool RequiresTransformComponent = true;
    
    // Затухание
    float linearDamping = 0.98f;                      ///< Линейное затухание (1 = нет)
    float angularDamping = 0.98f;                     ///< Угловое затухание (1 = нет)
    
    // Трение и упругость (материал)
    float staticFriction = 0.8f;                      ///< Статическое трение
    float dynamicFriction = 0.6f;                     ///< Динамическое трение
    float restitution = 0.0f;                         ///< Упругость (0 = неупругий, 1 = идеально упругий)
    
    // Гравитация
    float gravityScale = 1.0f;                        ///< Масштаб гравитации (0 = игнорировать)
    
    // Ограничения
    bool fixedRotation = false;                       ///< Запретить вращение
    bool lockX = false;                               ///< Запретить движение по X
    bool lockY = false;                               ///< Запретить движение по Y
    
    // Центр масс
    Vector2 centerOfMass = Vector2::Zero();           ///< Смещение центра масс

    // Backend bookkeeping
    bool bodyCreated = false;                         ///< Создано ли физическое тело во внешнем движке
    
    // Оптимизация сна
    float sleepTimer = 0.0f;                          ///< Таймер покоя
    bool isSleeping = false;                          ///< В режиме сна (не обновляется)
    
    // Переопределения (для ручной настройки)
    bool massOverride = false;                        ///< Масса задана вручную
    bool inertiaOverride = false;                     ///< Инерция задана вручную

    PhysicsComponent() {
        UpdateInverseMass();
    }

    explicit PhysicsComponent(float massValue, PhysicsBodyType bodyType = PhysicsBodyType::Dynamic)
        : mass(massValue), type(bodyType), massOverride(true) {
        UpdateInverseMass();
    }

    /// \brief Установить массу
    void SetMass(float massValue, bool markOverride = true) {
        if (std::isnan(massValue) || std::isinf(massValue) || massValue < 0.0f) {
            massValue = 0.01f;
        }
        mass = std::max(0.01f, massValue);
        if (markOverride) {
            massOverride = true;
        }
        UpdateInverseMass();
    }

    /// \brief Установить тип тела
    void SetType(PhysicsBodyType bodyType) {
        type = bodyType;
        UpdateInverseMass();
        if (type == PhysicsBodyType::Static) {
            velocity = Vector2::Zero();
            angularVelocity = 0.0f;
            torque = 0.0f;
        }
    }

    /// \brief Применить силу
    void ApplyForce(const Vector2& force) {
        if (type == PhysicsBodyType::Static) return;
        forces += force;
    }

    /// \brief Применить импульс (мгновенное изменение скорости)
    void ApplyImpulse(const Vector2& impulse) {
        if (type == PhysicsBodyType::Static) return;
        velocity += impulse * inverseMass;
    }

    /// \brief Применить крутящий момент
    void ApplyTorque(float torqueValue) {
        if (type == PhysicsBodyType::Static || fixedRotation) return;
        torque += torqueValue;
    }

    /// \brief Применить угловой импульс
    void ApplyAngularImpulse(float impulse) {
        if (type == PhysicsBodyType::Static || fixedRotation) return;
        angularVelocity += impulse * inverseInertia;
    }

    /// \brief Сбросить накопленные силы
    void ClearForces() {
        forces = Vector2::Zero();
        torque = 0.0f;
    }

    /// \brief Разбудить тело
    void WakeUp() {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    /// \brief Проверить, статическое ли тело
    bool IsStatic() const {
        return type == PhysicsBodyType::Static;
    }

    /// \brief Проверить, динамическое ли тело
    bool IsDynamic() const {
        return type == PhysicsBodyType::Dynamic;
    }

private:
    void UpdateInverseMass() {
        if (type == PhysicsBodyType::Static) {
            inverseMass = 0.0f;
            inverseInertia = 0.0f;
        } else {
            inverseMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
            inverseInertia = (inertia > 0.0f) ? (1.0f / inertia) : 0.0f;
        }
    }
};

} // namespace SAGE::ECS
