#pragma once

#include <cstdint>
#include <limits>

namespace SAGE::ECS {

/// @brief Уникальный идентификатор сущности с версионированием
/// Младшие 32 бита - ID, старшие 32 бита - версия (защита от ABA проблемы)
using Entity = std::uint64_t;

/// @brief Невалидная сущность (используется как null-значение)
constexpr Entity NullEntity = std::numeric_limits<Entity>::max();

/// @brief Извлечь ID из Entity
inline std::uint32_t GetEntityID(Entity entity) {
    return static_cast<std::uint32_t>(entity & 0xFFFFFFFF);
}

/// @brief Извлечь версию из Entity
inline std::uint32_t GetEntityVersion(Entity entity) {
    return static_cast<std::uint32_t>((entity >> 32) & 0xFFFFFFFF);
}

/// @brief Создать Entity из ID и версии
inline Entity MakeEntity(std::uint32_t id, std::uint32_t version) {
    return (static_cast<std::uint64_t>(version) << 32) | static_cast<std::uint64_t>(id);
}

/// @brief Проверка валидности Entity
inline bool IsValid(Entity entity) {
    return entity != NullEntity && entity != 0 && GetEntityID(entity) < 0xFFFFFFFE;
}

/// @brief Проверка валидности Entity с дополнительными проверками
inline bool IsValidStrict(Entity entity) {
    return IsValid(entity) && GetEntityID(entity) > 0;
}

/// @brief Получить Entity без версии (только ID) для использования в unordered_set
/// @warning Использовать только для внутренних нужд Registry!
inline Entity GetEntityIDOnly(Entity entity) {
    return static_cast<Entity>(GetEntityID(entity));
}

} // namespace SAGE::ECS
