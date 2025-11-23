#include "SAGE/Core/Prefab.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace SAGE {

// Helper to serialize a single entity
static json SerializeEntityToJson(ECS::Entity entity, ECS::Registry& reg) {
    json out = json::object();

    // TagComponent
    if (reg.Has<ECS::TagComponent>(entity)) {
        auto* tag = reg.Get<ECS::TagComponent>(entity);
        if (tag) {
            out["TagComponent"] = { {"Tag", tag->tag} };
        }
    }

    // TransformComponent
    if (reg.Has<ECS::TransformComponent>(entity)) {
        auto* tc = reg.Get<ECS::TransformComponent>(entity);
        if (tc) {
            json transformJson;
            transformJson["Position"] = { tc->position.x, tc->position.y };
            transformJson["Scale"] = { tc->scale.x, tc->scale.y };
            transformJson["Rotation"] = tc->rotation;
            transformJson["Origin"] = { tc->origin.x, tc->origin.y };
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
            spriteJson["Color"] = { sc->sprite.tint.r, sc->sprite.tint.g, sc->sprite.tint.b, sc->sprite.tint.a };
            // Note: Texture serialization is tricky without asset manager paths. 
            // For now we skip texture or assume white.
            out["SpriteComponent"] = spriteJson;
        }
    }

    // RigidBodyComponent
    if (reg.Has<ECS::RigidBodyComponent>(entity)) {
        auto* rb = reg.Get<ECS::RigidBodyComponent>(entity);
        if (rb) {
            json rbJson;
            rbJson["Type"] = static_cast<int>(rb->type);
            rbJson["FixedRotation"] = rb->fixedRotation;
            rbJson["GravityScale"] = rb->gravityScale;
            rbJson["Awake"] = rb->awake;
            out["RigidBodyComponent"] = rbJson;
        }
    }

    // PhysicsColliderComponent
    if (reg.Has<ECS::PhysicsColliderComponent>(entity)) {
        auto* pc = reg.Get<ECS::PhysicsColliderComponent>(entity);
        if (pc) {
            json pcJson;
            pcJson["Shape"] = static_cast<int>(pc->shape);
            pcJson["Size"] = { pc->size.x, pc->size.y };
            pcJson["Radius"] = pc->radius;
            pcJson["Offset"] = { pc->offset.x, pc->offset.y };
            pcJson["Density"] = pc->material.density;
            pcJson["Friction"] = pc->material.friction;
            pcJson["Restitution"] = pc->material.restitution;
            pcJson["IsSensor"] = pc->isSensor;
            out["PhysicsColliderComponent"] = pcJson;
        }
    }

    return out;
}

std::shared_ptr<Prefab> Prefab::Create(ECS::Entity entity, ECS::Registry& registry) {
    auto prefab = std::make_shared<Prefab>();
    json data = SerializeEntityToJson(entity, registry);
    prefab->m_Data = data.dump(4);
    return prefab;
}

bool Prefab::Save(const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) {
        SAGE_ERROR("Failed to save prefab: {}", filepath);
        return false;
    }
    fout << m_Data;
    return true;
}

std::shared_ptr<Prefab> Prefab::Load(const std::string& filepath) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        SAGE_ERROR("Failed to load prefab: {}", filepath);
        return nullptr;
    }
    
    std::stringstream buffer;
    buffer << fin.rdbuf();
    
    auto prefab = std::make_shared<Prefab>();
    prefab->m_Data = buffer.str();
    return prefab;
}

ECS::Entity Prefab::Instantiate(Scene* scene) {
    if (!scene) return ECS::kInvalidEntity;
    
    json data;
    try {
        data = json::parse(m_Data);
    } catch (const json::parse_error& e) {
        SAGE_ERROR("Failed to parse prefab data: {}", e.what());
        return ECS::kInvalidEntity;
    }

    ECS::Entity entity = scene->CreateEntity();
    auto& reg = scene->GetRegistry();

    // TagComponent
    if (data.contains("TagComponent")) {
        auto& tc = reg.Add<ECS::TagComponent>(entity);
        tc.tag = data["TagComponent"]["Tag"];
    }

    // TransformComponent
    if (data.contains("TransformComponent")) {
        auto& tc = reg.Add<ECS::TransformComponent>(entity);
        auto& tData = data["TransformComponent"];
        tc.position = { tData["Position"][0], tData["Position"][1] };
        tc.scale = { tData["Scale"][0], tData["Scale"][1] };
        tc.rotation = tData["Rotation"];
        if (tData.contains("Origin")) {
            tc.origin = { tData["Origin"][0], tData["Origin"][1] };
        }
    }

    // SpriteComponent
    if (data.contains("SpriteComponent")) {
        auto& sc = reg.Add<ECS::SpriteComponent>(entity);
        auto& sData = data["SpriteComponent"];
        sc.visible = sData["Visible"];
        sc.layer = sData["Layer"];
        sc.transparent = sData.value("Transparent", false);
        
        if (sData.contains("Color")) {
            sc.sprite.tint = { sData["Color"][0], sData["Color"][1], sData["Color"][2], sData["Color"][3] };
        }
        // Texture loading would go here
        sc.sprite.SetTexture(Texture::CreateWhiteTexture()); 
    }

    // RigidBodyComponent
    if (data.contains("RigidBodyComponent")) {
        auto& rbData = data["RigidBodyComponent"];
        auto type = static_cast<ECS::BodyType>(rbData["Type"]);
        auto& rb = reg.Add<ECS::RigidBodyComponent>(entity, type);
        rb.fixedRotation = rbData["FixedRotation"];
        rb.gravityScale = rbData["GravityScale"];
        rb.awake = rbData["Awake"];
    }

    // PhysicsColliderComponent
    if (data.contains("PhysicsColliderComponent")) {
        auto& pcData = data["PhysicsColliderComponent"];
        auto& pc = reg.Add<ECS::PhysicsColliderComponent>(entity);
        pc.shape = static_cast<ECS::ColliderShape>(pcData["Shape"]);
        pc.size = { pcData["Size"][0], pcData["Size"][1] };
        pc.radius = pcData["Radius"];
        pc.offset = { pcData["Offset"][0], pcData["Offset"][1] };
        pc.material.density = pcData["Density"];
        pc.material.friction = pcData["Friction"];
        pc.material.restitution = pcData["Restitution"];
        pc.isSensor = pcData["IsSensor"];
    }

    return entity;
}

} // namespace SAGE
