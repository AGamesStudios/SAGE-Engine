#pragma once

#include "../Core/Core.h"
#include "../Math/Vector2.h"
#include "../Core/Logger.h"

#include <vector>
#include <functional>

namespace SAGE {

    class GameObject;

    // Типы коллайдеров
    enum class ColliderType {
        Box,    // AABB (Axis-Aligned Bounding Box)
        Circle  // Круглый коллайдер
    };

    // Информация о столкновении
    struct CollisionInfo {
        GameObject* other = nullptr;        // С кем столкнулись
        Vector2 normal = {0.0f, 0.0f};      // Нормаль столкновения
        float penetration = 0.0f;            // Глубина проникновения
        Vector2 contactPoint = {0.0f, 0.0f}; // Точка контакта
    };

    // AABB коллайдер
    struct BoxCollider {
        Vector2 offset = {0.0f, 0.0f};  // Смещение относительно GameObject
        Vector2 size = {1.0f, 1.0f};    // Размер (ширина, высота)
        bool isTrigger = false;          // Триггер (не блокирует движение)

        // Получить AABB в мировых координатах
        struct AABB {
            Vector2 min;
            Vector2 max;
        };
        
        AABB GetWorldAABB(const Vector2& position) const {
            Vector2 halfSize = size * 0.5f;
            Vector2 center = position + offset;
            return AABB{
                center - halfSize,
                center + halfSize
            };
        }
    };

    // Круглый коллайдер
    struct CircleCollider {
        Vector2 offset = {0.0f, 0.0f};  // Смещение относительно GameObject
        float radius = 0.5f;             // Радиус
        bool isTrigger = false;

        Vector2 GetWorldCenter(const Vector2& position) const {
            return position + offset;
        }
    };

    // Физическая система
    class PhysicsSystem {
    public:
        // Инициализация
        static void Init();
        static void Shutdown();

        // Обновление физики
        static void Update(float deltaTime);

        // Регистрация/удаление объектов
        static void RegisterObject(GameObject* object);
        static void UnregisterObject(GameObject* object);

        // Настройки
        static void SetGravity(const Vector2& gravity);
        static Vector2 GetGravity();

        // Проверка столкновений
        static bool CheckCollision(const BoxCollider& a, const Vector2& posA,
                                  const BoxCollider& b, const Vector2& posB,
                                  CollisionInfo* info = nullptr);

        static bool CheckCollision(const CircleCollider& a, const Vector2& posA,
                                  const CircleCollider& b, const Vector2& posB,
                                  CollisionInfo* info = nullptr);

        static bool CheckCollision(const BoxCollider& box, const Vector2& boxPos,
                                  const CircleCollider& circle, const Vector2& circlePos,
                                  CollisionInfo* info = nullptr);

        // Raycast
        struct RaycastHit {
            GameObject* object = nullptr;
            Vector2 point = {0.0f, 0.0f};
            Vector2 normal = {0.0f, 0.0f};
            float distance = 0.0f;
            bool hit = false;
        };

        static RaycastHit Raycast(const Vector2& origin, const Vector2& direction, float maxDistance = 1000.0f);
        static std::vector<RaycastHit> RaycastAll(const Vector2& origin, const Vector2& direction, float maxDistance = 1000.0f);

        // Статистика
        static size_t GetRegisteredObjectCount();
        static size_t GetCollisionChecksLastFrame();

    private:
        PhysicsSystem() = default;

        static std::vector<GameObject*> s_Objects;
        static Vector2 s_Gravity;
        static bool s_Initialized;
        static size_t s_CollisionChecks;

        // Вспомогательные функции
        static bool AABBvsAABB(const BoxCollider::AABB& a, const BoxCollider::AABB& b, CollisionInfo* info);
        static bool CirclevsCircle(const Vector2& centerA, float radiusA,
                                   const Vector2& centerB, float radiusB, CollisionInfo* info);
        static bool AABBvsCircle(const BoxCollider::AABB& box, const Vector2& circleCenter, 
                                float radius, CollisionInfo* info);
    };

} // namespace SAGE
