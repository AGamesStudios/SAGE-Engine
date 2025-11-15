#pragma once

#include "Types.h"
#include <vector>
#include <string>

namespace SAGE {

/**
 * @brief Scene interface
 * 
 * Manages entities and scene state.
 */
class IScene {
public:
    virtual ~IScene() = default;
    
    // Properties
    virtual const std::string& GetName() const = 0;
    virtual void SetName(const std::string& name) = 0;
    
    // Entity management
    virtual EntityHandle CreateEntity(const std::string& name = "") = 0;
    virtual void DestroyEntity(EntityHandle entity) = 0;
    virtual EntityHandle DuplicateEntity(EntityHandle entity) = 0;
    virtual bool IsValidEntity(EntityHandle entity) const = 0;
    
    // Entity queries
    virtual std::vector<EntityHandle> GetAllEntities() const = 0;
    virtual size_t GetEntityCount() const = 0;
    
    // Entity properties
    virtual std::string GetEntityName(EntityHandle entity) const = 0;
    virtual void SetEntityName(EntityHandle entity, const std::string& name) = 0;
    
    // Component queries
    virtual bool HasComponent(EntityHandle entity, const std::string& componentType) const = 0;
    virtual std::vector<std::string> GetComponentTypes(EntityHandle entity) const = 0;
    
    // Transform (always present on all entities)
    virtual TransformData GetTransform(EntityHandle entity) const = 0;
    virtual void SetTransform(EntityHandle entity, const TransformData& data) = 0;
    
    // Sprite component
    virtual bool HasSprite(EntityHandle entity) const = 0;
    virtual SpriteData GetSprite(EntityHandle entity) const = 0;
    virtual void SetSprite(EntityHandle entity, const SpriteData& data) = 0;
    virtual void AddSprite(EntityHandle entity) = 0;
    virtual void RemoveSprite(EntityHandle entity) = 0;
    
    // Camera component
    virtual bool HasCamera(EntityHandle entity) const = 0;
    virtual CameraData GetCamera(EntityHandle entity) const = 0;
    virtual void SetCamera(EntityHandle entity, const CameraData& data) = 0;
    virtual void AddCamera(EntityHandle entity) = 0;
    virtual void RemoveCamera(EntityHandle entity) = 0;
    
    // Lifecycle (for runtime)
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
};

} // namespace SAGE
