#pragma once

#include "ECS/Registry.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Visual/SpriteComponentLoader.h"
#include "Math/Vector2.h"
#include "Core/Color.h"

namespace SAGE::ECS {

/// @brief Фабрика для создания стандартных сущностей
class EntityFactory {
public:
    /// @brief Создать пустую сущность с трансформом
    static Entity CreateEmpty(Registry& registry, const Vector2& position = Vector2::Zero()) {
        Entity entity = registry.CreateEntity();
        TransformComponent transform;
        transform.position = position;
        registry.AddComponent(entity, transform);
        return entity;
    }
    
    /// @brief Создать визуальный спрайт (Transform + Sprite)
    static Entity CreateSprite(
        Registry& registry,
        const Vector2& position,
        const std::string& texturePath = "",
        float width = 32.0f,
        float height = 32.0f,
        const Color& tint = Color::White(),
        int layer = 0)
    {
        Entity entity = registry.CreateEntity();
        
        // Transform с size
        TransformComponent transform;
        transform.position = position;
        transform.size = Vector2(width, height);
        registry.AddComponent(entity, transform);
        
        // Sprite
        SpriteComponent sprite;
        sprite.texturePath = texturePath;
        sprite.tint = tint;
        sprite.layer = layer;
        Detail::ResolveSpriteTexture(sprite);
        registry.AddComponent(entity, sprite);
        
        return entity;
    }
    
};

} // namespace SAGE::ECS
