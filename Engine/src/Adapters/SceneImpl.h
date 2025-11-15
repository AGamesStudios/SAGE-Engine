#pragma once

#include <SAGE/IScene.h>
#include <ECS/ECSContext.h>
#include <string>
#include <vector>

namespace SAGE {
namespace Internal {

class EngineImpl;

/**
 * @brief Internal implementation of IScene interface
 * 
 * Bridges public IScene API to internal ECS::ECSContext.
 */
class SceneImpl : public IScene {
public:
    SceneImpl(EngineImpl* engine, const std::string& name);
    ~SceneImpl() override;
    
    // IScene interface
    const std::string& GetName() const override;
    void SetName(const std::string& name) override;
    
    EntityHandle CreateEntity(const std::string& name = "") override;
    void DestroyEntity(EntityHandle entity) override;
    EntityHandle DuplicateEntity(EntityHandle entity) override;
    bool IsValidEntity(EntityHandle entity) const override;
    
    std::vector<EntityHandle> GetAllEntities() const override;
    size_t GetEntityCount() const override;
    
    std::string GetEntityName(EntityHandle entity) const override;
    void SetEntityName(EntityHandle entity, const std::string& name) override;
    
    bool HasComponent(EntityHandle entity, const std::string& componentType) const override;
    std::vector<std::string> GetComponentTypes(EntityHandle entity) const override;
    
    TransformData GetTransform(EntityHandle entity) const override;
    void SetTransform(EntityHandle entity, const TransformData& data) override;
    
    bool HasSprite(EntityHandle entity) const override;
    SpriteData GetSprite(EntityHandle entity) const override;
    void SetSprite(EntityHandle entity, const SpriteData& data) override;
    void AddSprite(EntityHandle entity) override;
    void RemoveSprite(EntityHandle entity) override;
    
    bool HasCamera(EntityHandle entity) const override;
    CameraData GetCamera(EntityHandle entity) const override;
    void SetCamera(EntityHandle entity, const CameraData& data) override;
    void AddCamera(EntityHandle entity) override;
    void RemoveCamera(EntityHandle entity) override;
    
    void Update(float deltaTime) override;
    void Render() override;
    
    // Internal access
    ECS::ECSContext& GetECSContext() { return m_ECS; }
    const ECS::ECSContext& GetECSContext() const { return m_ECS; }
    
private:
    struct EntityRecord {
        ECS::Entity id;
        std::string name;
    };
    
    EntityRecord* FindRecord(EntityHandle handle);
    const EntityRecord* FindRecord(EntityHandle handle) const;
    std::string GenerateUniqueName(const std::string& baseName);
    
    EngineImpl* m_Engine;
    std::string m_Name;
    ECS::ECSContext m_ECS;
    std::vector<EntityRecord> m_Entities;
    uint64_t m_NextEntityHandle = 1;
};

} // namespace Internal
} // namespace SAGE
