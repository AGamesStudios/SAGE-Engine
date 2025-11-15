#include "GameObject.h"
#include "Logger.h"
#include "Application.h"
#include "Graphics/API/Renderer.h"
#include "ECS/GameObjectECSBridge.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <unordered_set>
#include <unordered_map>

namespace SAGE {

    namespace {
        constexpr float DEFAULT_GRAVITY = 1200.0f;
        constexpr float DEFAULT_MAX_FALL_SPEED = 1500.0f;
        constexpr float DEFAULT_COYOTE_TIME = 0.08f;
        constexpr float DEFAULT_JUMP_BUFFER = 0.1f;
        constexpr float DEFAULT_JUMP_STRENGTH = 650.0f;
    }

    // ��T¦-T¦�TǦ�T������� ����T����-���-�-T˦�
    std::vector<GameObject*> GameObject::allObjects;
    std::vector<GameObject*> GameObject::objectsToDestroy;
    std::unordered_map<SceneID, std::vector<GameObject*>> GameObject::sceneObjects;

    // ��-���+�-�-���� �-�-Tʦ���T¦-
    GameObject* GameObject::Create(const std::string& name) {
        GameObject* obj = new GameObject();
        obj->name = name;
        obj->color = Color::White();
        obj->prevX = obj->x;
        obj->prevY = obj->y;
        obj->m_OnCreateDispatched = false;
        obj->markedForDestruction = false;
        obj->maxFallSpeed = DEFAULT_MAX_FALL_SPEED;
        obj->coyoteTime = DEFAULT_COYOTE_TIME;
        obj->jumpBuffer = DEFAULT_JUMP_BUFFER;
        obj->jumpStrength = DEFAULT_JUMP_STRENGTH;

        SceneID ownerSceneID = 0;
        Scene* ownerScene = nullptr;
        if (Application::HasInstance()) {
            ownerScene = Application::Get().GetSceneStack().GetTopScene();
            if (ownerScene) {
                ownerSceneID = ownerScene->GetID();
            }
        }
        obj->m_OwnerSceneID = ownerSceneID;

        allObjects.push_back(obj);
        sceneObjects[ownerSceneID].push_back(obj);

        // Создаём ECS сущность для объекта (ленивое создание)
        if (ownerScene) {
            ECS::GameObjectECSBridge::EnsureEntity(ownerScene, obj);
        }

        SAGE_INFO("GameObject created: {0}", name);
        return obj;
    }

    void GameObject::SetOwnerScene(Scene* scene) {
        SceneID newSceneID = scene ? scene->GetID() : 0;
        if (m_OwnerSceneID == newSceneID) {
            return;
        }

        auto removeIt = sceneObjects.find(m_OwnerSceneID);
        if (removeIt != sceneObjects.end()) {
            auto& perScene = removeIt->second;
            perScene.erase(std::remove(perScene.begin(), perScene.end(), this), perScene.end());
            if (perScene.empty()) {
                sceneObjects.erase(removeIt);
            }
        }

        m_OwnerSceneID = newSceneID;
        sceneObjects[newSceneID].push_back(this);
    }

    // �ަ-�-�-�-�����-���� �-T���T� �-�-Tʦ���T¦-�-
    void GameObject::UpdateAll(float deltaTime) {
        // Защита от обработки новых объектов, созданных во время update
        const size_t objectCount = allObjects.size();
        
        for (size_t index = 0; index < objectCount; ++index) {
            GameObject* obj = allObjects[index];
            if (!obj) continue;  // nullptr check

            if (!obj->m_OnCreateDispatched && obj->OnCreate) {
                obj->m_OnCreateDispatched = true;
                obj->OnCreate();
            }

            if (!obj->active) {
                continue;
            }

            obj->prevX = obj->x;
            obj->prevY = obj->y;
            obj->wasGroundedLastFrame = obj->grounded;

            obj->BeginPhysicsStep(deltaTime);

            if (obj->physics) {
                obj->UpdatePhysics(deltaTime);
            }

            obj->UpdatePosition(deltaTime);

            if (obj->OnUpdate) {
                obj->OnUpdate(deltaTime);
            }

            // ECS синхронизация после пользовательской логики
            Scene* ownerScene = nullptr;
            if (Application::HasInstance()) {
                auto& sceneStack = Application::Get().GetSceneStack();
                ownerScene = sceneStack.FindSceneByID(obj->m_OwnerSceneID);
            }
            
            if (ownerScene) {
                ECS::GameObjectECSBridge::Sync(ownerScene, obj);
            }
        }

        // Проверка коллизий только для объектов, существовавших до update
        for (size_t i = 0; i < objectCount; ++i) {
            GameObject* obj = allObjects[i];
            if (!obj || !obj->active || !obj->collision) {
                continue;
            }
            obj->CheckCollisions();
        }

        DestroyMarked();
    }

void GameObject::RenderAll() {
        // ��-T�T¦�T��-�-���- ���- T����-TϦ-
        std::vector<GameObject*> sorted = allObjects;
        std::sort(sorted.begin(), sorted.end(), [](GameObject* a, GameObject* b) {
            return a->layer < b->layer;
        });
        
        // ��T�T���T��-�-���-
        for (auto* obj : sorted) {
            if (!obj->active || !obj->visible) continue;
            
            // �ߦ-���- ��T��-T�T¦- T���T�Tæ��- ���-�-�+T��-T�T� (���-������ �+�-�-�-�-���- T¦���T�T�T�T�T�)
            QuadDesc quad;
            quad.position = Float2(obj->x, obj->y);
            quad.size = Float2(obj->width, obj->height);
            quad.color = Color(obj->color.r, obj->color.g, obj->color.b, obj->alpha);
            Renderer::DrawQuad(quad);
        }
    }

    // ��+�-�����-���� ���-�-��TǦ��-�-T�T� �-�-Tʦ���T¦-�-
    void GameObject::DestroyMarked() {
        if (objectsToDestroy.empty()) return;
        
        for (auto* obj : objectsToDestroy) {
            if (!obj) {
                continue;
            }
            
            const SceneID ownerSceneID = obj->m_OwnerSceneID;
            auto mapIt = sceneObjects.find(ownerSceneID);
            if (mapIt != sceneObjects.end()) {
                auto& perScene = mapIt->second;
                perScene.erase(std::remove(perScene.begin(), perScene.end(), obj), perScene.end());
                if (perScene.empty()) {
                    sceneObjects.erase(mapIt);
                }
            }

            // Удаляем ECS сущность
            Scene* ownerScene = nullptr;
            if (Application::HasInstance()) {
                ownerScene = Application::Get().GetSceneStack().FindSceneByID(ownerSceneID);
            }
            
            if (ownerScene) {
                ECS::GameObjectECSBridge::Remove(ownerScene, obj);
            }

            // ��T˦��-�-T�T� OnDestroy
            if (obj->OnDestroy) {
                obj->OnDestroy();
            }
            
            // ��+�-����T�T� ���� T�����T����-
            auto it = std::find(allObjects.begin(), allObjects.end(), obj);
            if (it != allObjects.end()) {
                allObjects.erase(it);
            }

            // Очищаем коллизии с nullptr проверкой
            for (auto* other : allObjects) {
                if (!other || other == obj) continue;
                other->ClearCollision(obj);
            }
            
            SAGE_INFO("GameObject destroyed: {0}", obj->name);
            delete obj;
        }
        
        objectsToDestroy.clear();
    }

    // ��+�-����T�T� �-T��� �-�-Tʦ���T�T�
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
        sceneObjects.clear();
        SAGE_INFO("All GameObjects destroyed");
    }

    // �ݦ-��T¦� �-�-Tʦ���T� ���- ���-���-��
    GameObject* GameObject::Find(const std::string& name) {
        for (auto* obj : allObjects) {
            if (obj->name == name) {
                return obj;
            }
        }
        return nullptr;
    }

    // �ݦ-��T¦� �-T��� �-�-Tʦ���T�T� T� ���-���-���-
    std::vector<GameObject*> GameObject::FindAll(const std::string& name) {
        std::vector<GameObject*> result;
        for (auto* obj : allObjects) {
            if (obj->name == name) {
                result.push_back(obj);
            }
        }
        return result;
    }

    // �ڦ-����TǦ�T�T¦-�- �-�-Tʦ���T¦-�-
    int GameObject::Count() {
        return static_cast<int>(allObjects.size());
    }

    void GameObject::DestroySceneObjects(SceneID sceneID) {
        std::vector<GameObject*> toDestroy;
        while (true) {
            auto it = sceneObjects.find(sceneID);
            if (it == sceneObjects.end() || it->second.empty()) {
                sceneObjects.erase(sceneID);
                break;
            }

            toDestroy = it->second;
            for (auto* obj : toDestroy) {
                if (obj) {
                    obj->Destroy();
                }
            }

            DestroyMarked();
        }
    }

    // �Ԧ-�������-���� �� ���-����TƦ���
    void GameObject::MoveTo(float newX, float newY) {
        prevX = x;
        prevY = y;
        x = newX;
        y = newY;
    }

    // �Ԧ-�������-���� �-�- T��-��Tɦ��-����
    void GameObject::MoveBy(float deltaX, float deltaY) {
        prevX = x;
        prevY = y;
        x += deltaX;
        y += deltaY;
    }

    // ��T�T˦��-��
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

    // ��-�����-T�T�T�
    void GameObject::Push(float forceX, float forceY) {
        ApplyImpulse(Vector2(forceX, forceY));
    }

    // ��T�T¦-�-�-�-��T�T�
    void GameObject::Stop() {
        speedX = 0;
        speedY = 0;
        accumulatedForces = Vector2::Zero();
    }

    // �ߦ-�-��T¦�T�T� �+��T� Tæ+�-�����-��T�
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

    // ��T��-�-��T����- ���-T��-�-��T�
    bool GameObject::IsTouching(GameObject* other) {
        if (!other || !collision || !other->collision) return false;
        
        // AABB ���-����������T� (box vs box)
        if (hitboxType == "box" && other->hitboxType == "box") {
            return !(x + width < other->x ||
                     x > other->x + other->width ||
                     y + height < other->y ||
                     y > other->y + other->height);
        }
        
        // Note: Circle-to-Circle and Circle-to-Box collision detection
        // will be implemented when needed (currently only AABB is used)
        return false;
    }

    // �ݦ- Tͦ�T��-�-��
    bool GameObject::IsOnScreen() {
        auto& window = Application::Get().GetWindow();
        float screenWidth = static_cast<float>(window.GetWidth());
        float screenHeight = static_cast<float>(window.GetHeight());
        
        return !(x + width < 0 || x > screenWidth ||
                 y + height < 0 || y > screenHeight);
    }

    // �ަ-�-�-�-�����-���� TĦ���������
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

    // �ަ-�-�-�-�����-���� ���-����TƦ���
    void GameObject::UpdatePosition(float deltaTime) {
        x += speedX * deltaTime;
        y += speedY * deltaTime;
    }

    // ��T��-�-��T����- ���-������������
    void GameObject::CheckCollisions() {
    std::unordered_set<GameObject*> newCollisionContacts;
    std::unordered_set<GameObject*> newTriggerContacts;

    float left = x;
    float right = x + width;
    float top = y;
    float bottom = y + height;

    for (auto* other : allObjects) {
        if (!other || other == this || !other->active || !other->collision) {
            continue;
        }

        float otherLeft = other->x;
        float otherRight = other->x + other->width;
        float otherTop = other->y;
        float otherBottom = other->y + other->height;

        float overlapX = std::min(right, otherRight) - std::max(left, otherLeft);
        float overlapY = std::min(bottom, otherBottom) - std::max(top, otherTop);

        if (overlapX > 0.0f && overlapY > 0.0f) {
            const bool triggerInteraction = isTrigger || other->isTrigger || !solid || !other->solid;
            const bool skipResolution = false;

            if (triggerInteraction) {
                newTriggerContacts.insert(other);
            } else {
                newCollisionContacts.insert(other);

                if (!skipResolution) {
                    ResolveCollision(other, overlapX, overlapY);

                    left = x;
                    right = x + width;
                    top = y;
                    bottom = y + height;
                }
            }
        }
    }

    for (auto* other : newCollisionContacts) {
        if (!other) continue;
        const bool wasColliding = currentContacts.count(other) > 0;
        HandlePhysicsContact(other, wasColliding ? PhysicsContactState::Stay : PhysicsContactState::Enter, false);
    }

    for (auto* other : newTriggerContacts) {
        if (!other) continue;
        const bool wasTriggering = currentTriggerContacts.count(other) > 0;
        HandlePhysicsContact(other, wasTriggering ? PhysicsContactState::Stay : PhysicsContactState::Enter, true);
    }

    std::vector<GameObject*> removedCollisions;
    removedCollisions.reserve(currentContacts.size());
    for (GameObject* other : currentContacts) {
        if (!other || !newCollisionContacts.count(other)) {
            removedCollisions.push_back(other);
        }
    }
    for (GameObject* other : removedCollisions) {
        if (other) {
            HandlePhysicsContact(other, PhysicsContactState::Exit, false);
        }
    }

    std::vector<GameObject*> removedTriggers;
    removedTriggers.reserve(currentTriggerContacts.size());
    for (GameObject* other : currentTriggerContacts) {
        if (!other || !newTriggerContacts.count(other)) {
            removedTriggers.push_back(other);
        }
    }
    for (GameObject* other : removedTriggers) {
        if (other) {
            HandlePhysicsContact(other, PhysicsContactState::Exit, true);
        }
    }
}
void GameObject::ResolveCollision(GameObject* other, float overlapX, float overlapY) {
        if (!(solid && other->solid)) {
            return;
        }

        const bool selfDynamic = physics;
        const bool otherDynamic = other->physics;

        if (!selfDynamic && otherDynamic) {
            // ��T¦-T¦�TǦ-T˦� �-�-Tʦ���T�T� �-T�T¦-�-��T�T�T� T��-��T���TȦ��-���� �+���-�-�-��TǦ�T����-�-T� T��-�-��T����+�-����T�
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
                // Только устанавливаем grounded если падаем вниз
                if (selfDynamic && speedY >= 0.0f) {
                    grounded = true;
                }
            } else if (prevY >= other->y + other->height) {
                y = other->y + other->height;
            } else {
                if (y < other->y) {
                    y = other->y - height;
                    if (selfDynamic && speedY >= 0.0f) {
                        grounded = true;
                    }
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
    if (!other) {
        return;
    }

    if (currentContacts.count(other) > 0) {
        HandlePhysicsContact(other, PhysicsContactState::Exit, false);
    }

    if (currentTriggerContacts.count(other) > 0) {
        HandlePhysicsContact(other, PhysicsContactState::Exit, true);
    }
}

    void GameObject::HandlePhysicsContact(GameObject* other, PhysicsContactState state, bool isTriggerContact) {
    if (!other) {
        return;
    }

    if (isTriggerContact) {
        switch (state) {
        case PhysicsContactState::Enter:
            if (currentTriggerContacts.insert(other).second) {
                if (OnTriggerEnter) {
                    OnTriggerEnter(other);
                }
            }
            break;
        case PhysicsContactState::Stay:
            currentTriggerContacts.insert(other);
            if (OnTriggerStay) {
                OnTriggerStay(other);
            }
            break;
        case PhysicsContactState::Exit:
            if (currentTriggerContacts.erase(other) > 0) {
                if (OnTriggerExit) {
                    OnTriggerExit(other);
                }
            }
            break;
        }
    } else {
        switch (state) {
        case PhysicsContactState::Enter:
            if (currentContacts.insert(other).second) {
                if (OnCollisionEnter) {
                    OnCollisionEnter(other);
                }
                if (OnCollision) {
                    OnCollision(other);
                }
            }
            break;
        case PhysicsContactState::Stay:
            currentContacts.insert(other);
            if (OnCollisionStay) {
                OnCollisionStay(other);
            }
            break;
        case PhysicsContactState::Exit:
            if (currentContacts.erase(other) > 0) {
                if (OnCollisionExit) {
                    OnCollisionExit(other);
                }
            }
            break;
        }
    }
}

// ==================== TAG SYSTEM IMPLEMENTATION ====================

// Find first object with tag (O(n) but with O(1) string comparison)
GameObject* GameObject::FindByTag(const char* tag) {
    StringID tagID(tag);
    return FindByTag(tagID);
}

GameObject* GameObject::FindByTag(StringID tagID) {
    for (auto* obj : allObjects) {
        if (obj->m_TagID == tagID) {
            return obj;
        }
    }
    return nullptr;
}

// Find all objects with tag
std::vector<GameObject*> GameObject::FindAllByTag(const char* tag) {
    StringID tagID(tag);
    return FindAllByTag(tagID);
}

std::vector<GameObject*> GameObject::FindAllByTag(StringID tagID) {
    std::vector<GameObject*> result;
    result.reserve(32); // Reserve space to avoid reallocations
    
    for (auto* obj : allObjects) {
        if (obj->m_TagID == tagID) {
            result.push_back(obj);
        }
    }
    
    return result;
}

} // namespace SAGE
