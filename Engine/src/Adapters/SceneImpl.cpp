#include "SceneImpl.h"
#include "EngineImpl.h"
#include <ECS/Components/Core/TransformComponent.h>
#include <ECS/Components/Visual/SpriteComponent.h>
#include <ECS/Components/Visual/CameraComponent.h>
#include <Core/Logger.h>
#include <algorithm>

namespace SAGE {
namespace Internal {

SceneImpl::SceneImpl(EngineImpl* engine, const std::string& name)
    : m_Engine(engine)
    , m_Name(name)
{
}

SceneImpl::~SceneImpl() = default;

const std::string& SceneImpl::GetName() const {
    return m_Name;
}

void SceneImpl::SetName(const std::string& name) {
    m_Name = name;
}

EntityHandle SceneImpl::CreateEntity(const std::string& name) {
    auto& registry = m_ECS.GetRegistry();
    
    // Create ECS entity
    ECS::Entity ecsEntity = registry.CreateEntity();
    if (!ECS::IsValid(ecsEntity)) {
        SAGE_ERROR("Failed to create ECS entity");
        return NullEntity;
    }
    
    // Add default Transform component
    registry.AddComponent<ECS::TransformComponent>(ecsEntity, ECS::TransformComponent{});
    
    // Create entity record
    EntityHandle handle = m_NextEntityHandle++;
    std::string entityName = name.empty() ? GenerateUniqueName("Entity") : name;
    
    m_Entities.push_back({ecsEntity, entityName});
    
    return handle;
}

void SceneImpl::DestroyEntity(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) {
        SAGE_WARN("Entity handle {} not found", handle);
        return;
    }
    
    auto& registry = m_ECS.GetRegistry();
    registry.DestroyEntity(record->id);
    
    // Remove from records
    m_Entities.erase(
        std::remove_if(m_Entities.begin(), m_Entities.end(),
            [handle, record](const EntityRecord& r) { return r.id == record->id; }),
        m_Entities.end()
    );
}

EntityHandle SceneImpl::DuplicateEntity(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) {
        SAGE_WARN("Entity handle {} not found", handle);
        return NullEntity;
    }
    
    // Create new entity with same components
    EntityHandle newHandle = CreateEntity(record->name + " (Copy)");
    
    // TODO: Copy all components
    
    return newHandle;
}

bool SceneImpl::IsValidEntity(EntityHandle handle) const {
    return FindRecord(handle) != nullptr;
}

std::vector<EntityHandle> SceneImpl::GetAllEntities() const {
    std::vector<EntityHandle> handles;
    handles.reserve(m_Entities.size());
    
    for (size_t i = 0; i < m_Entities.size(); ++i) {
        handles.push_back(static_cast<EntityHandle>(i + 1));
    }
    
    return handles;
}

size_t SceneImpl::GetEntityCount() const {
    return m_Entities.size();
}

std::string SceneImpl::GetEntityName(EntityHandle handle) const {
    auto* record = FindRecord(handle);
    return record ? record->name : "";
}

void SceneImpl::SetEntityName(EntityHandle handle, const std::string& name) {
    auto* record = FindRecord(handle);
    if (record) {
        record->name = name;
    }
}

bool SceneImpl::HasComponent(EntityHandle handle, const std::string& componentType) const {
    auto* record = FindRecord(handle);
    if (!record) return false;
    
    auto& registry = m_ECS.GetRegistry();
    
    if (componentType == "Transform") return registry.HasComponent<ECS::TransformComponent>(record->id);
    if (componentType == "Sprite") return registry.HasComponent<ECS::SpriteComponent>(record->id);
    if (componentType == "Camera") return registry.HasComponent<ECS::CameraComponent>(record->id);
    
    return false;
}

std::vector<std::string> SceneImpl::GetComponentTypes(EntityHandle handle) const {
    std::vector<std::string> types;
    
    auto* record = FindRecord(handle);
    if (!record) return types;
    
    auto& registry = m_ECS.GetRegistry();
    
    if (registry.HasComponent<ECS::TransformComponent>(record->id)) types.push_back("Transform");
    if (registry.HasComponent<ECS::SpriteComponent>(record->id)) types.push_back("Sprite");
    if (registry.HasComponent<ECS::CameraComponent>(record->id)) types.push_back("Camera");
    
    return types;
}

TransformData SceneImpl::GetTransform(EntityHandle handle) const {
    auto* record = FindRecord(handle);
    if (!record) return TransformData{};
    
    auto& registry = m_ECS.GetRegistry();
    const auto* transform = registry.GetComponent<ECS::TransformComponent>(record->id);
    
    if (!transform) return TransformData{};
    
    TransformData data;
    data.position = {transform->position.x, transform->position.y, 0.0f};
    data.rotation = {0.0f, 0.0f, transform->rotation};
    data.scale = {transform->scale.x, transform->scale.y, 1.0f};
    
    return data;
}

void SceneImpl::SetTransform(EntityHandle handle, const TransformData& data) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    auto* transform = registry.GetComponent<ECS::TransformComponent>(record->id);
    
    if (transform) {
        transform->position = {data.position.x, data.position.y};
        transform->rotation = data.rotation.z;
        transform->scale = {data.scale.x, data.scale.y};
    }
}

bool SceneImpl::HasSprite(EntityHandle handle) const {
    return HasComponent(handle, "Sprite");
}

SpriteData SceneImpl::GetSprite(EntityHandle handle) const {
    auto* record = FindRecord(handle);
    if (!record) return SpriteData{};
    
    auto& registry = m_ECS.GetRegistry();
    const auto* sprite = registry.GetComponent<ECS::SpriteComponent>(record->id);
    
    if (!sprite) return SpriteData{};
    
    SpriteData data;
    data.color = Color(sprite->tint.r, sprite->tint.g, sprite->tint.b, sprite->tint.a);
    data.layer = sprite->layer;
    data.flipX = sprite->flipX;
    data.flipY = sprite->flipY;
    data.uvOffset = {sprite->uvMin.x, sprite->uvMin.y};
    data.uvScale = {sprite->uvMax.x - sprite->uvMin.x, sprite->uvMax.y - sprite->uvMin.y};
    data.texture = NullTexture; // Texture handle mapping not yet available
    // Size remains as default until sprite rendering exposes explicit dimensions
    
    return data;
}

void SceneImpl::SetSprite(EntityHandle handle, const SpriteData& data) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    auto* sprite = registry.GetComponent<ECS::SpriteComponent>(record->id);
    
    if (sprite) {
        sprite->tint = Color(data.color.r, data.color.g, data.color.b, data.color.a);
        sprite->layer = data.layer;
        sprite->flipX = data.flipX;
        sprite->flipY = data.flipY;
        sprite->uvMin = {data.uvOffset.x, data.uvOffset.y};
        sprite->uvMax = {
            data.uvOffset.x + data.uvScale.x,
            data.uvOffset.y + data.uvScale.y
        };
        // Texture/material mapping requires resource system integration
    }
}

void SceneImpl::AddSprite(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    if (!registry.HasComponent<ECS::SpriteComponent>(record->id)) {
        registry.AddComponent<ECS::SpriteComponent>(record->id, ECS::SpriteComponent{});
    }
}

void SceneImpl::RemoveSprite(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    registry.RemoveComponent<ECS::SpriteComponent>(record->id);
}

bool SceneImpl::HasCamera(EntityHandle handle) const {
    return HasComponent(handle, "Camera");
}

CameraData SceneImpl::GetCamera(EntityHandle) const {
    // TODO: Implement
    return CameraData{};
}

void SceneImpl::SetCamera(EntityHandle, const CameraData&) {
    // TODO: Implement
}

void SceneImpl::AddCamera(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    if (!registry.HasComponent<ECS::CameraComponent>(record->id)) {
        registry.AddComponent<ECS::CameraComponent>(record->id, ECS::CameraComponent{});
    }
}

void SceneImpl::RemoveCamera(EntityHandle handle) {
    auto* record = FindRecord(handle);
    if (!record) return;
    
    auto& registry = m_ECS.GetRegistry();
    registry.RemoveComponent<ECS::CameraComponent>(record->id);
}

void SceneImpl::Update(float) {
    // TODO: Update ECS systems
}

void SceneImpl::Render() {
    // TODO: Render entities
}

SceneImpl::EntityRecord* SceneImpl::FindRecord(EntityHandle handle) {
    if (handle == NullEntity || handle > m_Entities.size()) return nullptr;
    return &m_Entities[handle - 1];
}

const SceneImpl::EntityRecord* SceneImpl::FindRecord(EntityHandle handle) const {
    if (handle == NullEntity || handle > m_Entities.size()) return nullptr;
    return &m_Entities[handle - 1];
}

std::string SceneImpl::GenerateUniqueName(const std::string& baseName) {
    std::string name = baseName;
    int counter = 1;
    
    while (std::any_of(m_Entities.begin(), m_Entities.end(),
        [&name](const EntityRecord& r) { return r.name == name; }))
    {
        name = baseName + " " + std::to_string(counter++);
    }
    
    return name;
}

} // namespace Internal
} // namespace SAGE
