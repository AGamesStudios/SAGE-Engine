#pragma once

#include "Core/Scene.h"
#include "ECS/Registry.h"
#include "ECS/ECS.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "Graphics/Core/Types/Color.h"
#include "Core/Logger.h"
#include "Graphics/Core/Types/MathTypes.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

namespace SAGE {

using json = nlohmann::json;

/// @brief Расширенный сериализатор сцен с поддержкой компонентов
class SceneSerializer {
public:
    /// @brief Сохранить сцену с ECS entities и компонентами
    static bool SaveToFile(const Scene* scene, const std::string& filePath) {
        if (!scene) return false;
        
        try {
            json sceneData;
            sceneData["name"] = scene->GetName();
            sceneData["entities"] = json::array();
            
            // Получаем registry из сцены (предполагаем что он есть)
            // const auto& registry = scene->GetRegistry();
            
            // TODO: Сериализация entities
            // Здесь должна быть итерация по всем entities
            // и сериализация их компонентов
            
            std::ofstream file(filePath);
            if (!file.is_open()) return false;
            
            file << sceneData.dump(4);
            file.close();
            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("SceneSerializer save error: {0}", e.what());
            return false;
        }
    }
    
    /// @brief Загрузить сцену из файла
    static bool LoadFromFile(Scene* scene, const std::string& filePath) {
        if (!scene) return false;
        
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) return false;
            
            json sceneData;
            file >> sceneData;
            file.close();
            
            // TODO: Десериализация entities и компонентов
            
            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("SceneSerializer load error: {0}", e.what());
            return false;
        }
    }
    
    /// @brief Сериализовать TransformComponent в JSON
    static json SerializeTransform(const ECS::TransformComponent& transform) {
        json j;
        j["position"] = MakeArray(transform.position.x, transform.position.y);
        j["rotation"] = transform.rotation;
        j["scale"] = MakeArray(transform.scale.x, transform.scale.y);
        j["size"] = MakeArray(transform.size.x, transform.size.y);
        j["pivot"] = MakeArray(transform.pivot.x, transform.pivot.y);
        return j;
    }
    
    /// @brief Десериализовать TransformComponent из JSON
    static ECS::TransformComponent DeserializeTransform(const json& j) {
        ECS::TransformComponent transform;
        if (j.contains("position") && j["position"].size() == 2) {
            transform.position = Vector2(
                j["position"][0].get<float>(),
                j["position"][1].get<float>());
        }
        if (j.contains("rotation")) {
            transform.rotation = j["rotation"].get<float>();
        }
        if (j.contains("scale") && j["scale"].size() == 2) {
            transform.scale = Vector2(
                j["scale"][0].get<float>(),
                j["scale"][1].get<float>());
        }
        if (j.contains("size") && j["size"].size() == 2) {
            transform.size = Vector2(
                j["size"][0].get<float>(),
                j["size"][1].get<float>());
        }
        if (j.contains("pivot") && j["pivot"].size() == 2) {
            transform.pivot = Vector2(
                j["pivot"][0].get<float>(),
                j["pivot"][1].get<float>());
        }
        return transform;
    }
    
    /// @brief Сериализовать SpriteComponent в JSON
    static json SerializeSprite(const ECS::SpriteComponent& sprite) {
        json j;
        j["texturePath"] = sprite.texturePath;
        j["tint"] = MakeArray(sprite.tint.r, sprite.tint.g, sprite.tint.b, sprite.tint.a);
        j["visible"] = sprite.visible;
        j["flipX"] = sprite.flipX;
        j["flipY"] = sprite.flipY;
        j["layer"] = sprite.layer;
        j["uvMin"] = MakeArray(sprite.uvMin.x, sprite.uvMin.y);
        j["uvMax"] = MakeArray(sprite.uvMax.x, sprite.uvMax.y);
        j["pivot"] = MakeArray(sprite.pivot.x, sprite.pivot.y);
        return j;
    }
    
    /// @brief Десериализовать SpriteComponent из JSON
    static ECS::SpriteComponent DeserializeSprite(const json& j) {
        ECS::SpriteComponent sprite;
        sprite.texturePath = j.value("texturePath", std::string{});

        if (j.contains("tint") && j["tint"].is_array() && j["tint"].size() == 4) {
            sprite.tint = Color(
                j["tint"][0].get<float>(),
                j["tint"][1].get<float>(),
                j["tint"][2].get<float>(),
                j["tint"][3].get<float>());
        }

        sprite.visible = j.value("visible", true);
        sprite.flipX = j.value("flipX", false);
        sprite.flipY = j.value("flipY", false);
        sprite.layer = j.value("layer", 0);

        if (j.contains("uvMin") && j["uvMin"].is_array() && j["uvMin"].size() == 2) {
            sprite.uvMin = Float2(
                j["uvMin"][0].get<float>(),
                j["uvMin"][1].get<float>());
        }
        if (j.contains("uvMax") && j["uvMax"].is_array() && j["uvMax"].size() == 2) {
            sprite.uvMax = Float2(
                j["uvMax"][0].get<float>(),
                j["uvMax"][1].get<float>());
        }
        if (j.contains("pivot") && j["pivot"].is_array() && j["pivot"].size() == 2) {
            sprite.pivot = Float2(
                j["pivot"][0].get<float>(),
                j["pivot"][1].get<float>());
        }

        return sprite;
    }
    
    /// @brief Сериализовать PhysicsComponent в JSON
    static json SerializePhysics(const ECS::PhysicsComponent& physics) {
        json j;
        j["type"] = static_cast<int>(physics.type);
        j["mass"] = physics.mass;
        j["linearDamping"] = physics.linearDamping;
        j["angularDamping"] = physics.angularDamping;
        j["staticFriction"] = physics.staticFriction;
        j["dynamicFriction"] = physics.dynamicFriction;
        j["restitution"] = physics.restitution;
        j["gravityScale"] = physics.gravityScale;
        j["fixedRotation"] = physics.fixedRotation;
        j["lockX"] = physics.lockX;
        j["lockY"] = physics.lockY;
        return j;
    }

    static void DeserializePhysics(const json& j, ECS::PhysicsComponent& physics) {
        physics.SetType(static_cast<ECS::PhysicsBodyType>(
            j.value("type", static_cast<int>(ECS::PhysicsBodyType::Dynamic))));
        physics.SetMass(j.value("mass", physics.mass));
        physics.linearDamping = j.value("linearDamping", physics.linearDamping);
        physics.angularDamping = j.value("angularDamping", physics.angularDamping);
        physics.staticFriction = j.value("staticFriction", physics.staticFriction);
        physics.dynamicFriction = j.value("dynamicFriction", physics.dynamicFriction);
        physics.restitution = j.value("restitution", physics.restitution);
        physics.gravityScale = j.value("gravityScale", physics.gravityScale);
        physics.fixedRotation = j.value("fixedRotation", physics.fixedRotation);
        physics.lockX = j.value("lockX", physics.lockX);
        physics.lockY = j.value("lockY", physics.lockY);
    }
    
    /// @brief Сериализовать Registry целиком
    static json SerializeRegistry(const ECS::Registry& registry) {
        json j;
        j["entities"] = json::array();
        
        const auto& entities = registry.GetEntities();

        for (ECS::Entity entity : entities) {
            json entityData;
            entityData["id"] = entity;
            entityData["components"] = json::object();
            
            // Проверяем и сериализуем каждый тип компонента
            if (auto* transform = registry.GetComponent<ECS::TransformComponent>(entity)) {
                entityData["components"]["Transform"] = SerializeTransform(*transform);
            }
            
            if (auto* sprite = registry.GetComponent<ECS::SpriteComponent>(entity)) {
                entityData["components"]["Sprite"] = SerializeSprite(*sprite);
            }
            
            if (auto* physics = registry.GetComponent<ECS::PhysicsComponent>(entity)) {
                entityData["components"]["Physics"] = SerializePhysics(*physics);
            }
            
            j["entities"].push_back(entityData);
        }
        
        return j;
    }
private:
    template <typename... TArgs>
    static json MakeArray(TArgs... values) {
        json arr = json::array();
        (arr.push_back(values), ...);
        return arr;
    }
};

}