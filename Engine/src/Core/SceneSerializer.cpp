#include "SAGE/Core/SceneSerializer.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Core/GameObject.h"
#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>

using json = nlohmann::json;

namespace SAGE {

SceneSerializer::SceneSerializer(Scene* scene)
    : m_Scene(scene) {
}

static void SerializeEntity(json& out, ECS::Entity entity, ECS::Registry& reg) {
    // TagComponent
    if (reg.Has<ECS::TagComponent>(entity)) {
        auto* tag = reg.Get<ECS::TagComponent>(entity);
        if (tag) {
            out["TagComponent"] = {
                {"Tag", tag->tag}
            };
        }
    }

    // TransformComponent
    if (reg.Has<ECS::TransformComponent>(entity)) {
        auto* tc = reg.Get<ECS::TransformComponent>(entity);
        if (tc) {
            json transformJson;
            
            json pos = json::array();
            pos.push_back(tc->position.x);
            pos.push_back(tc->position.y);
            transformJson["Position"] = pos;

            json scale = json::array();
            scale.push_back(tc->scale.x);
            scale.push_back(tc->scale.y);
            transformJson["Scale"] = scale;

            transformJson["Rotation"] = tc->rotation;

            json origin = json::array();
            origin.push_back(tc->origin.x);
            origin.push_back(tc->origin.y);
            transformJson["Origin"] = origin;

            out["TransformComponent"] = transformJson;
        }
    }

    // SpriteComponent
    if (reg.Has<ECS::SpriteComponent>(entity)) {
        auto* sc = reg.Get<ECS::SpriteComponent>(entity);
        if (sc) {
            json spriteJson;
            spriteJson["Visible"] = sc->visible;
            spriteJson["Layer"] = sc->layer;
            spriteJson["Transparent"] = sc->transparent;
            
            json color = json::array();
            color.push_back(sc->sprite.tint.r);
            color.push_back(sc->sprite.tint.g);
            color.push_back(sc->sprite.tint.b);
            color.push_back(sc->sprite.tint.a);
            spriteJson["Color"] = color;

            // Serialize Texture Path
            if (auto texture = sc->sprite.GetTexture()) {
                spriteJson["TexturePath"] = texture->GetPath();
            }

            out["SpriteComponent"] = spriteJson;
        }
    }

    // CameraComponent
    if (reg.Has<ECS::CameraComponent>(entity)) {
        auto* cc = reg.Get<ECS::CameraComponent>(entity);
        if (cc) {
            json cameraJson = json::object();
            cameraJson["Active"] = cc->active;
            cameraJson["Zoom"] = cc->camera.GetZoom();
            out["CameraComponent"] = cameraJson;
        }
    }

    // RigidBodyComponent
    if (reg.Has<ECS::RigidBodyComponent>(entity)) {
        auto* rb = reg.Get<ECS::RigidBodyComponent>(entity);
        if (rb) {
            json rbJson = json::object();
            rbJson["Type"] = static_cast<int>(rb->type);
            rbJson["FixedRotation"] = rb->fixedRotation;
            rbJson["GravityScale"] = rb->gravityScale;
            rbJson["Awake"] = rb->awake;
            out["RigidBodyComponent"] = rbJson;
        }
    }

    // AudioComponent
    if (reg.Has<ECS::AudioComponent>(entity)) {
        auto* ac = reg.Get<ECS::AudioComponent>(entity);
        if (ac) {
            json audioJson;
            audioJson["Path"] = ac->path;
            audioJson["Loop"] = ac->loop;
            audioJson["Volume"] = ac->volume;
            audioJson["Spatial"] = ac->spatial;
            audioJson["MinDistance"] = ac->minDistance;
            audioJson["MaxDistance"] = ac->maxDistance;
            out["AudioComponent"] = audioJson;
        }
    }

    // PhysicsColliderComponent
    if (reg.Has<ECS::PhysicsColliderComponent>(entity)) {
        auto* pc = reg.Get<ECS::PhysicsColliderComponent>(entity);
        if (pc) {
            json colliderJson;
            colliderJson["Shape"] = static_cast<int>(pc->shape);
            
            json size = json::array();
            size.push_back(pc->size.x);
            size.push_back(pc->size.y);
            colliderJson["Size"] = size;

            colliderJson["Radius"] = pc->radius;
            
            json offset = json::array();
            offset.push_back(pc->offset.x);
            offset.push_back(pc->offset.y);
            colliderJson["Offset"] = offset;

            colliderJson["IsSensor"] = pc->isSensor;
            colliderJson["Density"] = pc->material.density;
            colliderJson["Friction"] = pc->material.friction;
            colliderJson["Restitution"] = pc->material.restitution;
            out["PhysicsColliderComponent"] = colliderJson;
        }
    }
}

void SceneSerializer::Serialize(const std::string& filepath) {
    json root;
    root["Scene"] = m_Scene->GetName();
    
    json entities = json::array();
    auto& reg = m_Scene->GetRegistry();
    
    reg.ForEachEntity([&](ECS::Entity entity) {
        json entityNode;
        entityNode["Entity"] = static_cast<uint32_t>(entity);
        SerializeEntity(entityNode, entity, reg);
        entities.push_back(entityNode);
    });

    root["Entities"] = entities;

    std::ofstream fout(filepath);
    fout << root.dump(4);
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        SAGE_ERROR("Could not open file for reading: %s", filepath.c_str());
        return false;
    }

    json root;
    stream >> root;

    auto entities = root["Entities"];
    if (!entities.is_array()) return false;

    auto& reg = m_Scene->GetRegistry();
    reg.Clear(); // Clear existing scene

    for (auto& entityNode : entities) {
        ECS::Entity entity = reg.CreateEntity();

        // TagComponent
        if (entityNode.contains("TagComponent")) {
            auto& tc = reg.Add<ECS::TagComponent>(entity);
            tc.tag = entityNode["TagComponent"]["Tag"].get<std::string>();
        }

        // TransformComponent
        if (entityNode.contains("TransformComponent")) {
            auto& tc = reg.Add<ECS::TransformComponent>(entity);
            auto& node = entityNode["TransformComponent"];
            tc.position.x = node["Position"][0].get<float>();
            tc.position.y = node["Position"][1].get<float>();
            tc.scale.x = node["Scale"][0].get<float>();
            tc.scale.y = node["Scale"][1].get<float>();
            tc.rotation = node["Rotation"].get<float>();
            tc.origin.x = node["Origin"][0].get<float>();
            tc.origin.y = node["Origin"][1].get<float>();
        }

        // SpriteComponent
        if (entityNode.contains("SpriteComponent")) {
            auto& sc = reg.Add<ECS::SpriteComponent>(entity);
            auto& node = entityNode["SpriteComponent"];
            sc.visible = node["Visible"].get<bool>();
            sc.layer = node["Layer"].get<int>();
            sc.transparent = node["Transparent"].get<bool>();
            
            auto& color = node["Color"];
            sc.sprite.tint = Color::FromRGBA(
                static_cast<uint8_t>(color[0].get<float>() * 255.0f), 
                static_cast<uint8_t>(color[1].get<float>() * 255.0f), 
                static_cast<uint8_t>(color[2].get<float>() * 255.0f), 
                static_cast<uint8_t>(color[3].get<float>() * 255.0f)
            );
            
            // Load Texture
            if (node.contains("TexturePath")) {
                std::string texturePath = node["TexturePath"].get<std::string>();
                if (!texturePath.empty()) {
                    auto texture = ResourceManager::Get().Load<Texture>(texturePath);
                    if (texture) {
                        sc.sprite.SetTexture(texture);
                    } else {
                        SAGE_ERROR("Failed to load texture for sprite: {}", texturePath);
                    }
                }
            } else {
                // Fallback or no texture
                // sc.sprite.SetTexture(Texture::CreateWhiteTexture()); // Only if needed
            }
        }

        // CameraComponent
        if (entityNode.contains("CameraComponent")) {
            auto& cc = reg.Add<ECS::CameraComponent>(entity);
            auto& node = entityNode["CameraComponent"];
            cc.active = node["Active"].get<bool>();
            // Zoom handling if needed
        }

        // RigidBodyComponent
        if (entityNode.contains("RigidBodyComponent")) {
            auto& rb = reg.Add<ECS::RigidBodyComponent>(entity);
            auto& node = entityNode["RigidBodyComponent"];
            rb.type = static_cast<ECS::BodyType>(node["Type"].get<int>());
            rb.fixedRotation = node["FixedRotation"].get<bool>();
            rb.gravityScale = node["GravityScale"].get<float>();
            rb.awake = node["Awake"].get<bool>();
        }

        // AudioComponent
        if (entityNode.contains("AudioComponent")) {
            auto& ac = reg.Add<ECS::AudioComponent>(entity);
            auto& node = entityNode["AudioComponent"];
            ac.path = node["Path"].get<std::string>();
            ac.loop = node["Loop"].get<bool>();
            ac.volume = node["Volume"].get<float>();
            ac.spatial = node["Spatial"].get<bool>();
            ac.minDistance = node["MinDistance"].get<float>();
            ac.maxDistance = node["MaxDistance"].get<float>();
        }

        // PhysicsColliderComponent
        if (entityNode.contains("PhysicsColliderComponent")) {
            auto& pc = reg.Add<ECS::PhysicsColliderComponent>(entity);
            auto& node = entityNode["PhysicsColliderComponent"];
            pc.shape = static_cast<ECS::ColliderShape>(node["Shape"].get<int>());
            pc.size.x = node["Size"][0].get<float>();
            pc.size.y = node["Size"][1].get<float>();
            pc.radius = node["Radius"].get<float>();
            pc.offset.x = node["Offset"][0].get<float>();
            pc.offset.y = node["Offset"][1].get<float>();
            pc.isSensor = node["IsSensor"].get<bool>();
            pc.material.density = node["Density"].get<float>();
            pc.material.friction = node["Friction"].get<float>();
            pc.material.restitution = node["Restitution"].get<float>();
        }
    }

    return true;
}

} // namespace SAGE
