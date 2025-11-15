#pragma once

#include "Math/Vector2.h"
#include "ECS/Entity.h"

#include <vector>

namespace SAGE::Physics {

/// \brief Представление контакта, обнаруженного SAGE Physics за один симуляционный шаг.
struct Contact {
    ECS::Entity entityA = ECS::NullEntity; ///< Первая сущность в контакте
    ECS::Entity entityB = ECS::NullEntity; ///< Вторая сущность в контакте
    Vector2 normal = Vector2::Zero();      ///< Нормаль, направленная от A к B
    Vector2 relativeVelocity = Vector2::Zero(); ///< Относительная скорость (B - A) вдоль контакта
    float penetration = 0.0f;              ///< Глубина проникновения в мировых единицах
    bool isTrigger = false;                ///< True, если хотя бы один коллайдер был триггером
    bool resolved = false;                 ///< True, если физика применила позиционную/импульсную коррекцию
    std::vector<Vector2> contactPoints;    ///< Точки соприкосновения в мировых координатах (может быть пустым)
};

} // namespace SAGE::Physics
