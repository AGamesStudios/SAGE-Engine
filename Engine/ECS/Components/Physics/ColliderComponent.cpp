#include "ColliderComponent.h"
#include "ECS/Components/Core/TransformComponent.h"
#include <cmath>

namespace SAGE::ECS {

Vector2 ColliderComponent::GetCenter(const TransformComponent& transform) const {
    Vector2 worldOffset = m_offset;
    
    // Применяем масштаб и поворот к смещению
    if (transform.GetRotation() != 0.0f) {
        float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        float scaledX = worldOffset.x * transform.scale.x;
        float scaledY = worldOffset.y * transform.scale.y;
        worldOffset.x = scaledX * cosA - scaledY * sinA;
        worldOffset.y = scaledX * sinA + scaledY * cosA;
    } else {
        worldOffset.x *= transform.scale.x;
        worldOffset.y *= transform.scale.y;
    }
    
    return transform.position + worldOffset;
}

void ColliderComponent::GetAABB(const TransformComponent& transform, Vector2& outMin, Vector2& outMax) const {
    switch (m_type) {
        case Type::Circle: {
            float worldRadius = GetWorldRadius(transform);
            Vector2 center = GetCenter(transform);
            outMin = Vector2(center.x - worldRadius, center.y - worldRadius);
            outMax = Vector2(center.x + worldRadius, center.y + worldRadius);
            break;
        }
        
        case Type::Box: {
            std::vector<Vector2> vertices;
            GetWorldVertices(transform, vertices);
            if (vertices.empty()) {
                outMin = outMax = transform.position;
                return;
            }
            
            outMin = outMax = vertices[0];
            for (size_t i = 1; i < vertices.size(); ++i) {
                outMin.x = std::min(outMin.x, vertices[i].x);
                outMin.y = std::min(outMin.y, vertices[i].y);
                outMax.x = std::max(outMax.x, vertices[i].x);
                outMax.y = std::max(outMax.y, vertices[i].y);
            }
            break;
        }
        
        case Type::Capsule: {
            Vector2 pointA, pointB;
            GetCapsuleSegment(transform, pointA, pointB);
            float worldRadius = GetWorldRadius(transform);
            
            outMin.x = std::min(pointA.x, pointB.x) - worldRadius;
            outMin.y = std::min(pointA.y, pointB.y) - worldRadius;
            outMax.x = std::max(pointA.x, pointB.x) + worldRadius;
            outMax.y = std::max(pointA.y, pointB.y) + worldRadius;
            break;
        }
        
        case Type::Polygon: {
            std::vector<Vector2> vertices;
            GetWorldVertices(transform, vertices);
            if (vertices.empty()) {
                outMin = outMax = transform.position;
                return;
            }
            
            outMin = outMax = vertices[0];
            for (size_t i = 1; i < vertices.size(); ++i) {
                outMin.x = std::min(outMin.x, vertices[i].x);
                outMin.y = std::min(outMin.y, vertices[i].y);
                outMax.x = std::max(outMax.x, vertices[i].x);
                outMax.y = std::max(outMax.y, vertices[i].y);
            }
            break;
        }
        
        case Type::Compound: {
            // Для составного коллайдера объединяем AABB всех подколлайдеров
            bool first = true;
            for (const auto& sub : m_subColliders) {
                Vector2 subMin, subMax;
                // Создаем временный transform для подколлайдера
                TransformComponent subTransform = transform;
                Vector2 localOffset = sub.offset;
                
                // Применяем поворот родителя к смещению
                if (transform.GetRotation() != 0.0f) {
                    float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
                    float cosA = std::cos(angleRad);
                    float sinA = std::sin(angleRad);
                    float scaledX = localOffset.x * transform.scale.x;
                    float scaledY = localOffset.y * transform.scale.y;
                    localOffset.x = scaledX * cosA - scaledY * sinA;
                    localOffset.y = scaledX * sinA + scaledY * cosA;
                } else {
                    localOffset.x *= transform.scale.x;
                    localOffset.y *= transform.scale.y;
                }
                
                subTransform.position += localOffset;
                subTransform.SetRotation(transform.GetRotation() + sub.rotation);
                
                // Вычисляем AABB в зависимости от типа подколлайдера
                switch (sub.type) {
                    case Type::Circle: {
                        float radius = sub.radius * std::max(subTransform.scale.x, subTransform.scale.y);
                        subMin = Vector2(subTransform.position.x - radius, subTransform.position.y - radius);
                        subMax = Vector2(subTransform.position.x + radius, subTransform.position.y + radius);
                        break;
                    }
                    case Type::Box: {
                        // Упрощенная версия для Box в compound
                        float hw = sub.size.x * 0.5f * subTransform.scale.x;
                        float hh = sub.size.y * 0.5f * subTransform.scale.y;
                        float maxHalf = std::max(hw, hh);
                        subMin = Vector2(subTransform.position.x - maxHalf, subTransform.position.y - maxHalf);
                        subMax = Vector2(subTransform.position.x + maxHalf, subTransform.position.y + maxHalf);
                        break;
                    }
                    default:
                        subMin = subMax = subTransform.position;
                        break;
                }
                
                if (first) {
                    outMin = subMin;
                    outMax = subMax;
                    first = false;
                } else {
                    outMin.x = std::min(outMin.x, subMin.x);
                    outMin.y = std::min(outMin.y, subMin.y);
                    outMax.x = std::max(outMax.x, subMax.x);
                    outMax.y = std::max(outMax.y, subMax.y);
                }
            }
            
            if (first) {
                outMin = outMax = transform.position;
            }
            break;
        }
    }
}

void ColliderComponent::GetWorldVertices(const TransformComponent& transform, std::vector<Vector2>& outVertices) const {
    outVertices.clear();
    
    switch (m_type) {
        case Type::Box: {
            outVertices.resize(4);
            
            float hw = m_data.box.width * 0.5f;
            float hh = m_data.box.height * 0.5f;
            
            // Локальные вершины
            Vector2 localVerts[4] = {
                Vector2(-hw, -hh),
                Vector2( hw, -hh),
                Vector2( hw,  hh),
                Vector2(-hw,  hh)
            };
            
            float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
            float cosA = std::cos(angleRad);
            float sinA = std::sin(angleRad);
            
            Vector2 center = GetCenter(transform);
            
            for (int i = 0; i < 4; ++i) {
                float scaledX = localVerts[i].x * transform.scale.x;
                float scaledY = localVerts[i].y * transform.scale.y;
                
                outVertices[i].x = center.x + (scaledX * cosA - scaledY * sinA);
                outVertices[i].y = center.y + (scaledX * sinA + scaledY * cosA);
            }
            break;
        }
        
        case Type::Polygon: {
            if (m_polygonVertices.empty()) break;
            
            outVertices.resize(m_polygonVertices.size());
            
            float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
            float cosA = std::cos(angleRad);
            float sinA = std::sin(angleRad);
            
            Vector2 center = GetCenter(transform);
            
            for (size_t i = 0; i < m_polygonVertices.size(); ++i) {
                float scaledX = m_polygonVertices[i].x * transform.scale.x;
                float scaledY = m_polygonVertices[i].y * transform.scale.y;
                
                outVertices[i].x = center.x + (scaledX * cosA - scaledY * sinA);
                outVertices[i].y = center.y + (scaledX * sinA + scaledY * cosA);
            }
            break;
        }
        
        default:
            // Circle, Capsule, Compound не имеют вершин в обычном смысле
            break;
    }
}

void ColliderComponent::GetCapsuleSegment(const TransformComponent& transform, Vector2& outA, Vector2& outB) const {
    if (m_type != Type::Capsule) {
        outA = outB = transform.position;
        return;
    }
    
    Vector2 center = GetCenter(transform);
    
    // Получаем мировое направление оси
    float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);
    
    float localAxisX = m_data.capsule.axisX;
    float localAxisY = m_data.capsule.axisY;
    
    float worldAxisX = localAxisX * cosA - localAxisY * sinA;
    float worldAxisY = localAxisX * sinA + localAxisY * cosA;
    
    // Применяем масштаб к высоте
    float scaledHeight = m_data.capsule.height * std::max(transform.scale.x, transform.scale.y);
    float halfHeight = scaledHeight * 0.5f;
    
    outA = Vector2(center.x - worldAxisX * halfHeight, center.y - worldAxisY * halfHeight);
    outB = Vector2(center.x + worldAxisX * halfHeight, center.y + worldAxisY * halfHeight);
}

float ColliderComponent::GetWorldRadius(const TransformComponent& transform) const {
    float radius = 0.0f;
    
    switch (m_type) {
        case Type::Circle:
            radius = m_data.circle.radius;
            break;
        case Type::Capsule:
            radius = m_data.capsule.radius;
            break;
        default:
            return 0.0f;
    }
    
    // Применяем максимальный масштаб
    return radius * std::max(transform.scale.x, transform.scale.y);
}

bool ColliderComponent::ContainsPoint(const Vector2& point, const TransformComponent& transform) const {
    switch (m_type) {
        case Type::Circle: {
            Vector2 center = GetCenter(transform);
            float worldRadius = GetWorldRadius(transform);
            float dx = point.x - center.x;
            float dy = point.y - center.y;
            float distSq = dx * dx + dy * dy;
            return distSq <= (worldRadius * worldRadius);
        }
        
        case Type::Box: {
            // Трансформируем точку в локальное пространство коллайдера
            Vector2 center = GetCenter(transform);
            float dx = point.x - center.x;
            float dy = point.y - center.y;
            
            // Обратный поворот
            float angleRad = -transform.GetRotation() * 3.14159265359f / 180.0f;
            float cosA = std::cos(angleRad);
            float sinA = std::sin(angleRad);
            float localX = dx * cosA - dy * sinA;
            float localY = dx * sinA + dy * cosA;
            
            // Учитываем масштаб
            localX /= transform.scale.x;
            localY /= transform.scale.y;
            
            // Проверяем локальные границы
            float hw = m_data.box.width * 0.5f;
            float hh = m_data.box.height * 0.5f;
            return (std::abs(localX) <= hw) && (std::abs(localY) <= hh);
        }
        
        case Type::Capsule: {
            Vector2 pointA, pointB;
            GetCapsuleSegment(transform, pointA, pointB);
            float worldRadius = GetWorldRadius(transform);
            
            // Ближайшая точка на отрезке AB к точке point
            float dx = pointB.x - pointA.x;
            float dy = pointB.y - pointA.y;
            float lenSq = dx * dx + dy * dy;
            
            float t = 0.5f; // По умолчанию центр
            if (lenSq > 1e-6f) {
                float px = point.x - pointA.x;
                float py = point.y - pointA.y;
                t = (px * dx + py * dy) / lenSq;
                t = std::clamp(t, 0.0f, 1.0f);
            }
            
            Vector2 closest(pointA.x + t * dx, pointA.y + t * dy);
            float distX = point.x - closest.x;
            float distY = point.y - closest.y;
            float distSq = distX * distX + distY * distY;
            
            return distSq <= (worldRadius * worldRadius);
        }
        
        case Type::Polygon: {
            if (m_polygonVertices.empty()) return false;
            
            std::vector<Vector2> worldVerts;
            GetWorldVertices(transform, worldVerts);
            if (worldVerts.size() < 3) return false;
            
            // Ray casting algorithm
            int crossings = 0;
            for (size_t i = 0; i < worldVerts.size(); ++i) {
                size_t j = (i + 1) % worldVerts.size();
                
                if (((worldVerts[i].y > point.y) != (worldVerts[j].y > point.y)) &&
                    (point.x < (worldVerts[j].x - worldVerts[i].x) * (point.y - worldVerts[i].y) /
                               (worldVerts[j].y - worldVerts[i].y) + worldVerts[i].x)) {
                    crossings++;
                }
            }
            
            return (crossings % 2) == 1;
        }
        
        case Type::Compound: {
            // Проверяем каждый подколлайдер
            for (const auto& sub : m_subColliders) {
                // Создаем временный transform для подколлайдера
                TransformComponent subTransform = transform;
                Vector2 localOffset = sub.offset;
                
                // Применяем поворот родителя к смещению
                if (transform.GetRotation() != 0.0f) {
                    float angleRad = transform.GetRotation() * 3.14159265359f / 180.0f;
                    float cosA = std::cos(angleRad);
                    float sinA = std::sin(angleRad);
                    float scaledX = localOffset.x * transform.scale.x;
                    float scaledY = localOffset.y * transform.scale.y;
                    localOffset.x = scaledX * cosA - scaledY * sinA;
                    localOffset.y = scaledX * sinA + scaledY * cosA;
                } else {
                    localOffset.x *= transform.scale.x;
                    localOffset.y *= transform.scale.y;
                }
                
                subTransform.position += localOffset;
                subTransform.SetRotation(transform.GetRotation() + sub.rotation);
                
                // Создаем временный коллайдер нужного типа и проверяем
                switch (sub.type) {
                    case Type::Circle: {
                        auto tempCollider = CreateCircle(sub.radius);
                        if (tempCollider.ContainsPoint(point, subTransform)) return true;
                        break;
                    }
                    case Type::Box: {
                        auto tempCollider = CreateBox(sub.size);
                        if (tempCollider.ContainsPoint(point, subTransform)) return true;
                        break;
                    }
                    default:
                        break;
                }
            }
            
            return false;
        }
    }
    
    return false;
}

float ColliderComponent::GetBoundingRadius(const TransformComponent& transform) const {
    switch (m_type) {
        case Type::Circle:
            return GetWorldRadius(transform);
        
        case Type::Box: {
            // Диагональ прямоугольника / 2
            float scaledWidth = m_data.box.width * transform.scale.x;
            float scaledHeight = m_data.box.height * transform.scale.y;
            return std::sqrt(scaledWidth * scaledWidth + scaledHeight * scaledHeight) * 0.5f;
        }
        
        case Type::Capsule: {
            float worldRadius = GetWorldRadius(transform);
            float scaledHeight = m_data.capsule.height * std::max(transform.scale.x, transform.scale.y);
            float halfHeight = scaledHeight * 0.5f;
            return worldRadius + halfHeight;
        }
        
        case Type::Polygon: {
            if (m_polygonVertices.empty()) return 0.0f;
            
            // Максимальное расстояние от центра до вершины
            float maxDistSq = 0.0f;
            for (const auto& vert : m_polygonVertices) {
                float scaledX = vert.x * transform.scale.x;
                float scaledY = vert.y * transform.scale.y;
                float distSq = scaledX * scaledX + scaledY * scaledY;
                maxDistSq = std::max(maxDistSq, distSq);
            }
            return std::sqrt(maxDistSq);
        }
        
        case Type::Compound: {
            // Максимальный радиус среди подколлайдеров + смещения
            float maxRadius = 0.0f;
            for (const auto& sub : m_subColliders) {
                float subRadius = 0.0f;
                switch (sub.type) {
                    case Type::Circle:
                        subRadius = sub.radius;
                        break;
                    case Type::Box: {
                        float diag = std::sqrt(sub.size.x * sub.size.x + sub.size.y * sub.size.y);
                        subRadius = diag * 0.5f;
                        break;
                    }
                    default:
                        break;
                }
                
                float offsetLen = std::sqrt(sub.offset.x * sub.offset.x + sub.offset.y * sub.offset.y);
                float totalRadius = (offsetLen + subRadius) * std::max(transform.scale.x, transform.scale.y);
                maxRadius = std::max(maxRadius, totalRadius);
            }
            return maxRadius;
        }
    }
    
    return 0.0f;
}

} // namespace SAGE::ECS
