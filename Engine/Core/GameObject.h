#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_set>

#include "Color.h"
#include "../Math/Vector2.h"

namespace SAGE {

    class GameObject {
    public:
        // 🏷️ Базовая информация
        std::string name;
        bool active = true;
        int layer = 0;
        
        // 📍 Позиция и размер
        float x = 0;
        float y = 0;
        float angle = 0;        // Угол поворота (градусы)
        float width = 32;       // Ширина
        float height = 32;      // Высота
        
        // 🏃 Скорость
        float speedX = 0;
        float speedY = 0;
        
        // ⚙️ Физика
        float gravity = 0;      // Гравитация (пиксели/сек²)
        float friction = 0;     // Трение (0-1)
        float bounce = 0;       // Отскок (0-1)
        bool physics = false;   // Включить физику
    float maxFallSpeed = 1500.0f; // Ограничение скорости падения
        float mass = 1.0f;        // Масса для расчёта сил
        float gravityScale = 1.0f;// Множитель гравитации
        float jumpStrength = 650.0f; // Стандартная сила прыжка
        float coyoteTime = 0.08f;    // Время на лесу
        float jumpBuffer = 0.1f;     // Время буфера прыжка
        
        // 🎨 Визуал
        std::string image;      // Путь к картинке
        Color color;            // Цвет
        float alpha = 1.0f;     // Прозрачность (0-1)
        bool visible = true;    // Видимый
        bool flipX = false;     // Отзеркалить по X
        bool flipY = false;     // Отзеркалить по Y
        
        // 💥 Коллизия
        bool collision = false; // Включить столкновения
        bool solid = true;      // Твёрдый (блокирует движение)
        std::string hitboxType = "box"; // "box" или "circle"
        
        // 🎯 События (лямбды)
    std::function<void()> OnCreate;
    std::function<void(float)> OnUpdate;
        std::function<void(GameObject*)> OnCollision;          // Вызов при входе в коллизию (совместимость)
        std::function<void(GameObject*)> OnCollisionEnter;     // Альтернатива OnCollision: событие начала контакта
        std::function<void(GameObject*)> OnCollisionStay;      // Срабатывает каждый кадр, пока есть контакт
        std::function<void(GameObject*)> OnCollisionExit;      // Срабатывает при выходе из контакта
        std::function<void()> OnDestroy;
        
        // 🛠️ Методы
        void MoveTo(float newX, float newY);
        void MoveBy(float deltaX, float deltaY);
        void Jump(float force);
        void Push(float forceX, float forceY);
        void Stop();
        void Destroy();
        bool IsGrounded() const { return grounded || wasGroundedLastFrame; }
        bool IsGroundedStrict() const { return grounded; }
        void SetMass(float newMass);
        void SetGravityScale(float scale) { gravityScale = scale; }
        void SetJumpStrength(float strength) { jumpStrength = strength; }
        void SetCoyoteTime(float seconds);
        void SetJumpBuffer(float seconds);
        void ApplyForce(const Vector2& force);
        void ApplyImpulse(const Vector2& impulse);
        void ClearForces();
        Vector2 GetVelocity() const { return Vector2(speedX, speedY); }
        void SetVelocity(const Vector2& velocity);
        void Jump();
        
        bool IsTouching(GameObject* other);
        bool IsOnScreen();
        
        // 📊 Статические методы (управление всеми объектами)
        static GameObject* Create(const std::string& name);
        static void UpdateAll(float deltaTime);
        static void RenderAll();
        static void DestroyMarked();
        static void DestroyAll();
        static GameObject* Find(const std::string& name);
        static std::vector<GameObject*> FindAll(const std::string& name);
        static int Count();
        
    private:
        bool markedForDestruction = false;
        bool grounded = false;
        float prevX = 0;
        float prevY = 0;
        
    float inverseMass = 1.0f;
    Vector2 accumulatedForces = Vector2::Zero();
    float coyoteTimer = 0.0f;
    float jumpBufferTimer = 0.0f;
    float pendingJumpVelocity = 0.0f;
    bool jumpQueued = false;
    bool wasGroundedLastFrame = false;

    void BeginPhysicsStep(float deltaTime);
    void HandleJumpRequest();
        void UpdatePhysics(float deltaTime);
        void UpdatePosition(float deltaTime);
        void CheckCollisions(float deltaTime);
        void ResolveCollision(GameObject* other, float overlapX, float overlapY);
        void ClearCollision(GameObject* other);
        
        static std::vector<GameObject*> allObjects;
        static std::vector<GameObject*> objectsToDestroy;
        std::unordered_set<GameObject*> currentContacts;
    };

} // namespace SAGE
