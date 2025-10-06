#include "PhysicsSystem.h"
#include "../Core/GameObject.h"
#include <algorithm>
#include <cmath>

namespace SAGE {

    // Статические члены
    std::vector<GameObject*> PhysicsSystem::s_Objects;
    Vector2 PhysicsSystem::s_Gravity = {0.0f, -9.8f};
    bool PhysicsSystem::s_Initialized = false;
    size_t PhysicsSystem::s_CollisionChecks = 0;

    void PhysicsSystem::Init() {
        if (s_Initialized) {
            SAGE_WARNING("PhysicsSystem уже инициализирована");
            return;
        }

        SAGE_INFO("PhysicsSystem инициализирована");
        SAGE_INFO("Гравитация: ({}, {})", s_Gravity.x, s_Gravity.y);
        s_Initialized = true;
    }

    void PhysicsSystem::Shutdown() {
        if (!s_Initialized) {
            return;
        }

        SAGE_INFO("PhysicsSystem: Очистка {} объектов", s_Objects.size());
        s_Objects.clear();
        s_Initialized = false;
        SAGE_INFO("PhysicsSystem завершена");
    }

    void PhysicsSystem::SetGravity(const Vector2& gravity) {
        s_Gravity = gravity;
        SAGE_INFO("Гравитация изменена на: ({}, {})", gravity.x, gravity.y);
    }

    Vector2 PhysicsSystem::GetGravity() {
        return s_Gravity;
    }

    void PhysicsSystem::RegisterObject(GameObject* object) {
        if (!object) return;

        auto it = std::find(s_Objects.begin(), s_Objects.end(), object);
        if (it == s_Objects.end()) {
            s_Objects.push_back(object);
        }
    }

    void PhysicsSystem::UnregisterObject(GameObject* object) {
        if (!object) return;

        auto it = std::find(s_Objects.begin(), s_Objects.end(), object);
        if (it != s_Objects.end()) {
            s_Objects.erase(it);
        }
    }

    void PhysicsSystem::Update(float /*deltaTime*/) {
        if (!s_Initialized) return;

        s_CollisionChecks = 0;

        // Обновление физики для каждого объекта
        // (Гравитация и velocity уже обрабатываются в GameObject::Update)

        // Проверка коллизий между всеми объектами
        for (size_t i = 0; i < s_Objects.size(); ++i) {
            GameObject* objA = s_Objects[i];
            if (!objA || !objA->active) continue;

            for (size_t j = i + 1; j < s_Objects.size(); ++j) {
                GameObject* objB = s_Objects[j];
                if (!objB || !objB->active) continue;

                s_CollisionChecks++;

                // Здесь можно добавить проверку коллизий между objA и objB
                // и вызвать OnCollisionEnter/OnCollisionStay/OnCollisionExit
                // Пока оставим заглушку для будущей интеграции с GameObject
            }
        }
    }

    // ============ AABB vs AABB ============

    bool PhysicsSystem::AABBvsAABB(const BoxCollider::AABB& a, const BoxCollider::AABB& b, CollisionInfo* info) {
        // Проверка пересечения
        if (a.max.x < b.min.x || a.min.x > b.max.x ||
            a.max.y < b.min.y || a.min.y > b.max.y) {
            return false;
        }

        if (info) {
            // Вычисление проникновения
            float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
            float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);

            if (overlapX < overlapY) {
                // Столкновение по X
                info->penetration = overlapX;
                info->normal = (a.min.x + a.max.x < b.min.x + b.max.x) ? Vector2{-1.0f, 0.0f} : Vector2{1.0f, 0.0f};
            } else {
                // Столкновение по Y
                info->penetration = overlapY;
                info->normal = (a.min.y + a.max.y < b.min.y + b.max.y) ? Vector2{0.0f, -1.0f} : Vector2{0.0f, 1.0f};
            }

            // Точка контакта - центр пересечения
            info->contactPoint = Vector2{
                (std::max(a.min.x, b.min.x) + std::min(a.max.x, b.max.x)) * 0.5f,
                (std::max(a.min.y, b.min.y) + std::min(a.max.y, b.max.y)) * 0.5f
            };
        }

        return true;
    }

    bool PhysicsSystem::CheckCollision(const BoxCollider& a, const Vector2& posA,
                                       const BoxCollider& b, const Vector2& posB,
                                       CollisionInfo* info) {
        auto aabbA = a.GetWorldAABB(posA);
        auto aabbB = b.GetWorldAABB(posB);
        return AABBvsAABB(aabbA, aabbB, info);
    }

    // ============ Circle vs Circle ============

    bool PhysicsSystem::CirclevsCircle(const Vector2& centerA, float radiusA,
                                       const Vector2& centerB, float radiusB,
                                       CollisionInfo* info) {
        Vector2 delta = centerB - centerA;
        float distanceSq = delta.LengthSquared();
        float radiusSum = radiusA + radiusB;

        if (distanceSq >= radiusSum * radiusSum) {
            return false;
        }

        if (info) {
            float distance = std::sqrt(distanceSq);
            info->penetration = radiusSum - distance;
            
            if (distance > 0.0001f) {
                info->normal = delta / distance;
            } else {
                info->normal = Vector2{1.0f, 0.0f}; // Fallback
            }

            info->contactPoint = centerA + info->normal * radiusA;
        }

        return true;
    }

    bool PhysicsSystem::CheckCollision(const CircleCollider& a, const Vector2& posA,
                                       const CircleCollider& b, const Vector2& posB,
                                       CollisionInfo* info) {
        Vector2 centerA = a.GetWorldCenter(posA);
        Vector2 centerB = b.GetWorldCenter(posB);
        return CirclevsCircle(centerA, a.radius, centerB, b.radius, info);
    }

    // ============ AABB vs Circle ============

    bool PhysicsSystem::AABBvsCircle(const BoxCollider::AABB& box, const Vector2& circleCenter,
                                     float radius, CollisionInfo* info) {
        // Находим ближайшую точку на AABB к центру круга
        Vector2 closestPoint;
        closestPoint.x = std::max(box.min.x, std::min(circleCenter.x, box.max.x));
        closestPoint.y = std::max(box.min.y, std::min(circleCenter.y, box.max.y));

        Vector2 delta = circleCenter - closestPoint;
        float distanceSq = delta.LengthSquared();

        if (distanceSq >= radius * radius) {
            return false;
        }

        if (info) {
            float distance = std::sqrt(distanceSq);
            info->penetration = radius - distance;

            if (distance > 0.0001f) {
                info->normal = delta / distance;
            } else {
                // Круг внутри AABB - выбираем направление выталкивания
                Vector2 boxCenter = (box.min + box.max) * 0.5f;
                Vector2 toCenter = circleCenter - boxCenter;
                
                if (std::abs(toCenter.x) > std::abs(toCenter.y)) {
                    info->normal = toCenter.x > 0 ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};
                } else {
                    info->normal = toCenter.y > 0 ? Vector2{0.0f, 1.0f} : Vector2{0.0f, -1.0f};
                }
            }

            info->contactPoint = closestPoint;
        }

        return true;
    }

    bool PhysicsSystem::CheckCollision(const BoxCollider& box, const Vector2& boxPos,
                                       const CircleCollider& circle, const Vector2& circlePos,
                                       CollisionInfo* info) {
        auto aabb = box.GetWorldAABB(boxPos);
        Vector2 circleCenter = circle.GetWorldCenter(circlePos);
        return AABBvsCircle(aabb, circleCenter, circle.radius, info);
    }

    // ============ Raycast ============

    // Вспомогательная функция: проверка пересечения луча с AABB
    static bool RaycastAABB(const Vector2& origin, const Vector2& dir, const BoxCollider::AABB& aabb, float maxDistance, float& outDistance, Vector2& outNormal) {
        // Алгоритм пересечения луча с AABB (slab method)
        Vector2 invDir = {
            std::abs(dir.x) > 0.00001f ? 1.0f / dir.x : 1e10f,
            std::abs(dir.y) > 0.00001f ? 1.0f / dir.y : 1e10f
        };

        float t1 = (aabb.min.x - origin.x) * invDir.x;
        float t2 = (aabb.max.x - origin.x) * invDir.x;
        float t3 = (aabb.min.y - origin.y) * invDir.y;
        float t4 = (aabb.max.y - origin.y) * invDir.y;

        float tmin = std::max(std::min(t1, t2), std::min(t3, t4));
        float tmax = std::min(std::max(t1, t2), std::max(t3, t4));

        // Нет пересечения если tmax < 0 или tmin > tmax
        if (tmax < 0.0f || tmin > tmax || tmin > maxDistance) {
            return false;
        }

        outDistance = tmin >= 0.0f ? tmin : tmax;
        
        // Определение нормали
        Vector2 hitPoint = origin + dir * outDistance;
        Vector2 center = (aabb.min + aabb.max) * 0.5f;
        Vector2 halfSize = (aabb.max - aabb.min) * 0.5f;
        Vector2 delta = hitPoint - center;

        // Найти ближайшую грань
        float distX = std::abs(std::abs(delta.x) - halfSize.x);
        float distY = std::abs(std::abs(delta.y) - halfSize.y);

        if (distX < distY) {
            outNormal = delta.x > 0.0f ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};
        } else {
            outNormal = delta.y > 0.0f ? Vector2{0.0f, 1.0f} : Vector2{0.0f, -1.0f};
        }

        return true;
    }

    PhysicsSystem::RaycastHit PhysicsSystem::Raycast(const Vector2& origin, const Vector2& direction, float maxDistance) {
        RaycastHit hit;
        hit.distance = maxDistance + 1.0f; // Больше максимума
        hit.hit = false;

        Vector2 dir = direction.Normalized();

        for (GameObject* obj : s_Objects) {
            if (!obj || !obj->active) continue;

            // TODO: Добавить поддержку разных типов коллайдеров через GameObject
            // Пока используем простой AABB размером 1x1 на позиции объекта
            Vector2 objPos = {obj->x, obj->y};
            BoxCollider tempBox = { .offset = {0, 0}, .size = {1.0f, 1.0f} };
            auto aabb = tempBox.GetWorldAABB(objPos);

            float distance;
            Vector2 normal;
            if (RaycastAABB(origin, dir, aabb, maxDistance, distance, normal)) {
                if (distance < hit.distance) {
                    hit.hit = true;
                    hit.object = obj;
                    hit.distance = distance;
                    hit.point = origin + dir * distance;
                    hit.normal = normal;
                }
            }
        }

        return hit;
    }

    std::vector<PhysicsSystem::RaycastHit> PhysicsSystem::RaycastAll(const Vector2& origin, const Vector2& direction, float maxDistance) {
        std::vector<RaycastHit> hits;

        Vector2 dir = direction.Normalized();

        for (GameObject* obj : s_Objects) {
            if (!obj || !obj->active) continue;

            Vector2 objPos = {obj->x, obj->y};
            BoxCollider tempBox = { .offset = {0, 0}, .size = {1.0f, 1.0f} };
            auto aabb = tempBox.GetWorldAABB(objPos);

            float distance;
            Vector2 normal;
            if (RaycastAABB(origin, dir, aabb, maxDistance, distance, normal)) {
                RaycastHit hit;
                hit.hit = true;
                hit.object = obj;
                hit.distance = distance;
                hit.point = origin + dir * distance;
                hit.normal = normal;
                hits.push_back(hit);
            }
        }

        // Сортировка по расстоянию
        std::sort(hits.begin(), hits.end(), [](const RaycastHit& a, const RaycastHit& b) {
            return a.distance < b.distance;
        });

        return hits;
    }

    // ============ Статистика ============

    size_t PhysicsSystem::GetRegisteredObjectCount() {
        return s_Objects.size();
    }

    size_t PhysicsSystem::GetCollisionChecksLastFrame() {
        return s_CollisionChecks;
    }

} // namespace SAGE
