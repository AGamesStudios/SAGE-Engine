#pragma once

#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Visual/SpriteComponentLoader.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/ECSContext.h"
#include "Core/Logger.h"

#include <unordered_map>

namespace SAGE::ECS {

/// \brief Адаптер для постепенной миграции GameObject -> ECS
/// Для каждого GameObject создаёт Entity и стандартные компоненты
class GameObjectECSBridge {
public:
    struct Entry {
        Entity entity = NullEntity;
        GameObject* object = nullptr;
        Registry* registry = nullptr;
    };

    /// Создать или получить сущность для GameObject в контексте сцены
    static Entity EnsureEntity(Scene* scene, GameObject* obj) {
        if (!scene || !obj) {
            return NullEntity;
        }
        auto& ecs = scene->GetECS();
        auto& registry = ecs.GetRegistry();
        auto it = m_Mapping.find(obj);
        if (it != m_Mapping.end()) {
            Entry& entry = it->second;
            if (entry.registry == &registry && entry.entity != NullEntity && registry.ContainsEntity(entry.entity)) {
                obj->SetOwnerScene(scene);
                return entry.entity;
            }

            if (entry.registry && entry.registry != &registry && entry.registry->ContainsEntity(entry.entity)) {
                entry.registry->DestroyEntity(entry.entity);
            }

            m_Mapping.erase(it);
        }
        Entity e = registry.CreateEntity();
        // Создаём базовые компоненты
        TransformComponent transform(obj->x, obj->y, obj->angle);
        transform.size = Vector2(obj->width, obj->height);  // width/height должны идти в size, а не в scale!
        registry.AddComponent(e, transform);
        SpriteComponent sprite;
        sprite.texturePath = obj->image;
        sprite.tint = obj->color;
        sprite.tint.a = obj->alpha;  // GameObject.alpha -> SpriteComponent.tint.a
        sprite.visible = obj->visible;
        sprite.flipX = obj->flipX;
        sprite.flipY = obj->flipY;
        sprite.layer = obj->layer;
        Detail::ResolveSpriteTexture(sprite);
        registry.AddComponent(e, sprite);
        obj->SetOwnerScene(scene);
        m_Mapping[obj] = { e, obj, &registry };
        return e;
    }

    /// Синхронизировать данные GameObject -> ECS компоненты
    static void Sync(Scene* scene, GameObject* obj) {
        Entity e = EnsureEntity(scene, obj);
        auto& registry = scene->GetECS().GetRegistry();
        if (!registry.ContainsEntity(e)) {
            return;
        }
        if (auto* t = registry.GetComponent<TransformComponent>(e)) {
            t->position = Vector2(obj->x, obj->y);
            t->SetRotation(obj->angle);
            t->size = Vector2(obj->width, obj->height);  // width/height -> size, НЕ scale!
        }
        if (auto* s = registry.GetComponent<SpriteComponent>(e)) {
            s->tint = obj->color;
            s->tint.a = obj->alpha;  // GameObject.alpha -> SpriteComponent.tint.a
            s->visible = obj->visible;
            s->flipX = obj->flipX;
            s->flipY = obj->flipY;
            s->layer = obj->layer;
            if (s->texturePath != obj->image) {
                s->texturePath = obj->image;
                Detail::ResolveSpriteTexture(*s);
            }
        }
    }

    /// Удалить сущность при уничтожении GameObject
    static void Remove(Scene* scene, GameObject* obj) {
        if (!scene || !obj) return;
        auto it = m_Mapping.find(obj);
        if (it == m_Mapping.end()) return;
        auto& registry = scene->GetECS().GetRegistry();
        Entry& entry = it->second;
        if (entry.registry == &registry && registry.ContainsEntity(entry.entity)) {
            registry.DestroyEntity(entry.entity);
        } else if (entry.registry && entry.registry != &registry && entry.registry->ContainsEntity(entry.entity)) {
            entry.registry->DestroyEntity(entry.entity);
        }
        m_Mapping.erase(it);
    }

private:
    static inline std::unordered_map<GameObject*, Entry> m_Mapping; // mapping GO->Entity
};

} // namespace SAGE::ECS