#include "GameObject.h"
#include "Logger.h"
#include "Application.h"
#include "../Graphics/Renderer.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace SAGE {

    namespace {
        constexpr float DEFAULT_GRAVITY = 1200.0f;
    }

    // Статические переменные
    std::vector<GameObject*> GameObject::allObjects;
    std::vector<GameObject*> GameObject::objectsToDestroy;

    // Создание объекта
    GameObject* GameObject::Create(const std::string& name) {
        GameObject* obj = new GameObject();
        obj->name = name;
        obj->color = Color::White();
        obj->prevX = obj->x;
        obj->prevY = obj->y;
        allObjects.push_back(obj);
        
        // Вызвать OnCreate если установлен
        if (obj->OnCreate) {
            obj->OnCreate();
        }
        
        SAGE_INFO("GameObject created: {0}", name);
        return obj;
    }

    // Обновление всех объектов
    void GameObject::UpdateAll(float deltaTime) {
        // Обновить физику и позицию
        for (auto* obj : allObjects) {
            if (!obj->active) continue;
            obj->BeginPhysicsStep(deltaTime);
            
            // Физика
            if (obj->physics) {
                obj->UpdatePhysics(deltaTime);
            }
            
            // Позиция
            obj->UpdatePosition(deltaTime);
            
            // Пользовательское обновление
            if (obj->OnUpdate) {
                obj->OnUpdate(deltaTime);
            }
        }
        
        // Проверка коллизий
        for (auto* obj : allObjects) {
            if (!obj->active || !obj->collision) continue;
            obj->CheckCollisions(deltaTime);
        }
        
        // Удалить помеченные объекты
        DestroyMarked();
    }

    // Рендеринг всех объектов
    void GameObject::RenderAll() {
        // Сортировка по слоям
        std::vector<GameObject*> sorted = allObjects;
        std::sort(sorted.begin(), sorted.end(), [](GameObject* a, GameObject* b) {
            return a->layer < b->layer;
        });
        
        // Отрисовка
        for (auto* obj : sorted) {
            if (!obj->active || !obj->visible) continue;
            
            // Пока просто рисуем квадраты (позже добавим текстуры)
            QuadDesc quad;
            quad.position = Float2(obj->x, obj->y);
            quad.size = Float2(obj->width, obj->height);
            quad.color = Color(obj->color.r, obj->color.g, obj->color.b, obj->alpha);
            Renderer::DrawQuad(quad);
        }
    }

    // Удаление помеченных объектов
    void GameObject::DestroyMarked() {
        if (objectsToDestroy.empty()) return;
        
        for (auto* obj : objectsToDestroy) {
            // Вызвать OnDestroy
            if (obj->OnDestroy) {
                obj->OnDestroy();
            }
            
            // Удалить из списка
            auto it = std::find(allObjects.begin(), allObjects.end(), obj);
            if (it != allObjects.end()) {
                allObjects.erase(it);
            }

            for (auto* other : allObjects) {
                if (other == obj) continue;
                other->ClearCollision(obj);
            }
            
            SAGE_INFO("GameObject destroyed: {0}", obj->name);
            delete obj;
        }
        
        objectsToDestroy.clear();
    }

    // Удалить все объекты
    void GameObject::DestroyAll() {
        for (auto* obj : allObjects) {
            if (obj->OnDestroy) {
                obj->OnDestroy();
            }
        }

        for (auto* obj : allObjects) {
            obj->currentContacts.clear();
            delete obj;
        }
        allObjects.clear();
        objectsToDestroy.clear();
        SAGE_INFO("All GameObjects destroyed");
    }

    // Найти объект по имени
    GameObject* GameObject::Find(const std::string& name) {
        for (auto* obj : allObjects) {
            if (obj->name == name) {
                return obj;
            }
        }
        return nullptr;
    }

    // Найти все объекты с именем
    std::vector<GameObject*> GameObject::FindAll(const std::string& name) {
        std::vector<GameObject*> result;
        for (auto* obj : allObjects) {
            if (obj->name == name) {
                result.push_back(obj);
            }
        }
        return result;
    }

    // Количество объектов
    int GameObject::Count() {
        return static_cast<int>(allObjects.size());
    }

    // Движение к позиции
    void GameObject::MoveTo(float newX, float newY) {
        prevX = x;
        prevY = y;
        x = newX;
        y = newY;
    }

    // Движение на смещение
    void GameObject::MoveBy(float deltaX, float deltaY) {
        prevX = x;
        prevY = y;
        x += deltaX;
        y += deltaY;
    }

    // Прыжок
    void GameObject::Jump(float force) {
        if (!physics) {
            return;
        }

        pendingJumpVelocity = std::max(force, 0.0f);
        jumpQueued = true;
        jumpBufferTimer = jumpBuffer;
    }

    void GameObject::Jump() {
        Jump(jumpStrength);
    }

    // Толкнуть
    void GameObject::Push(float forceX, float forceY) {
        ApplyImpulse(Vector2(forceX, forceY));
    }

    // Остановить
    void GameObject::Stop() {
        speedX = 0;
        speedY = 0;
        accumulatedForces = Vector2::Zero();
    }

    // Пометить для удаления
    void GameObject::Destroy() {
        if (!markedForDestruction) {
            markedForDestruction = true;
            objectsToDestroy.push_back(this);
        }
    }

    void GameObject::SetMass(float newMass) {
        mass = std::max(newMass, 0.0001f);
        inverseMass = 1.0f / mass;
    }

    void GameObject::SetCoyoteTime(float seconds) {
        coyoteTime = std::max(seconds, 0.0f);
    }

    void GameObject::SetJumpBuffer(float seconds) {
        jumpBuffer = std::max(seconds, 0.0f);
    }

    void GameObject::ApplyForce(const Vector2& force) {
        if (!physics) {
            return;
        }
        accumulatedForces += force;
    }

    void GameObject::ApplyImpulse(const Vector2& impulse) {
        if (!physics) {
            return;
        }

        speedX += impulse.x;
        speedY += impulse.y;
    }

    void GameObject::ClearForces() {
        accumulatedForces = Vector2::Zero();
    }

    void GameObject::SetVelocity(const Vector2& velocity) {
        speedX = velocity.x;
        speedY = velocity.y;
    }

    // Проверка касания
    bool GameObject::IsTouching(GameObject* other) {
        if (!other || !collision || !other->collision) return false;
        
        // AABB коллизия (box vs box)
        if (hitboxType == "box" && other->hitboxType == "box") {
            return !(x + width < other->x ||
                     x > other->x + other->width ||
                     y + height < other->y ||
                     y > other->y + other->height);
        }
        
        // TODO: Circle и Circle vs Box
        return false;
    }

    // На экране
    bool GameObject::IsOnScreen() {
        auto& window = Application::Get().GetWindow();
        float screenWidth = static_cast<float>(window.GetWidth());
        float screenHeight = static_cast<float>(window.GetHeight());
        
        return !(x + width < 0 || x > screenWidth ||
                 y + height < 0 || y > screenHeight);
    }

    // Обновление физики
    void GameObject::BeginPhysicsStep(float deltaTime) {
        prevX = x;
        prevY = y;

        wasGroundedLastFrame = grounded;

        if (!physics) {
            return;
        }

        if (wasGroundedLastFrame) {
            coyoteTimer = coyoteTime;
        } else if (coyoteTimer > 0.0f) {
            coyoteTimer = std::max(0.0f, coyoteTimer - deltaTime);
        }

        if (jumpQueued) {
            if (jumpBufferTimer > 0.0f) {
                jumpBufferTimer = std::max(0.0f, jumpBufferTimer - deltaTime);
            }

            if (jumpBufferTimer <= 0.0f && !(wasGroundedLastFrame || coyoteTimer > 0.0f)) {
                jumpQueued = false;
                pendingJumpVelocity = 0.0f;
            }
        }

        grounded = false;
    }

    void GameObject::HandleJumpRequest() {
        if (!jumpQueued) {
            return;
        }

        if (wasGroundedLastFrame || coyoteTimer > 0.0f) {
            speedY = -pendingJumpVelocity;
            jumpQueued = false;
            pendingJumpVelocity = 0.0f;
            jumpBufferTimer = 0.0f;
            coyoteTimer = 0.0f;
        }
    }

    void GameObject::UpdatePhysics(float deltaTime) {
        const float appliedGravity = (gravity != 0.0f) ? gravity : DEFAULT_GRAVITY;
        float gravityAcceleration = appliedGravity * gravityScale;

        HandleJumpRequest();

        Vector2 acceleration = accumulatedForces * inverseMass;
        ClearForces();

        acceleration.y += gravityAcceleration;

        speedX += acceleration.x * deltaTime;
        speedY += acceleration.y * deltaTime;

        if (speedY > maxFallSpeed) {
            speedY = maxFallSpeed;
        }

        if (friction > 0.0f) {
            float modifier = wasGroundedLastFrame ? 1.0f : 0.2f;
            float factor = std::clamp(friction * modifier * deltaTime, 0.0f, 1.0f);
            speedX *= (1.0f - factor);
            if (std::abs(speedX) < 0.05f) {
                speedX = 0.0f;
            }
        }
    }

    // Обновление позиции
    void GameObject::UpdatePosition(float deltaTime) {
        x += speedX * deltaTime;
        y += speedY * deltaTime;
    }

    // Проверка коллизий
    void GameObject::CheckCollisions(float deltaTime) {
        (void)deltaTime;

        std::unordered_set<GameObject*> newContacts;

        float left = x;
        float right = x + width;
        float top = y;
        float bottom = y + height;

        for (auto* other : allObjects) {
            if (other == this || !other->active || !other->collision) {
                continue;
            }

            float otherLeft = other->x;
            float otherRight = other->x + other->width;
            float otherTop = other->y;
            float otherBottom = other->y + other->height;

            float overlapX = std::min(right, otherRight) - std::max(left, otherLeft);
            float overlapY = std::min(bottom, otherBottom) - std::max(top, otherTop);

            if (overlapX > 0.0f && overlapY > 0.0f) {
                newContacts.insert(other);

                ResolveCollision(other, overlapX, overlapY);

                left = x;
                right = x + width;
                top = y;
                bottom = y + height;
            }
        }

        for (auto* other : newContacts) {
            bool wasColliding = currentContacts.count(other) > 0;

            if (!wasColliding) {
                if (OnCollisionEnter) {
                    OnCollisionEnter(other);
                }
                if (OnCollision) {
                    OnCollision(other);
                }
            }

            if (OnCollisionStay) {
                OnCollisionStay(other);
            }
        }

        if (OnCollisionExit) {
            for (auto* other : currentContacts) {
                if (!newContacts.count(other)) {
                    OnCollisionExit(other);
                }
            }
        }

        currentContacts = std::move(newContacts);
    }

    void GameObject::ResolveCollision(GameObject* other, float overlapX, float overlapY) {
        if (!(solid && other->solid)) {
            return;
        }

        const bool selfDynamic = physics;
        const bool otherDynamic = other->physics;

        if (!selfDynamic && otherDynamic) {
            // Статичные объекты оставляют разрешение динамическому собеседнику
            return;
        }

        const float invMassSelf = selfDynamic ? inverseMass : 0.0f;
        const float invMassOther = otherDynamic ? other->inverseMass : 0.0f;
        const float totalInvMass = invMassSelf + invMassOther;
        const float restitution = std::max(bounce, other->bounce);

        if (overlapX < overlapY) {
            if (prevX + width <= other->x) {
                x = other->x - width;
            } else if (prevX >= other->x + other->width) {
                x = other->x + other->width;
            } else {
                if (x < other->x) {
                    x = other->x - width;
                } else {
                    x = other->x + other->width;
                }
            }

            const float relativeVelocity = speedX - (otherDynamic ? other->speedX : 0.0f);
            if (otherDynamic && totalInvMass > 0.0f) {
                const float impulse = -(1.0f + restitution) * relativeVelocity / totalInvMass;
                if (selfDynamic) {
                    speedX += impulse * invMassSelf;
                }
                other->speedX -= impulse * invMassOther;
            } else if (selfDynamic) {
                speedX = (bounce > 0.0f) ? -speedX * bounce : 0.0f;
            }
        } else {
            if (prevY + height <= other->y) {
                y = other->y - height;
                grounded = selfDynamic ? true : grounded;
            } else if (prevY >= other->y + other->height) {
                y = other->y + other->height;
            } else {
                if (y < other->y) {
                    y = other->y - height;
                    grounded = selfDynamic ? true : grounded;
                } else {
                    y = other->y + other->height;
                }
            }

            const float relativeVelocity = speedY - (otherDynamic ? other->speedY : 0.0f);
            if (otherDynamic && totalInvMass > 0.0f) {
                const float impulse = -(1.0f + restitution) * relativeVelocity / totalInvMass;
                if (selfDynamic) {
                    speedY += impulse * invMassSelf;
                }
                other->speedY -= impulse * invMassOther;
            } else if (selfDynamic) {
                if (grounded && std::abs(speedY) < 1.0f) {
                    speedY = 0.0f;
                } else {
                    speedY = (bounce > 0.0f) ? -speedY * bounce : 0.0f;
                }
            }
        }
    }

    void GameObject::ClearCollision(GameObject* other) {
        if (currentContacts.erase(other) > 0 && OnCollisionExit) {
            OnCollisionExit(other);
        }
    }

} // namespace SAGE
