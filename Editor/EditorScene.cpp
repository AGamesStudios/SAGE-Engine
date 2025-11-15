#include "EditorScene.h"

#include <Core/Logger.h>
#include <Core/Color.h>
#include <Core/FileSystem.h>
#include <nlohmann/json.hpp>
#include <Core/ResourceManager.h>
#include <Physics/Box2DBackend.h>

// Include only stable ECS components with verified structure
#include <ECS/Components/SpriteComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/ColliderComponent.h>
#include <ECS/Components/BoxColliderComponent.h>
#include <ECS/Components/CircleColliderComponent.h>
#include <ECS/Components/ParticleSystemComponent.h>
#include <ECS/Systems/ParticleUpdateSystem.h>

// TODO: Add serialization for additional components once their APIs are stabilized:
// - RigidBodyComponent, BoxColliderComponent, CircleColliderComponent (physics)
// - HealthComponent, TriggerComponent, InventoryComponent (gameplay)
// - AudioSourceComponent, AudioListenerComponent, AnimationComponent (audio/animation)
// - TilemapComponent, CapsuleColliderComponent, CompoundColliderComponent, PolygonColliderComponent (complex types)
// - AIComponent (requires BehaviorTree.h), ScriptComponent (requires sol.hpp), StateMachineComponent (requires StateMachine.h)

#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <cstdint>

namespace SAGE {
namespace Editor {

namespace {
bool HasName(const std::vector<EditorScene::EntityRecord>& entities, const std::string& name) {
    return std::any_of(entities.begin(), entities.end(), [&](const EditorScene::EntityRecord& record) {
        return record.name == name;
    });
}

std::string ToUtf8(const std::filesystem::path& path) {
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
}

template <typename T>
T JsonGetOrDefault(const nlohmann::json& object, const char* key, const T& fallback) {
    if (!object.is_object() || key == nullptr) {
        return fallback;
    }

    try {
        return object.value(std::string(key), fallback);
    } catch (const std::exception&) {
        return fallback;
    }
}

const nlohmann::json* JsonGetObjectPtr(const nlohmann::json& object, const char* key) {
    if (!object.is_object() || key == nullptr) {
        return nullptr;
    }

    const std::string keyStr(key);
    if (!object.contains(keyStr)) {
        return nullptr;
    }

    const nlohmann::json& value = object[keyStr];
    if (!value.is_object()) {
        return nullptr;
    }

    return &value;
}

const nlohmann::json* JsonGetArrayPtr(const nlohmann::json& object, const char* key) {
    if (!object.is_object() || key == nullptr) {
        return nullptr;
    }

    const std::string keyStr(key);
    if (!object.contains(keyStr)) {
        return nullptr;
    }

    const nlohmann::json& value = object[keyStr];
    if (!value.is_array()) {
        return nullptr;
    }

    return &value;
}
}

using json = nlohmann::json;

EditorScene::EditorScene() 
    : Scene("EditorScene") // Call base Scene constructor
{
    // Initialize physics system with Box2D backend
    auto backend = std::make_unique<Physics::Box2DBackend>();
    m_PhysicsSystem = std::make_unique<ECS::PhysicsSystem>(std::move(backend));
    
    // Configure physics settings
    Physics::PhysicsSettings settings;
    settings.gravity = Vector2(0.0f, 980.0f); // 9.8 m/s² = 980 pixels/s² (100px = 1m)
    settings.velocityIterations = 8;
    settings.positionIterations = 3;
    m_PhysicsSystem->SetPhysicsSettings(settings);
    
    // Physics system starts inactive (will be active only in play mode)
    m_PhysicsSystem->SetActive(false);
    
    // Add particle update system to ECS
    m_ECS.AddSystem<ECS::ParticleUpdateSystem>();
    
    SAGE_INFO("EditorScene created with physics system and particle system");
}

void EditorScene::Clear() {
    m_ECS.Shutdown();
    m_Entities.clear();
    m_DefaultNameCounter = 1;
    m_Dirty = false;
}

// Scene interface implementation
void EditorScene::OnEnter(const TransitionContext& /*context*/) {
    SAGE_INFO("EditorScene::OnEnter");
    // Initialize ECS systems if needed
}

void EditorScene::OnExit() {
    SAGE_INFO("EditorScene::OnExit");
    // Cleanup ECS if needed
}

void EditorScene::OnUpdate(float deltaTime) {
    // Update all ECS systems (includes ParticleUpdateSystem now)
    m_ECS.Update(deltaTime);
    
    // Update physics only in play mode
    if (m_PlayState == PlayState::Playing && m_PhysicsSystem) {
        m_PhysicsSystem->Update(m_ECS.GetRegistry(), deltaTime);
    }
}

void EditorScene::OnRender() {
    // Render all entities - ECSContext doesn't have OnRender
    // Viewport will handle rendering via Renderer systems
}

void EditorScene::OnEvent(Event& /*event*/) {
    // Handle events if needed
}

ECS::Entity EditorScene::CreateEntity(const std::string& suggestedName) {
    // Limit entity count to prevent performance issues
    const size_t maxEntityCount = 10000;
    if (m_Entities.size() >= maxEntityCount) {
        SAGE_ERROR("EditorScene::CreateEntity: Maximum entity count ({}) reached", maxEntityCount);
        return ECS::NullEntity;
    }
    
    auto& registry = m_ECS.GetRegistry();
    ECS::Entity entity = registry.CreateEntity();

    std::string baseName = suggestedName.empty() ? "Entity" : suggestedName;
    std::string uniqueName = GenerateUniqueName(baseName);

    EntityRecord record{entity, uniqueName};
    m_Entities.push_back(record);

    registry.AddComponent<ECS::TransformComponent>(entity, ECS::TransformComponent{});
    registry.AddComponent<ECS::SpriteComponent>(entity, ECS::SpriteComponent{});

    m_Dirty = true;
    return entity;
}

bool EditorScene::DestroyEntity(ECS::Entity entity) {
    if (!ECS::IsValid(entity)) {
        return false;
    }

    auto it = std::find_if(m_Entities.begin(), m_Entities.end(), [entity](const EntityRecord& record) {
        return record.id == entity;
    });

    if (it != m_Entities.end()) {
        m_ECS.GetRegistry().DestroyEntity(entity);
        m_Entities.erase(it);
        m_Dirty = true;
        return true;
    }

    return false;
}

bool EditorScene::RenameEntity(ECS::Entity entity, const std::string& newName) {
    if (!ECS::IsValid(entity)) {
        return false;
    }

    auto* record = FindRecord(entity);
    if (!record) {
        return false;
    }

    if (newName.empty()) {
        SAGE_WARNING("EditorScene::RenameEntity rejected empty name");
        return false;
    }
    
    // Validate name length
    const size_t maxNameLength = 256;
    if (newName.length() > maxNameLength) {
        SAGE_WARNING("EditorScene::RenameEntity rejected name longer than {} characters", maxNameLength);
        return false;
    }
    
    // Validate no newlines or null characters
    if (newName.find('\n') != std::string::npos || 
        newName.find('\r') != std::string::npos ||
        newName.find('\0') != std::string::npos) {
        SAGE_WARNING("EditorScene::RenameEntity rejected name with newline or null characters");
        return false;
    }

    if (record->name == newName) {
        return true;
    }

    std::string candidate = newName;
    if (HasName(m_Entities, candidate)) {
        candidate = GenerateUniqueName(newName);
    }

    record->name = candidate;
    m_Dirty = true;
    return true;
}

EditorScene::EntityRecord* EditorScene::FindRecord(ECS::Entity entity) {
    auto it = std::find_if(m_Entities.begin(), m_Entities.end(), [entity](const EntityRecord& record) {
        return record.id == entity;
    });
    if (it != m_Entities.end()) {
        return &(*it);
    }
    return nullptr;
}

const EditorScene::EntityRecord* EditorScene::FindRecord(ECS::Entity entity) const {
    auto it = std::find_if(m_Entities.begin(), m_Entities.end(), [entity](const EntityRecord& record) {
        return record.id == entity;
    });
    if (it != m_Entities.end()) {
        return &(*it);
    }
    return nullptr;
}

bool EditorScene::SerializeEntity(const EntityRecord& record, json& outEntity) const {
    outEntity = json::object();
    outEntity["name"] = record.name;

    // Transform Component
    if (const auto* transform = GetTransform(record.id)) {
        json transformJson = json::object();
        transformJson["position"] = json{{"x", transform->position.x}, {"y", transform->position.y}};
        transformJson["rotation"] = transform->GetRotation();
        transformJson["scale"] = json{{"x", transform->scale.x}, {"y", transform->scale.y}};
        transformJson["size"] = json{{"x", transform->size.x}, {"y", transform->size.y}};
        transformJson["pivot"] = json{{"x", transform->pivot.x}, {"y", transform->pivot.y}};
        outEntity["transform"] = std::move(transformJson);
    }

    // Sprite Component
    if (const auto* sprite = GetSprite(record.id)) {
        json spriteJson = json::object();
        spriteJson["texturePath"] = sprite->texturePath;
        spriteJson["visible"] = sprite->visible;
        spriteJson["flipX"] = sprite->flipX;
        spriteJson["flipY"] = sprite->flipY;
        spriteJson["layer"] = sprite->layer;
        spriteJson["pivot"] = json{{"x", sprite->pivot.x}, {"y", sprite->pivot.y}};
        spriteJson["uvMin"] = json{{"x", sprite->uvMin.x}, {"y", sprite->uvMin.y}};
        spriteJson["uvMax"] = json{{"x", sprite->uvMax.x}, {"y", sprite->uvMax.y}};
        spriteJson["tint"] = json{{"r", sprite->tint.r}, {"g", sprite->tint.g}, {"b", sprite->tint.b}, {"a", sprite->tint.a}};
        outEntity["sprite"] = std::move(spriteJson);
    }

    // ParticleSystem Component
    auto& registry = m_ECS.GetRegistry();
    if (const auto* particles = registry.GetComponent<ECS::ParticleSystemComponent>(record.id)) {
        json particlesJson = json::object();
        particlesJson["emissionRate"] = particles->config.emissionRate;
        particlesJson["maxParticles"] = particles->config.maxParticles;
        particlesJson["looping"] = particles->config.looping;
        particlesJson["duration"] = particles->config.duration;
        particlesJson["minLifetime"] = particles->config.minLifetime;
        particlesJson["maxLifetime"] = particles->config.maxLifetime;
        particlesJson["position"] = json{{"x", particles->config.position.x}, {"y", particles->config.position.y}};
        particlesJson["positionVariance"] = json{{"x", particles->config.positionVariance.x}, {"y", particles->config.positionVariance.y}};
        particlesJson["spawnRadius"] = particles->config.spawnRadius;
        particlesJson["velocityMin"] = json{{"x", particles->config.velocityMin.x}, {"y", particles->config.velocityMin.y}};
        particlesJson["velocityMax"] = json{{"x", particles->config.velocityMax.x}, {"y", particles->config.velocityMax.y}};
        particlesJson["acceleration"] = json{{"x", particles->config.acceleration.x}, {"y", particles->config.acceleration.y}};
        particlesJson["startSize"] = particles->config.startSize;
        particlesJson["endSize"] = particles->config.endSize;
        particlesJson["sizeVariance"] = particles->config.sizeVariance;
        particlesJson["startColor"] = json{{"r", particles->config.startColor.r}, {"g", particles->config.startColor.g}, {"b", particles->config.startColor.b}, {"a", particles->config.startColor.a}};
        particlesJson["endColor"] = json{{"r", particles->config.endColor.r}, {"g", particles->config.endColor.g}, {"b", particles->config.endColor.b}, {"a", particles->config.endColor.a}};
        particlesJson["rotationMin"] = particles->config.rotationMin;
        particlesJson["rotationMax"] = particles->config.rotationMax;
        particlesJson["angularVelocityMin"] = particles->config.angularVelocityMin;
        particlesJson["angularVelocityMax"] = particles->config.angularVelocityMax;
        particlesJson["playOnStart"] = particles->playOnStart;
        particlesJson["autoDestroy"] = particles->autoDestroy;
        outEntity["particleSystem"] = std::move(particlesJson);
    }

    // TODO: Add serialization for additional components once their APIs are stabilized

    return true;
}

float EditorScene::ReadFloat(const json& object, const char* key, float defaultValue) {
    if (!object.is_object() || key == nullptr) {
        return defaultValue;
    }

    try {
        return object.value(std::string(key), defaultValue);
    } catch (const std::exception&) {
        return defaultValue;
    }

    return defaultValue;
}

bool EditorScene::DeserializeEntity(const json& entityData) {
    if (!entityData.is_object()) {
        return false;
    }

    const std::string name = JsonGetOrDefault<std::string>(entityData, "name", std::string("Entity"));
    ECS::Entity entity = CreateEntity(name);
    auto* record = FindRecord(entity);
    if (!record) {
        return false;
    }
    if (record->name != name) {
        RenameEntity(entity, name);
    }

    ECS::TransformComponent* transform = GetTransform(entity);
    bool sizeLoaded = false;
    if (transform) {
        if (const json* transformJson = JsonGetObjectPtr(entityData, "transform")) {
            if (const json* posJson = JsonGetObjectPtr(*transformJson, "position")) {
                transform->position.x = ReadFloat(*posJson, "x", transform->position.x);
                transform->position.y = ReadFloat(*posJson, "y", transform->position.y);
            }
            float rotation = ReadFloat(*transformJson, "rotation", transform->GetRotation());
            transform->SetRotation(rotation);
            if (const json* scaleJson = JsonGetObjectPtr(*transformJson, "scale")) {
                transform->scale.x = ReadFloat(*scaleJson, "x", transform->scale.x);
                transform->scale.y = ReadFloat(*scaleJson, "y", transform->scale.y);
            }
            if (const json* sizeJson = JsonGetObjectPtr(*transformJson, "size")) {
                transform->size.x = ReadFloat(*sizeJson, "x", transform->size.x);
                transform->size.y = ReadFloat(*sizeJson, "y", transform->size.y);
                sizeLoaded = true;
            }
            if (const json* pivotJson = JsonGetObjectPtr(*transformJson, "pivot")) {
                transform->pivot.x = ReadFloat(*pivotJson, "x", transform->pivot.x);
                transform->pivot.y = ReadFloat(*pivotJson, "y", transform->pivot.y);
            }
        }
    }

    ECS::SpriteComponent* sprite = GetSprite(entity);
    float legacyWidth = -1.0f;
    float legacyHeight = -1.0f;
    bool legacySizeFound = false;
    if (sprite) {
        if (const json* spriteJson = JsonGetObjectPtr(entityData, "sprite")) {
            if (spriteJson->contains("texturePath")) {
                sprite->texturePath = JsonGetOrDefault<std::string>(*spriteJson, "texturePath", sprite->texturePath);
            }
            sprite->visible = JsonGetOrDefault<bool>(*spriteJson, "visible", sprite->visible);
            sprite->flipX = JsonGetOrDefault<bool>(*spriteJson, "flipX", sprite->flipX);
            sprite->flipY = JsonGetOrDefault<bool>(*spriteJson, "flipY", sprite->flipY);
            sprite->layer = JsonGetOrDefault<int>(*spriteJson, "layer", sprite->layer);

            if (const json* pivotJson = JsonGetObjectPtr(*spriteJson, "pivot")) {
                sprite->pivot.x = ReadFloat(*pivotJson, "x", sprite->pivot.x);
                sprite->pivot.y = ReadFloat(*pivotJson, "y", sprite->pivot.y);
            }
            if (const json* uvMinJson = JsonGetObjectPtr(*spriteJson, "uvMin")) {
                sprite->uvMin.x = ReadFloat(*uvMinJson, "x", sprite->uvMin.x);
                sprite->uvMin.y = ReadFloat(*uvMinJson, "y", sprite->uvMin.y);
            }
            if (const json* uvMaxJson = JsonGetObjectPtr(*spriteJson, "uvMax")) {
                sprite->uvMax.x = ReadFloat(*uvMaxJson, "x", sprite->uvMax.x);
                sprite->uvMax.y = ReadFloat(*uvMaxJson, "y", sprite->uvMax.y);
            }

            if (spriteJson->contains("width")) {
                legacyWidth = JsonGetOrDefault<float>(*spriteJson, "width", legacyWidth);
                legacySizeFound = true;
            }
            if (spriteJson->contains("height")) {
                legacyHeight = JsonGetOrDefault<float>(*spriteJson, "height", legacyHeight);
                legacySizeFound = true;
            }

            if (const json* tintJson = JsonGetObjectPtr(*spriteJson, "tint")) {
                sprite->tint.r = ReadFloat(*tintJson, "r", sprite->tint.r);
                sprite->tint.g = ReadFloat(*tintJson, "g", sprite->tint.g);
                sprite->tint.b = ReadFloat(*tintJson, "b", sprite->tint.b);
                sprite->tint.a = ReadFloat(*tintJson, "a", sprite->tint.a);
            }
        }
    }

    if (legacySizeFound && transform && !sizeLoaded) {
        if (legacyWidth > 0.0f) {
            transform->size.x = legacyWidth;
        }
        if (legacyHeight > 0.0f) {
            transform->size.y = legacyHeight;
        }
    }

    // ParticleSystem Component deserialization
    auto& registry = m_ECS.GetRegistry();
    if (const json* particlesJson = JsonGetObjectPtr(entityData, "particleSystem")) {
        // Add component if it doesn't exist
        if (!registry.HasComponent<ECS::ParticleSystemComponent>(entity)) {
            registry.AddComponent<ECS::ParticleSystemComponent>(entity, ECS::ParticleSystemComponent{});
        }
        
        auto* particles = registry.GetComponent<ECS::ParticleSystemComponent>(entity);
        if (particles) {
            particles->config.emissionRate = JsonGetOrDefault<float>(*particlesJson, "emissionRate", particles->config.emissionRate);
            particles->config.maxParticles = JsonGetOrDefault<std::size_t>(*particlesJson, "maxParticles", particles->config.maxParticles);
            particles->config.looping = JsonGetOrDefault<bool>(*particlesJson, "looping", particles->config.looping);
            particles->config.duration = JsonGetOrDefault<float>(*particlesJson, "duration", particles->config.duration);
            particles->config.minLifetime = JsonGetOrDefault<float>(*particlesJson, "minLifetime", particles->config.minLifetime);
            particles->config.maxLifetime = JsonGetOrDefault<float>(*particlesJson, "maxLifetime", particles->config.maxLifetime);
            
            if (const json* posJson = JsonGetObjectPtr(*particlesJson, "position")) {
                particles->config.position.x = ReadFloat(*posJson, "x", particles->config.position.x);
                particles->config.position.y = ReadFloat(*posJson, "y", particles->config.position.y);
            }
            if (const json* posVarJson = JsonGetObjectPtr(*particlesJson, "positionVariance")) {
                particles->config.positionVariance.x = ReadFloat(*posVarJson, "x", particles->config.positionVariance.x);
                particles->config.positionVariance.y = ReadFloat(*posVarJson, "y", particles->config.positionVariance.y);
            }
            particles->config.spawnRadius = JsonGetOrDefault<float>(*particlesJson, "spawnRadius", particles->config.spawnRadius);
            
            if (const json* velMinJson = JsonGetObjectPtr(*particlesJson, "velocityMin")) {
                particles->config.velocityMin.x = ReadFloat(*velMinJson, "x", particles->config.velocityMin.x);
                particles->config.velocityMin.y = ReadFloat(*velMinJson, "y", particles->config.velocityMin.y);
            }
            if (const json* velMaxJson = JsonGetObjectPtr(*particlesJson, "velocityMax")) {
                particles->config.velocityMax.x = ReadFloat(*velMaxJson, "x", particles->config.velocityMax.x);
                particles->config.velocityMax.y = ReadFloat(*velMaxJson, "y", particles->config.velocityMax.y);
            }
            if (const json* accelJson = JsonGetObjectPtr(*particlesJson, "acceleration")) {
                particles->config.acceleration.x = ReadFloat(*accelJson, "x", particles->config.acceleration.x);
                particles->config.acceleration.y = ReadFloat(*accelJson, "y", particles->config.acceleration.y);
            }
            
            particles->config.startSize = JsonGetOrDefault<float>(*particlesJson, "startSize", particles->config.startSize);
            particles->config.endSize = JsonGetOrDefault<float>(*particlesJson, "endSize", particles->config.endSize);
            particles->config.sizeVariance = JsonGetOrDefault<float>(*particlesJson, "sizeVariance", particles->config.sizeVariance);
            
            if (const json* startColorJson = JsonGetObjectPtr(*particlesJson, "startColor")) {
                particles->config.startColor.r = ReadFloat(*startColorJson, "r", particles->config.startColor.r);
                particles->config.startColor.g = ReadFloat(*startColorJson, "g", particles->config.startColor.g);
                particles->config.startColor.b = ReadFloat(*startColorJson, "b", particles->config.startColor.b);
                particles->config.startColor.a = ReadFloat(*startColorJson, "a", particles->config.startColor.a);
            }
            if (const json* endColorJson = JsonGetObjectPtr(*particlesJson, "endColor")) {
                particles->config.endColor.r = ReadFloat(*endColorJson, "r", particles->config.endColor.r);
                particles->config.endColor.g = ReadFloat(*endColorJson, "g", particles->config.endColor.g);
                particles->config.endColor.b = ReadFloat(*endColorJson, "b", particles->config.endColor.b);
                particles->config.endColor.a = ReadFloat(*endColorJson, "a", particles->config.endColor.a);
            }
            
            particles->config.rotationMin = JsonGetOrDefault<float>(*particlesJson, "rotationMin", particles->config.rotationMin);
            particles->config.rotationMax = JsonGetOrDefault<float>(*particlesJson, "rotationMax", particles->config.rotationMax);
            particles->config.angularVelocityMin = JsonGetOrDefault<float>(*particlesJson, "angularVelocityMin", particles->config.angularVelocityMin);
            particles->config.angularVelocityMax = JsonGetOrDefault<float>(*particlesJson, "angularVelocityMax", particles->config.angularVelocityMax);
            
            particles->playOnStart = JsonGetOrDefault<bool>(*particlesJson, "playOnStart", particles->playOnStart);
            particles->autoDestroy = JsonGetOrDefault<bool>(*particlesJson, "autoDestroy", particles->autoDestroy);
            
            // Recreate emitter with loaded config
            particles->emitter = std::make_unique<ParticleEmitter>(particles->config);
        }
    }

    // TODO: Add deserialization for additional components once their APIs are stabilized

    return true;
}

bool EditorScene::SaveToFile(const std::string& path) const {
    if (path.empty()) {
        SAGE_WARNING("EditorScene::SaveToFile received empty path");
        return false;
    }
    
    // Validate path security
    if (!FileSystem::IsSafePath(path)) {
        SAGE_ERROR("EditorScene::SaveToFile: Unsafe path rejected: '{}'", path);
        return false;
    }

    try {
        std::filesystem::path fsPath(path);
        if (!fsPath.has_extension()) {
            fsPath.replace_extension(".sscene");  // По умолчанию .sscene
        }
        if (auto parent = fsPath.parent_path(); !parent.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                SAGE_WARNING("Failed to create scene directory '{}': {}", parent.string(), ec.message());
            }
        }

    json data = json::object();
    data["sceneVersion"] = SceneFormatVersion;
    data["defaultNameCounter"] = m_DefaultNameCounter;
    json entitiesJson = json::array();
        for (const auto& record : m_Entities) {
            json entityJson;
            if (SerializeEntity(record, entityJson)) {
                entitiesJson.push_back(std::move(entityJson));
            }
        }
    data["entities"] = std::move(entitiesJson);

        std::ofstream file(fsPath, std::ios::trunc);
        if (!file.is_open()) {
            SAGE_ERROR("EditorScene::SaveToFile unable to open '{}'", fsPath.string());
            return false;
        }

        file << data.dump(4);
        file.close();
        
        if (file.fail()) {
            SAGE_ERROR("EditorScene::SaveToFile: Failed to write scene file: '{}'", fsPath.string());
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("EditorScene::SaveToFile caught exception: {}", e.what());
        return false;
    }
}

bool EditorScene::LoadFromFile(const std::string& path) {
    if (path.empty()) {
        SAGE_WARNING("EditorScene::LoadFromFile received empty path");
        return false;
    }
    
    // Validate path security
    if (!FileSystem::IsSafePath(path)) {
        SAGE_ERROR("EditorScene::LoadFromFile: Unsafe path rejected: '{}'", path);
        return false;
    }

    try {
        std::filesystem::path fsPath(path);
        if (!std::filesystem::exists(fsPath)) {
            SAGE_ERROR("EditorScene::LoadFromFile missing file '{}'", fsPath.string());
            return false;
        }

        std::ifstream file(fsPath);
        if (!file.is_open()) {
            SAGE_ERROR("EditorScene::LoadFromFile unable to open '{}'", fsPath.string());
            return false;
        }

        json data;
        try {
            file >> data;
        } catch (const std::exception& e) {
            SAGE_ERROR("EditorScene::LoadFromFile JSON parsing error in '{}': {}", fsPath.string(), e.what());
            return false;
        }
        
        // Validate JSON structure
        if (!data.is_object()) {
            SAGE_ERROR("EditorScene::LoadFromFile: Invalid scene format - root is not an object");
            return false;
        }
        
        const json* entitiesPtr = JsonGetArrayPtr(data, "entities");
        if (!entitiesPtr) {
            SAGE_ERROR("EditorScene::LoadFromFile: Invalid scene format - missing 'entities' array");
            return false;
        }

        int fileVersion = JsonGetOrDefault<int>(data, "sceneVersion", 0);
        if (fileVersion > SceneFormatVersion) {
            SAGE_ERROR("EditorScene::LoadFromFile: Scene version {} is newer than supported {}. Cannot load scene.", 
                       fileVersion, SceneFormatVersion);
            return false;  // Block loading of incompatible newer versions
        }
        
        Clear();
        m_DefaultNameCounter = JsonGetOrDefault<uint64_t>(data, "defaultNameCounter", 1ULL);

        for (const auto& entityJson : *entitiesPtr) {
            if (!DeserializeEntity(entityJson)) {
                SAGE_WARNING("EditorScene::LoadFromFile skipped malformed entity entry");
            }
        }

        RefreshSpriteTextures();

        m_Dirty = false;
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("EditorScene::LoadFromFile caught exception: {}", e.what());
        return false;
    }
}

ECS::TransformComponent* EditorScene::GetTransform(ECS::Entity entity) {
    return m_ECS.GetRegistry().GetComponent<ECS::TransformComponent>(entity);
}

const ECS::TransformComponent* EditorScene::GetTransform(ECS::Entity entity) const {
    return m_ECS.GetRegistry().GetComponent<ECS::TransformComponent>(entity);
}

ECS::SpriteComponent* EditorScene::GetSprite(ECS::Entity entity) {
    return m_ECS.GetRegistry().GetComponent<ECS::SpriteComponent>(entity);
}

const ECS::SpriteComponent* EditorScene::GetSprite(ECS::Entity entity) const {
    return m_ECS.GetRegistry().GetComponent<ECS::SpriteComponent>(entity);
}

ECS::RigidBodyComponent* EditorScene::GetRigidBody(ECS::Entity entity) {
    return m_ECS.GetRegistry().GetComponent<ECS::RigidBodyComponent>(entity);
}

const ECS::RigidBodyComponent* EditorScene::GetRigidBody(ECS::Entity entity) const {
    return m_ECS.GetRegistry().GetComponent<ECS::RigidBodyComponent>(entity);
}

bool EditorScene::SetSpriteTexture(ECS::Entity entity, const std::string& path) {
    auto* sprite = GetSprite(entity);
    auto* transform = GetTransform(entity);
    if (!sprite) {
        return false;
    }

    const float defaultSize = ECS::TransformComponent::DefaultSize;
    bool shouldAutoSize = false;
    if (transform) {
        const float sizeX = transform->size.x;
        const float sizeY = transform->size.y;
        shouldAutoSize = std::abs(sizeX - defaultSize) < 0.5f && std::abs(sizeY - defaultSize) < 0.5f;
    }

    std::string previousPath = sprite->texturePath;
    if (!LoadSpriteTexture(*sprite, path, true)) {
        return false;
    }

    bool textureChanged = (!path.empty() && previousPath != sprite->texturePath);

    if (textureChanged) {
        sprite->uvMin = {0.0f, 0.0f};
        sprite->uvMax = {1.0f, 1.0f};
    }

    if (transform && sprite->texture && (shouldAutoSize || textureChanged)) {
        transform->size.x = static_cast<float>(sprite->texture->GetWidth());
        transform->size.y = static_cast<float>(sprite->texture->GetHeight());
        // Reset scale to keep pixel size consistent when auto-sizing
        transform->scale = Vector2(1.0f, 1.0f);
        m_Dirty = true;
    }

    return true;
}

void EditorScene::RefreshSpriteTextures() {
    for (const auto& record : m_Entities) {
        if (auto* sprite = GetSprite(record.id)) {
            LoadSpriteTexture(*sprite, sprite->texturePath, false);
        }
    }
}

bool EditorScene::LoadSpriteTexture(ECS::SpriteComponent& sprite, const std::string& path,
                                    bool markDirty) {
    if (path.empty()) {
        if (!sprite.texturePath.empty() || sprite.texture) {
            sprite.texturePath.clear();
            sprite.texture.reset();
            if (markDirty) {
                m_Dirty = true;
            }
        }
        return true;
    }

    std::string normalizedPath = NormalizeAssetPath(path); // relative
    std::filesystem::path absolutePath = ResolveAbsoluteAssetPath(normalizedPath);
    if (absolutePath.empty()) {
        SAGE_WARNING("EditorScene::LoadSpriteTexture received unresolved path '{}'", path);
        return false;
    }

    std::string absoluteUtf8 = ToUtf8(absolutePath);
    // Load via unified ResourceManager (Texture now implements IResource)
    auto texture = ResourceManager::Get().Load<Texture>(absoluteUtf8);
    if (!texture || !texture->IsLoaded()) {
        SAGE_ERROR("EditorScene::LoadSpriteTexture unable to load '{}'", absoluteUtf8);
        return false;
    }
    
    // Validate texture dimensions to prevent crashes/OOM
    const int maxDim = 8192;
    if (texture->GetWidth() > maxDim || texture->GetHeight() > maxDim) {
        SAGE_ERROR("Texture too large: {}x{} (max {}x{}). Rejecting to prevent OOM.", 
                   texture->GetWidth(), texture->GetHeight(), maxDim, maxDim);
        return false;  // Block loading of oversized textures
    }
    
    // Warn about very large textures (but still load them)
    const int warnDim = 4096;
    if (texture->GetWidth() > warnDim || texture->GetHeight() > warnDim) {
        SAGE_WARNING("Large texture {}x{} may impact performance", texture->GetWidth(), texture->GetHeight());
    }

    sprite.texturePath = normalizedPath;
    sprite.texture = texture;

    if (markDirty) {
        m_Dirty = true;
    }

    return true;
}

std::string EditorScene::NormalizeAssetPath(const std::string& path) {
    if (path.empty()) {
        return {};
    }

    std::filesystem::path fsPath = std::filesystem::u8path(path);
    std::filesystem::path normalized = fsPath.lexically_normal();

    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(normalized, ec);
    if (!ec) {
        normalized = canonical;
    }

    // Force relative to assets root if possible
    std::filesystem::path assetsRoot = GetAssetsRoot();
    std::error_code relEc;
    std::filesystem::path relative = std::filesystem::relative(normalized, assetsRoot, relEc);
    if (!relEc && !relative.empty() && relative.native().size() < normalized.native().size()) {
        normalized = relative;
    }

    normalized = normalized.lexically_normal();
    return ToUtf8(normalized);
}

std::filesystem::path EditorScene::GetAssetsRoot() {
    // For now use current working directory / assets
    std::filesystem::path cwd = std::filesystem::current_path();
    return cwd / "assets";
}

std::filesystem::path EditorScene::ResolveAbsoluteAssetPath(const std::string& normalizedPath) {
    if (normalizedPath.empty()) {
        return {};
    }

    std::filesystem::path fsPath = std::filesystem::u8path(normalizedPath);
    std::filesystem::path root = GetAssetsRoot();
    std::filesystem::path absolute = fsPath.is_absolute() ? fsPath : (root / fsPath);

    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(absolute, ec);
    if (!ec) {
        return canonical;
    }

    return absolute.lexically_normal();
}

ECS::Entity EditorScene::DuplicateEntity(ECS::Entity sourceEntity, const std::string& optionalNewName) {
    const EntityRecord* sourceRecord = FindRecord(sourceEntity);
    if (!sourceRecord) {
        return ECS::NullEntity;
    }
    std::string baseName = optionalNewName.empty() ? sourceRecord->name : optionalNewName;
    ECS::Entity newEntity = CreateEntity(baseName);
    if (!ECS::IsValid(newEntity)) {
        return ECS::NullEntity;
    }
    // Copy Transform
    if (auto* srcTransform = GetTransform(sourceEntity)) {
        if (auto* dstTransform = GetTransform(newEntity)) {
            *dstTransform = *srcTransform;
        }
    }
    // Copy Sprite
    if (auto* srcSprite = GetSprite(sourceEntity)) {
        if (auto* dstSprite = GetSprite(newEntity)) {
            *dstSprite = *srcSprite; // Shallow copy; texture Ref shared intentionally
        }
    }
    MarkDirty();
    return newEntity;
}

std::string EditorScene::GenerateUniqueName(const std::string& base) const {
    if (!HasName(m_Entities, base)) {
        return base;
    }

    std::uint64_t suffix = m_DefaultNameCounter;
    std::string candidate;
    do {
        candidate = base + " " + std::to_string(suffix++);
    } while (HasName(m_Entities, candidate));

    // Update counter so next call continues from last attempt
    const_cast<EditorScene*>(this)->m_DefaultNameCounter = suffix;

    return candidate;
}

void EditorScene::StartPlayMode() {
    if (m_PlayState != PlayState::Stopped) {
        return; // Already playing or paused
    }
    
    Logger::Info("EditorScene: Entering Play Mode");
    
    // Save snapshot of current scene state
    json snapshot;
    snapshot["version"] = SceneFormatVersion;
    snapshot["entities"] = json::array();
    
    for (const auto& record : m_Entities) {
        json entityJson;
        if (SerializeEntity(record, entityJson)) {
            snapshot["entities"].push_back(entityJson);
        }
    }
    
    m_PlayModeSnapshot = snapshot.dump();
    
    // Activate physics system
    m_PhysicsSystem->SetActive(true);
    
    // Create physics bodies for all entities with RigidBody + Collider
    auto& registry = m_ECS.GetRegistry();
    for (const auto& record : m_Entities) {
        if (registry.HasComponent<ECS::RigidBodyComponent>(record.id) &&
            (registry.HasComponent<ECS::ColliderComponent>(record.id) ||
             registry.HasComponent<ECS::BoxColliderComponent>(record.id) ||
             registry.HasComponent<ECS::CircleColliderComponent>(record.id))) {
            m_PhysicsSystem->CreateBody(record.id, registry);
        }
    }
    
    m_PlayState = PlayState::Playing;
    Logger::Info("EditorScene: Play Mode started ({} entities)", m_Entities.size());
}

void EditorScene::StopPlayMode() {
    if (m_PlayState == PlayState::Stopped) {
        return;
    }
    
    Logger::Info("EditorScene: Stopping Play Mode");
    
    // Deactivate physics
    m_PhysicsSystem->SetActive(false);
    
    // Restore scene from snapshot
    if (!m_PlayModeSnapshot.empty()) {
        try {
            json snapshot = json::parse(m_PlayModeSnapshot);
            
            // Clear current entities
            Clear();
            
            // Restore entities from snapshot
            if (snapshot.contains("entities") && snapshot["entities"].is_array()) {
                for (const auto& entityJson : snapshot["entities"]) {
                    DeserializeEntity(entityJson);
                }
            }
            
            m_PlayModeSnapshot.clear();
            Logger::Info("EditorScene: Scene restored from snapshot");
        }
        catch (const std::exception& e) {
            Logger::Error("EditorScene: Failed to restore scene from snapshot: {}", e.what());
        }
    }
    
    m_PlayState = PlayState::Stopped;
    Logger::Info("EditorScene: Play Mode stopped");
}

void EditorScene::PausePlayMode() {
    if (m_PlayState == PlayState::Playing) {
        m_PlayState = PlayState::Paused;
        m_PhysicsSystem->SetActive(false);
        Logger::Info("EditorScene: Play Mode paused");
    }
    else if (m_PlayState == PlayState::Paused) {
        m_PlayState = PlayState::Playing;
        m_PhysicsSystem->SetActive(true);
        Logger::Info("EditorScene: Play Mode resumed");
    }
}

} // namespace Editor
} // namespace SAGE
