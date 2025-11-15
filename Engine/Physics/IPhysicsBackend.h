#pragma once

#include "Math/Vector2.h"
#include "ECS/Entity.h"
#include "Physics/PhysicsContact.h"
#include <vector>
#include <functional>

namespace SAGE { 
namespace ECS { 
    class Registry; 
    struct TransformComponent;
} 
}

namespace SAGE::Physics {

/// \brief Настройки физической симуляции
struct PhysicsSettings {
    Vector2 gravity = Vector2(0.0f, 980.0f);  ///< Гравитация (пиксели/с²)
    
    // NOTE: Box2D v3 uses subStepCount (internal parameter) instead of iterations
    // These parameters are kept for API compatibility but may not be used by all backends
    int velocityIterations = 8;                ///< [LEGACY] Итерации решателя скорости (не используется в Box2D v3)
    int positionIterations = 3;                ///< [LEGACY] Итерации решателя позиции (не используется в Box2D v3)
    
    bool enableSleeping = true;                ///< Разрешить засыпание тел
    bool enableCCD = false;                    ///< Continuous Collision Detection (предотвращает tunneling)
    float timeScale = 1.0f;                    ///< Масштаб времени (для замедления/ускорения)
};

/// \brief Результат raycast запроса
struct RaycastHit {
    ECS::Entity entity = ECS::NullEntity;
    Vector2 point = Vector2::Zero();
    Vector2 normal = Vector2::Zero();
    float fraction = 0.0f;  ///< [0-1] положение на луче
    bool hit = false;
};

/// \brief Callback для столкновений
using ContactCallback = std::function<void(const Contact&)>;

/// \brief Абстрактный интерфейс для физических движков
/// 
/// Позволяет легко заменять backend (Box2D, Bullet, кастомный и т.д.)
/// без изменения остальной кодовой базы
class IPhysicsBackend {
public:
    virtual ~IPhysicsBackend() = default;

    /// \brief Инициализация backend
    virtual void Initialize(const PhysicsSettings& settings) = 0;

    /// \brief Выполнить шаг симуляции
    /// \param registry ECS registry
    /// \param deltaTime Время кадра (секунды)
    virtual void Step(ECS::Registry& registry, float deltaTime) = 0;

    /// \brief Синхронизировать тела с ECS компонентами
    /// \param registry ECS registry
    virtual void SyncTransforms(ECS::Registry& registry) = 0;

    /// \brief Создать физическое тело для entity
    /// \param entity Entity ID
    /// \param registry ECS registry
    /// \return true если успешно создано
    virtual bool CreateBody(ECS::Entity entity, ECS::Registry& registry) = 0;

    /// \brief Удалить физическое тело
    /// \param entity Entity ID
    virtual void DestroyBody(ECS::Entity entity) = 0;

    /// \brief Raycast в физическом мире
    /// \param origin Начальная точка (пиксели)
    /// \param direction Направление (нормализованное)
    /// \param maxDistance Максимальная дистанция (пиксели)
    /// \param[out] hit Результат raycast
    /// \return true если попадание найдено
    virtual bool Raycast(const Vector2& origin, const Vector2& direction, 
                        float maxDistance, RaycastHit& hit) = 0;

    /// \brief Найти все тела в прямоугольной области
    /// \param min Минимальная точка AABB (пиксели)
    /// \param max Максимальная точка AABB (пиксели)
    /// \param[out] entities Найденные entities
    virtual void QueryAABB(const Vector2& min, const Vector2& max, 
                          std::vector<ECS::Entity>& entities) = 0;

    /// \brief Установить гравитацию
    /// \param gravity Вектор гравитации (пиксели/с²)
    virtual void SetGravity(const Vector2& gravity) = 0;

    /// \brief Получить текущую гравитацию
    /// \return Вектор гравитации (пиксели/с²)
    virtual Vector2 GetGravity() const = 0;

    /// \brief Очистить все тела
    virtual void Clear() = 0;

    /// \brief Установить callback для столкновений
    /// \param callback Функция обратного вызова
    virtual void SetContactCallback(ContactCallback callback) = 0;

    /// \brief Включить/выключить debug отрисовку
    /// \param enabled true для включения
    virtual void SetDebugDraw(bool enabled) = 0;

    /// \brief Получить список активных контактов
    /// \return Вектор контактов
    virtual const std::vector<Contact>& GetContacts() const = 0;

    /// \brief Получить название backend
    /// \return Имя движка ("Box2D", "Custom", и т.д.)
    virtual const char* GetName() const = 0;
};

} // namespace SAGE::Physics
