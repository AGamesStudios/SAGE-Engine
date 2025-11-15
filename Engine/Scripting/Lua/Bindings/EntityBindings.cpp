#include "EntityBindings.h"
#include "Core/Logger.h"

namespace SAGE {
namespace Scripting {

    void EntityBindings::BindAll(sol::state& lua, entt::registry& registry) {
        // EntityHandle wrapper
        lua.new_usertype<EntityHandle>("Entity",
            sol::constructors<EntityHandle(entt::entity, entt::registry*)>(),
            "IsValid", &EntityHandle::IsValid,
            "Destroy", &EntityHandle::Destroy,
            
            // Transform
            "HasTransform", &EntityHandle::HasTransform,
            "GetTransform", &EntityHandle::GetTransform,
            "AddTransform", &EntityHandle::AddTransform,
            
            // Sprite
            "HasSprite", &EntityHandle::HasSprite,
            "GetSprite", &EntityHandle::GetSprite,
            "AddSprite", &EntityHandle::AddSprite,
            
            // Physics
            "HasPhysics", &EntityHandle::HasPhysics,
            "GetPhysics", &EntityHandle::GetPhysics,
            "AddPhysics", &EntityHandle::AddPhysics
        );

        // Entity creation
        lua.set_function("CreateEntity", [&registry]() -> EntityHandle {
            auto entity = registry.create();
            SAGE_INFO("Lua: Created entity {}", static_cast<uint32_t>(entity));
            return EntityHandle(entity, &registry);
        });

        // TransformComponent
        lua.new_usertype<ECS::TransformComponent>("Transform",
            sol::constructors<ECS::TransformComponent()>(),
            "position", &ECS::TransformComponent::position,
            "rotation", sol::property(&ECS::TransformComponent::GetRotation, &ECS::TransformComponent::SetRotation),
            "scale", &ECS::TransformComponent::scale,
            "size", &ECS::TransformComponent::size,
            "pivot", &ECS::TransformComponent::pivot,
            "setPivot", &ECS::TransformComponent::SetPivot,
            "setPivotCenter", &ECS::TransformComponent::SetPivotCenter,
            "rotate", &ECS::TransformComponent::Rotate,
            "getWorldPosition", &ECS::TransformComponent::GetWorldPosition,
            "getRenderPosition", &ECS::TransformComponent::GetRenderPosition
        );

        // SpriteComponent  
        lua.new_usertype<ECS::SpriteComponent>("Sprite",
            sol::constructors<ECS::SpriteComponent()>(),
            "texturePath", &ECS::SpriteComponent::texturePath,
            "tint", &ECS::SpriteComponent::tint,
            "visible", &ECS::SpriteComponent::visible,
            "flipX", &ECS::SpriteComponent::flipX,
            "flipY", &ECS::SpriteComponent::flipY,
            "layer", &ECS::SpriteComponent::layer,
            "uvMin", &ECS::SpriteComponent::uvMin,
            "uvMax", &ECS::SpriteComponent::uvMax,
            "pivot", &ECS::SpriteComponent::pivot
        );

        // PhysicsComponent
        lua.new_usertype<ECS::PhysicsComponent>("Physics",
            sol::constructors<ECS::PhysicsComponent()>(),
            "type", &ECS::PhysicsComponent::type,
            "velocity", &ECS::PhysicsComponent::velocity,
            "mass", &ECS::PhysicsComponent::mass,
            "angularVelocity", &ECS::PhysicsComponent::angularVelocity,
            "linearDamping", &ECS::PhysicsComponent::linearDamping,
            "angularDamping", &ECS::PhysicsComponent::angularDamping,
            "staticFriction", &ECS::PhysicsComponent::staticFriction,
            "dynamicFriction", &ECS::PhysicsComponent::dynamicFriction,
            "restitution", &ECS::PhysicsComponent::restitution,
            "gravityScale", &ECS::PhysicsComponent::gravityScale,
            "fixedRotation", &ECS::PhysicsComponent::fixedRotation,
            "lockX", &ECS::PhysicsComponent::lockX,
            "lockY", &ECS::PhysicsComponent::lockY,
            "setMass", &ECS::PhysicsComponent::SetMass,
            "setType", &ECS::PhysicsComponent::SetType,
            "applyForce", &ECS::PhysicsComponent::ApplyForce,
            "applyImpulse", &ECS::PhysicsComponent::ApplyImpulse,
            "applyTorque", &ECS::PhysicsComponent::ApplyTorque,
            "clearForces", &ECS::PhysicsComponent::ClearForces,
            "wakeUp", &ECS::PhysicsComponent::WakeUp
        );

        // PlayerMovementComponent
        lua.new_usertype<ECS::PlayerMovementComponent>("PlayerMovement",
            sol::constructors<ECS::PlayerMovementComponent()>(),
            "mode", &ECS::PlayerMovementComponent::mode,
            "moveSpeed", &ECS::PlayerMovementComponent::moveSpeed,
            "sprintMultiplier", &ECS::PlayerMovementComponent::sprintMultiplier,
            "jumpForce", &ECS::PlayerMovementComponent::jumpForce,
            "maxJumps", &ECS::PlayerMovementComponent::maxJumps,
            "canDash", &ECS::PlayerMovementComponent::canDash,
            "dashSpeed", &ECS::PlayerMovementComponent::dashSpeed,
            "velocity", &ECS::PlayerMovementComponent::velocity,
            "isGrounded", &ECS::PlayerMovementComponent::isGrounded,
            "isDashing", &ECS::PlayerMovementComponent::isDashing,
            "setPlatformerMode", &ECS::PlayerMovementComponent::SetPlatformerMode,
            "setTopDownMode", &ECS::PlayerMovementComponent::SetTopDownMode,
            "enableWallJump", &ECS::PlayerMovementComponent::EnableWallJump,
            "enableDash", &ECS::PlayerMovementComponent::EnableDash,
            "tryJump", &ECS::PlayerMovementComponent::TryJump,
            "tryDash", &ECS::PlayerMovementComponent::TryDash
        );

        // InventoryComponent
        lua.new_usertype<ECS::InventoryComponent>("Inventory",
            sol::constructors<ECS::InventoryComponent(), ECS::InventoryComponent(int)>(),
            "maxSlots", &ECS::InventoryComponent::maxSlots,
            "maxWeight", &ECS::InventoryComponent::maxWeight,
            "isOpen", &ECS::InventoryComponent::isOpen,
            "addItem", &ECS::InventoryComponent::AddItem,
            "removeItem", &ECS::InventoryComponent::RemoveItem,
            "hasItem", &ECS::InventoryComponent::HasItem,
            "getItemCount", &ECS::InventoryComponent::GetItemCount,
            "getCurrentWeight", &ECS::InventoryComponent::GetCurrentWeight,
            "isFull", &ECS::InventoryComponent::IsFull,
            "clear", &ECS::InventoryComponent::Clear,
            "sort", &ECS::InventoryComponent::Sort,
            "toggleUI", &ECS::InventoryComponent::ToggleUI
        );

        // InventoryItem
        lua.new_usertype<ECS::InventoryItem>("InventoryItem",
            sol::constructors<ECS::InventoryItem(), ECS::InventoryItem(const std::string&, const std::string&, int)>(),
            "id", &ECS::InventoryItem::id,
            "name", &ECS::InventoryItem::name,
            "description", &ECS::InventoryItem::description,
            "iconPath", &ECS::InventoryItem::iconPath,
            "type", &ECS::InventoryItem::type,
            "quantity", &ECS::InventoryItem::quantity,
            "maxStack", &ECS::InventoryItem::maxStack,
            "sellPrice", &ECS::InventoryItem::sellPrice,
            "buyPrice", &ECS::InventoryItem::buyPrice,
            "weight", &ECS::InventoryItem::weight
        );

        SAGE_INFO("Entity bindings registered");
    }

} // namespace Scripting
} // namespace SAGE
