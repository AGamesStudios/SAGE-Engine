#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "ECS/Components/Gameplay/PlayerMovementComponent.h"
#include "ECS/Components/Gameplay/InventoryComponent.h"
#include "Math/Vector2.h"
#include "Scripting/Lua/Core/LuaForward.h"
#include <entt/entt.hpp>

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief Entity/ECS bindings for Lua
     * 
     * Provides API to create, destroy, and manipulate entities from Lua scripts.
     * 
     * Lua Usage:
     *   local entity = Entity.Create(registry)
     *   local transform = entity:AddTransform(Vector2(100, 100))
     *   transform.position.x = transform.position.x + 10
     */
    class EntityBindings {
    public:
        // Helper wrapper for entt::entity in Lua
        struct EntityHandle {
            entt::entity entity;
            entt::registry* registry;

            EntityHandle(entt::entity e, entt::registry* r) 
                : entity(e), registry(r) {}

            bool IsValid() const {
                return registry && registry->valid(entity);
            }

            void Destroy() {
                if (IsValid()) {
                    registry->destroy(entity);
                }
            }

            // Transform component
            bool HasTransform() const {
                return IsValid() && registry->all_of<ECS::TransformComponent>(entity);
            }

            ECS::TransformComponent* GetTransform() {
                if (HasTransform()) {
                    return &registry->get<ECS::TransformComponent>(entity);
                }
                return nullptr;
            }

            ECS::TransformComponent* AddTransform(const Vector2& pos = Vector2::Zero) {
                if (!IsValid()) return nullptr;
                return &registry->emplace_or_replace<ECS::TransformComponent>(entity, pos.x, pos.y);
            }

            void RemoveTransform() {
                if (HasTransform()) {
                    registry->remove<ECS::TransformComponent>(entity);
                }
            }

            // Sprite component
            bool HasSprite() const {
                return IsValid() && registry->all_of<ECS::SpriteComponent>(entity);
            }

            ECS::SpriteComponent* GetSprite() {
                if (HasSprite()) {
                    return &registry->get<ECS::SpriteComponent>(entity);
                }
                return nullptr;
            }

            ECS::SpriteComponent* AddSprite() {
                if (!IsValid()) return nullptr;
                return &registry->emplace_or_replace<ECS::SpriteComponent>(entity);
            }

            void RemoveSprite() {
                if (HasSprite()) {
                    registry->remove<ECS::SpriteComponent>(entity);
                }
            }

            // Physics component
            #ifdef SAGE_HAS_BOX2D
            bool HasPhysics() const {
                return IsValid() && registry->all_of<ECS::PhysicsComponent>(entity);
            }

            ECS::PhysicsComponent* GetPhysics() {
                if (HasPhysics()) {
                    return &registry->get<ECS::PhysicsComponent>(entity);
                }
                return nullptr;
            }

            ECS::PhysicsComponent* AddPhysics() {
                if (!IsValid()) return nullptr;
                return &registry->emplace_or_replace<ECS::PhysicsComponent>(entity);
            }

            void RemovePhysics() {
                if (HasPhysics()) {
                    registry->remove<ECS::PhysicsComponent>(entity);
                }
            }
            #endif
        };

        static void Bind(sol::state& lua) {
            // EntityHandle
            lua.new_usertype<EntityHandle>("Entity",
                "IsValid", &EntityHandle::IsValid,
                "Destroy", &EntityHandle::Destroy,
                
                // Transform
                "HasTransform", &EntityHandle::HasTransform,
                "GetTransform", &EntityHandle::GetTransform,
                "AddTransform", &EntityHandle::AddTransform,
                "RemoveTransform", &EntityHandle::RemoveTransform,
                
                // Sprite
                "HasSprite", &EntityHandle::HasSprite,
                "GetSprite", &EntityHandle::GetSprite,
                "AddSprite", &EntityHandle::AddSprite,
                "RemoveSprite", &EntityHandle::RemoveSprite

                #ifdef SAGE_HAS_BOX2D
                ,
                // Physics
                "HasPhysics", &EntityHandle::HasPhysics,
                "GetPhysics", &EntityHandle::GetPhysics,
                "AddPhysics", &EntityHandle::AddPhysics,
                "RemovePhysics", &EntityHandle::RemovePhysics
                #endif
            );

            // Entity creation helper
            lua.set_function("CreateEntity", [](entt::registry* registry) -> EntityHandle {
                return EntityHandle(registry->create(), registry);
            });

            // TransformComponent
            using namespace ECS;
            lua.new_usertype<TransformComponent>("Transform",
                sol::constructors<TransformComponent(), TransformComponent(float, float, float)>(),
                
                "position", &TransformComponent::position,
                "rotation", sol::property(
                    &TransformComponent::GetRotation,
                    &TransformComponent::SetRotation
                ),
                "scale", &TransformComponent::scale,
                "size", &TransformComponent::size,
                "pivot", &TransformComponent::pivot,
                
                // Methods
                "GetWorldPosition", &TransformComponent::GetWorldPosition,
                "GetRenderPosition", &TransformComponent::GetRenderPosition,
                "SetPivot", &TransformComponent::SetPivot,
                "SetRotation", &TransformComponent::SetRotation,
                
                // Helpers
                "Translate", [](TransformComponent& t, const Vector2& offset) {
                    t.position = t.position + offset;
                },
                "Rotate", [](TransformComponent& t, float angleDegrees) {
                    t.SetRotation(t.GetRotation() + angleDegrees);
                }
            );

            // SpriteComponent
            lua.new_usertype<SpriteComponent>("Sprite",
                sol::constructors<SpriteComponent()>(),
                
                "textureID", &SpriteComponent::textureID,
                "textureRect", &SpriteComponent::textureRect,
                "color", &SpriteComponent::color,
                "visible", &SpriteComponent::visible,
                "layer", &SpriteComponent::layer,
                "flipX", &SpriteComponent::flipX,
                "flipY", &SpriteComponent::flipY
            );

            // PhysicsComponent (if Box2D available)
            #ifdef SAGE_HAS_BOX2D
            lua.new_usertype<PhysicsComponent>("Physics",
                sol::constructors<PhysicsComponent()>(),
                
                "velocity", &PhysicsComponent::velocity,
                "angularVelocity", &PhysicsComponent::angularVelocity,
                "mass", &PhysicsComponent::mass,
                "friction", &PhysicsComponent::friction,
                "restitution", &PhysicsComponent::restitution,
                "isStatic", &PhysicsComponent::isStatic,
                "isSensor", &PhysicsComponent::isSensor
            );
            #endif
        }
    };

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    class EntityBindings {
    public:
        static void Bind(sol::state&) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif

