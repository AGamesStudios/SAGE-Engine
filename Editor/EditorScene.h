#pragma once

#include <Core/Scene.h>
#include <ECS/ECSContext.h>
#include <ECS/Entity.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/SpriteComponent.h>
#include <ECS/Components/RigidBodyComponent.h>
#include <ECS/Systems/PhysicsSystem.h>

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include <filesystem>
#include <nlohmann/json.hpp>

namespace SAGE {
namespace Editor {

// EditorScene now inherits from Scene to properly integrate with engine
class EditorScene : public Scene {
public:
    struct EntityRecord {
        ECS::Entity id = ECS::NullEntity;
        std::string name;
    };
    
    enum class PlayState {
        Stopped,
        Playing,
        Paused
    };

    static constexpr int SceneFormatVersion = 2; // Increment when JSON layout changes

    EditorScene();
    virtual ~EditorScene() override = default;
    
    // Scene interface implementation
    void OnEnter(const TransitionContext& context) override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnEvent(Event& event) override;

    // Editor-specific entity management
    ECS::Entity CreateEntity(const std::string& suggestedName = {});
    bool DestroyEntity(ECS::Entity entity);
    bool RenameEntity(ECS::Entity entity, const std::string& newName);
    ECS::Entity DuplicateEntity(ECS::Entity sourceEntity, const std::string& optionalNewName = {});

    EntityRecord* FindRecord(ECS::Entity entity);
    const EntityRecord* FindRecord(ECS::Entity entity) const;

    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);
    void Clear();
    
    void MarkDirty() { m_Dirty = true; }
    void ClearDirtyFlag() { m_Dirty = false; }
    bool IsDirty() const { return m_Dirty; }

    const std::vector<EntityRecord>& GetEntities() const { return m_Entities; }

    ECS::ECSContext& GetECS() { return m_ECS; }
    const ECS::ECSContext& GetECS() const { return m_ECS; }

    ECS::TransformComponent* GetTransform(ECS::Entity entity);
    const ECS::TransformComponent* GetTransform(ECS::Entity entity) const;

    ECS::SpriteComponent* GetSprite(ECS::Entity entity);
    const ECS::SpriteComponent* GetSprite(ECS::Entity entity) const;

    ECS::RigidBodyComponent* GetRigidBody(ECS::Entity entity);
    const ECS::RigidBodyComponent* GetRigidBody(ECS::Entity entity) const;

    bool SetSpriteTexture(ECS::Entity entity, const std::string& path);
    void RefreshSpriteTextures();
    
    // Play mode controls
    void StartPlayMode();
    void StopPlayMode();
    void PausePlayMode();
    PlayState GetPlayState() const { return m_PlayState; }
    bool IsPlaying() const { return m_PlayState == PlayState::Playing; }
    
    // Physics access
    ECS::PhysicsSystem* GetPhysicsSystem() { return m_PhysicsSystem.get(); }

private:
    std::string GenerateUniqueName(const std::string& base) const;
    bool SerializeEntity(const EntityRecord& record, nlohmann::json& outEntity) const;
    bool DeserializeEntity(const nlohmann::json& entityData);
    static float ReadFloat(const nlohmann::json& object, const char* key, float defaultValue);
    bool LoadSpriteTexture(ECS::SpriteComponent& sprite, const std::string& path, bool markDirty);
    static std::string NormalizeAssetPath(const std::string& path);
    static std::filesystem::path ResolveAbsoluteAssetPath(const std::string& normalizedPath);
    static std::filesystem::path GetAssetsRoot();

    ECS::ECSContext m_ECS;
    std::vector<EntityRecord> m_Entities;
    std::uint64_t m_DefaultNameCounter = 1;
    bool m_Dirty = false;
    
    // Play mode state
    PlayState m_PlayState = PlayState::Stopped;
    std::unique_ptr<ECS::PhysicsSystem> m_PhysicsSystem;
    std::string m_PlayModeSnapshot; // JSON snapshot for restore
};

} // namespace Editor
} // namespace SAGE
