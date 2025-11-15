#pragma once

#include "Math/Vector2.h"
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace SAGE::ECS {

struct TransformComponent;

/// \brief Универсальный компонент коллайдера с поддержкой всех типов
class ColliderComponent {
public:
    /// \brief Типы коллайдеров
    enum class Type {
        Circle,   ///< Круглый коллайдер (радиус)
        Box,      ///< Прямоугольный коллайдер (размер)
        Capsule,  ///< Капсульный коллайдер (радиус + высота + ось)
        Polygon,  ///< Полигональный коллайдер (произвольные вершины)
        Compound  ///< Составной коллайдер (несколько подколлайдеров)
    };

    /// \brief Подколлайдер для Compound типа
    struct SubCollider {
        Type type = Type::Box;
        Vector2 offset = Vector2::Zero();
        float rotation = 0.0f;
        
        // Данные для разных типов
        Vector2 size = Vector2(32.0f, 32.0f);  // Box
        float radius = 16.0f;                   // Circle, Capsule
        float height = 32.0f;                   // Capsule
        Vector2 axis = Vector2(0.0f, 1.0f);     // Capsule
        std::vector<Vector2> vertices;          // Polygon
        
        float density = 1.0f;
        bool isTrigger = false;
    };

private:
    Type m_type = Type::Box;
    Vector2 m_offset = Vector2::Zero();
    bool m_isTrigger = false;
    
    // Физические материалы
    float m_density = 1.0f;      ///< Плотность материала (кг/м²)
    float m_friction = 0.5f;     ///< Коэффициент трения (0 = скользко, 1 = липко)
    float m_restitution = 0.3f;  ///< Коэффициент упругости (0 = не отскакивает, 1 = резиновый)

    // Данные для каждого типа коллайдера
    union ColliderData {
        struct {
            float radius;
        } circle;
        
        struct {
            float width;
            float height;
        } box;
        
        struct {
            float radius;
            float height;
            float axisX;
            float axisY;
        } capsule;
        
        ColliderData() : circle{16.0f} {}
    } m_data;
    
    // Polygon и Compound используют динамические данные
    std::vector<Vector2> m_polygonVertices;
    std::vector<SubCollider> m_subColliders;
    bool m_autoCenter = true;  // Для Compound
    Vector2 m_cachedCenter = Vector2::Zero();

public:
    // ============ Конструкторы ============
    
    ColliderComponent() : m_type(Type::Box) {
        m_data.box.width = 32.0f;
        m_data.box.height = 32.0f;
    }

    /// \brief Создать круглый коллайдер
    static ColliderComponent CreateCircle(float radius, const Vector2& offset = Vector2::Zero(), 
                                         bool isTrigger = false, float friction = 0.5f, 
                                         float restitution = 0.3f, float density = 1.0f) {
        ColliderComponent collider;
        collider.m_type = Type::Circle;
        collider.m_offset = offset;
        collider.m_isTrigger = isTrigger;
        collider.m_friction = friction;
        collider.m_restitution = restitution;
        collider.m_density = density;
        collider.SetCircleRadius(radius);
        return collider;
    }

    /// \brief Создать прямоугольный коллайдер
    static ColliderComponent CreateBox(const Vector2& size, const Vector2& offset = Vector2::Zero(), 
                                      bool isTrigger = false, float friction = 0.5f, 
                                      float restitution = 0.3f, float density = 1.0f) {
        ColliderComponent collider;
        collider.m_type = Type::Box;
        collider.m_offset = offset;
        collider.m_isTrigger = isTrigger;
        collider.m_friction = friction;
        collider.m_restitution = restitution;
        collider.m_density = density;
        collider.SetBoxSize(size);
        return collider;
    }

    /// \brief Создать капсульный коллайдер
    static ColliderComponent CreateCapsule(float radius, float height, const Vector2& axis = Vector2(0.0f, 1.0f),
                                          const Vector2& offset = Vector2::Zero(), bool isTrigger = false,
                                          float friction = 0.4f, float restitution = 0.2f, float density = 0.9f) {
        ColliderComponent collider;
        collider.m_type = Type::Capsule;
        collider.m_offset = offset;
        collider.m_isTrigger = isTrigger;
        collider.m_friction = friction;
        collider.m_restitution = restitution;
        collider.m_density = density;
        collider.SetCapsuleRadius(radius);
        collider.SetCapsuleHeight(height);
        collider.SetCapsuleAxis(axis);
        return collider;
    }

    /// \brief Создать полигональный коллайдер
    static ColliderComponent CreatePolygon(const std::vector<Vector2>& vertices, const Vector2& offset = Vector2::Zero(), 
                                          bool isTrigger = false) {
        ColliderComponent collider;
        collider.m_type = Type::Polygon;
        collider.m_offset = offset;
        collider.m_isTrigger = isTrigger;
        collider.SetPolygonVertices(vertices);
        return collider;
    }

    /// \brief Создать составной коллайдер
    static ColliderComponent CreateCompound(const std::vector<SubCollider>& subColliders, bool autoCenter = true,
                                           const Vector2& offset = Vector2::Zero(), bool isTrigger = false) {
        ColliderComponent collider;
        collider.m_type = Type::Compound;
        collider.m_offset = offset;
        collider.m_isTrigger = isTrigger;
        collider.m_autoCenter = autoCenter;
        collider.m_subColliders = subColliders;
        collider.UpdateCompoundCenter();
        return collider;
    }
    
    // ============ Удобные фабрики для типичных форм ============
    
    /// \brief Создать квадратный коллайдер
    static ColliderComponent CreateSquare(float size, const Vector2& offset = Vector2::Zero(), 
                                         bool isTrigger = false, float friction = 0.5f, 
                                         float restitution = 0.3f, float density = 1.0f) {
        return CreateBox(Vector2(size, size), offset, isTrigger, friction, restitution, density);
    }
    
    /// \brief Создать прямоугольник с указанной шириной и высотой
    static ColliderComponent CreateRectangle(float width, float height, const Vector2& offset = Vector2::Zero(),
                                            bool isTrigger = false, float friction = 0.5f,
                                            float restitution = 0.3f, float density = 1.0f) {
        return CreateBox(Vector2(width, height), offset, isTrigger, friction, restitution, density);
    }
    
    /// \brief Создать коллайдер игрока (капсула 32x64 по умолчанию)
    static ColliderComponent CreatePlayer(float radius = 16.0f, float height = 32.0f, 
                                         const Vector2& offset = Vector2(0.0f, 0.0f)) {
        return CreateCapsule(radius, height, Vector2(0.0f, 1.0f), offset, false, 0.4f, 0.0f, 0.9f);
    }
    
    /// \brief Создать круглый триггер (для зон подбора предметов, чекпоинтов)
    static ColliderComponent CreateTriggerCircle(float radius, const Vector2& offset = Vector2::Zero()) {
        return CreateCircle(radius, offset, true, 0.0f, 0.0f, 1.0f);
    }
    
    /// \brief Создать прямоугольный триггер
    static ColliderComponent CreateTriggerBox(const Vector2& size, const Vector2& offset = Vector2::Zero()) {
        return CreateBox(size, offset, true, 0.0f, 0.0f, 1.0f);
    }
    
    /// \brief Создать коллайдер мяча (высокая упругость, низкое трение)
    static ColliderComponent CreateBall(float radius, const Vector2& offset = Vector2::Zero()) {
        return CreateCircle(radius, offset, false, 0.3f, 0.7f, 0.8f);
    }
    
    /// \brief Создать коллайдер стены (статичный, высокое трение)
    static ColliderComponent CreateWall(const Vector2& size, const Vector2& offset = Vector2::Zero()) {
        return CreateBox(size, offset, false, 0.7f, 0.3f, 1.0f);
    }

    // ============ Геттеры типа ============
    
    Type GetType() const { return m_type; }
    bool IsCircle() const { return m_type == Type::Circle; }
    bool IsBox() const { return m_type == Type::Box; }
    bool IsCapsule() const { return m_type == Type::Capsule; }
    bool IsPolygon() const { return m_type == Type::Polygon; }
    bool IsCompound() const { return m_type == Type::Compound; }
    
    // ============ Общие свойства ============
    
    const Vector2& GetOffset() const { return m_offset; }
    void SetOffset(const Vector2& offset) { m_offset = offset; }
    
    bool IsTrigger() const { return m_isTrigger; }
    void SetTrigger(bool trigger) { m_isTrigger = trigger; }
    
    // Физические материалы
    float GetDensity() const { return m_density; }
    void SetDensity(float density) { m_density = std::max(0.01f, density); }
    
    float GetFriction() const { return m_friction; }
    void SetFriction(float friction) { m_friction = std::clamp(friction, 0.0f, 1.0f); }
    
    float GetRestitution() const { return m_restitution; }
    void SetRestitution(float restitution) { m_restitution = std::clamp(restitution, 0.0f, 1.0f); }

    // ============ Circle данные ============
    
    float GetCircleRadius() const {
        if (m_type != Type::Circle) {
            std::fprintf(stderr, "[ColliderComponent] GetCircleRadius called on non-Circle type\n");
            return 0.0f;
        }
        return m_data.circle.radius;
    }
    
    void SetCircleRadius(float radius) {
        if (m_type != Type::Circle) {
            std::fprintf(stderr, "[ColliderComponent] SetCircleRadius called on non-Circle type\n");
            return;
        }
        if (radius <= 0.0f) {
            std::fprintf(stderr, "[ColliderComponent] Invalid circle radius %.2f, clamping to 0.1f\n", radius);
            radius = 0.1f;
        }
        m_data.circle.radius = std::max(0.1f, radius);
    }

    // ============ Box данные ============
    
    Vector2 GetBoxSize() const {
        if (m_type != Type::Box) {
            std::fprintf(stderr, "[ColliderComponent] GetBoxSize called on non-Box type\n");
            return Vector2::Zero();
        }
        return Vector2(m_data.box.width, m_data.box.height);
    }
    
    void SetBoxSize(const Vector2& size) {
        if (m_type != Type::Box) {
            std::fprintf(stderr, "[ColliderComponent] SetBoxSize called on non-Box type\n");
            return;
        }
        if (size.x <= 0.0f || size.y <= 0.0f) {
            std::fprintf(stderr, "[ColliderComponent] Invalid box size (%.2f, %.2f), clamping to 0.1f\n", size.x, size.y);
        }
        m_data.box.width = std::max(0.1f, size.x);
        m_data.box.height = std::max(0.1f, size.y);
    }
    
    Vector2 GetBoxHalfSize() const {
        return GetBoxSize() * 0.5f;
    }

    // ============ Capsule данные ============
    
    float GetCapsuleRadius() const {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] GetCapsuleRadius called on non-Capsule type\n");
            return 0.0f;
        }
        return m_data.capsule.radius;
    }
    
    void SetCapsuleRadius(float radius) {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] SetCapsuleRadius called on non-Capsule type\n");
            return;
        }
        if (radius <= 0.0f) {
            std::fprintf(stderr, "[ColliderComponent] Invalid capsule radius %.2f, clamping to 0.1f\n", radius);
            radius = 0.1f;
        }
        m_data.capsule.radius = std::max(0.1f, radius);
    }
    
    float GetCapsuleHeight() const {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] GetCapsuleHeight called on non-Capsule type\n");
            return 0.0f;
        }
        return m_data.capsule.height;
    }
    
    void SetCapsuleHeight(float height) {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] SetCapsuleHeight called on non-Capsule type\n");
            return;
        }
        if (height <= 0.0f) {
            std::fprintf(stderr, "[ColliderComponent] Invalid capsule height %.2f, clamping to 0.1f\n", height);
            height = 0.1f;
        }
        m_data.capsule.height = std::max(0.1f, height);
    }
    
    Vector2 GetCapsuleAxis() const {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] GetCapsuleAxis called on non-Capsule type\n");
            return Vector2(0.0f, 1.0f);
        }
        return Vector2(m_data.capsule.axisX, m_data.capsule.axisY);
    }
    
    void SetCapsuleAxis(const Vector2& axis) {
        if (m_type != Type::Capsule) {
            std::fprintf(stderr, "[ColliderComponent] SetCapsuleAxis called on non-Capsule type\n");
            return;
        }
        Vector2 normalized = NormalizeAxis(axis);
        m_data.capsule.axisX = normalized.x;
        m_data.capsule.axisY = normalized.y;
    }

    // ============ Polygon данные ============
    
    const std::vector<Vector2>& GetPolygonVertices() const {
        if (m_type != Type::Polygon) {
            std::fprintf(stderr, "[ColliderComponent] GetPolygonVertices called on non-Polygon type\n");
        }
        return m_polygonVertices;
    }
    
    void SetPolygonVertices(const std::vector<Vector2>& vertices) {
        if (m_type != Type::Polygon) {
            std::fprintf(stderr, "[ColliderComponent] SetPolygonVertices called on non-Polygon type\n");
            return;
        }
        if (vertices.size() < 3) {
            std::fprintf(stderr, "[ColliderComponent] Polygon must have at least 3 vertices, got %zu\n", vertices.size());
            return;
        }
        m_polygonVertices = vertices;
    }

    // ============ Compound данные ============
    
    const std::vector<SubCollider>& GetSubColliders() const {
        if (m_type != Type::Compound) {
            std::fprintf(stderr, "[ColliderComponent] GetSubColliders called on non-Compound type\n");
        }
        return m_subColliders;
    }
    
    void SetSubColliders(const std::vector<SubCollider>& subColliders) {
        if (m_type != Type::Compound) {
            std::fprintf(stderr, "[ColliderComponent] SetSubColliders called on non-Compound type\n");
            return;
        }
        m_subColliders = subColliders;
        UpdateCompoundCenter();
    }
    
    void AddSubCollider(const SubCollider& subCollider) {
        if (m_type != Type::Compound) {
            std::fprintf(stderr, "[ColliderComponent] AddSubCollider called on non-Compound type\n");
            return;
        }
        m_subColliders.push_back(subCollider);
        UpdateCompoundCenter();
    }
    
    bool GetAutoCenter() const { return m_autoCenter; }
    void SetAutoCenter(bool autoCenter) { 
        m_autoCenter = autoCenter;
        if (autoCenter && m_type == Type::Compound) {
            UpdateCompoundCenter();
        }
    }
    
    const Vector2& GetCompoundCenter() const { return m_cachedCenter; }

    // ============ Геометрические методы ============
    
    /// \brief Получить центр коллайдера в мировых координатах
    Vector2 GetCenter(const TransformComponent& transform) const;
    
    /// \brief Получить AABB коллайдера в мировых координатах
    void GetAABB(const TransformComponent& transform, Vector2& outMin, Vector2& outMax) const;
    
    /// \brief Получить мировые вершины (для Box и Polygon)
    void GetWorldVertices(const TransformComponent& transform, std::vector<Vector2>& outVertices) const;
    
    /// \brief Получить сегмент капсулы в мировых координатах
    void GetCapsuleSegment(const TransformComponent& transform, Vector2& outA, Vector2& outB) const;
    
    /// \brief Получить мировой радиус (для Circle и Capsule)
    float GetWorldRadius(const TransformComponent& transform) const;
    
    /// \brief Проверить, содержится ли точка внутри коллайдера
    bool ContainsPoint(const Vector2& point, const TransformComponent& transform) const;
    
    /// \brief Получить ограничивающий радиус (для broad-phase оптимизации)
    float GetBoundingRadius(const TransformComponent& transform) const;

private:
    /// \brief Нормализовать вектор оси
    static Vector2 NormalizeAxis(const Vector2& axis) {
        float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
        constexpr float kEpsilon = 1e-6f;
        if (len < kEpsilon) {
            std::fprintf(stderr, "[ColliderComponent] Zero axis, using (0,1)\n");
            return Vector2(0.0f, 1.0f);
        }
        return Vector2(axis.x / len, axis.y / len);
    }
    
    /// \brief Обновить центр для Compound коллайдера
    void UpdateCompoundCenter() {
        if (m_type != Type::Compound || !m_autoCenter || m_subColliders.empty()) {
            m_cachedCenter = Vector2::Zero();
            return;
        }
        
        Vector2 sum = Vector2::Zero();
        for (const auto& sub : m_subColliders) {
            sum += sub.offset;
        }
        m_cachedCenter = sum / static_cast<float>(m_subColliders.size());
    }
};

} // namespace SAGE::ECS
