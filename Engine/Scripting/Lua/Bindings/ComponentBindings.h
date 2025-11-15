#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "ECS/Components/Physics/ColliderComponent.h"
#include "ECS/Components/Visual/AnimationComponent.h"
#include "ECS/Components/Audio/AudioComponent.h"
#include "ECS/Components/UI/NineSliceComponent.h"
#include "ECS/Components/Effects/ScreenEffectsComponent.h"
#include "ECS/Components/Effects/TrailComponent.h"
#include "Core/Color.h"
#include "Math/Vector2.h"
#include "Scripting/Lua/Core/LuaForward.h"
#include <entt/entt.hpp>

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief Bindings for all ECS Components
     * 
     * Exposes component types to Lua for manipulation via entities.
     */
    class ComponentBindings {
    public:
        static void BindAll(sol::state& lua) {
            BindSprite(lua);
            BindPhysics(lua);
            BindColliders(lua);
            BindStats(lua);
            BindAnimation(lua);
            BindAudio(lua);
            BindNineSlice(lua);
            BindScreenEffects(lua);
            BindTrail(lua);
        }

    private:
        static void BindSprite(sol::state& lua) {
            using namespace ECS;
            
            lua.new_usertype<SpriteComponent>("SpriteComponent",
                sol::constructors<SpriteComponent()>(),
                "texturePath", &SpriteComponent::texturePath,
                "tint", &SpriteComponent::tint,
                "visible", &SpriteComponent::visible,
                "flipX", &SpriteComponent::flipX,
                "flipY", &SpriteComponent::flipY,
                "layer", &SpriteComponent::layer,
                "uvRect", &SpriteComponent::uvRect
            );
        }

        static void BindPhysics(sol::state& lua) {
            using namespace ECS;
            
            // BodyType enum
            lua.new_enum("PhysicsBodyType",
                "Static", PhysicsBodyType::Static,
                "Kinematic", PhysicsBodyType::Kinematic,
                "Dynamic", PhysicsBodyType::Dynamic
            );

            // PhysicsComponent
            lua.new_usertype<PhysicsComponent>("Physics",
                sol::constructors<PhysicsComponent()>(),
                "type", &PhysicsComponent::type,
                "velocity", &PhysicsComponent::velocity,
                "angularVelocity", &PhysicsComponent::angularVelocity,
                "mass", &PhysicsComponent::mass,
                "staticFriction", &PhysicsComponent::staticFriction,
                "dynamicFriction", &PhysicsComponent::dynamicFriction,
                "gravityScale", &PhysicsComponent::gravityScale,
                "linearDamping", &PhysicsComponent::linearDamping,
                "angularDamping", &PhysicsComponent::angularDamping,
                "fixedRotation", &PhysicsComponent::fixedRotation,
                
                // Methods
                "ApplyForce", &PhysicsComponent::ApplyForce,
                "ApplyImpulse", &PhysicsComponent::ApplyImpulse,
                "ApplyTorque", &PhysicsComponent::ApplyTorque,
                "ClearForces", &PhysicsComponent::ClearForces,
                "SetMass", &PhysicsComponent::SetMass,
                "IsStatic", &PhysicsComponent::IsStatic,
                "IsDynamic", &PhysicsComponent::IsDynamic
            );
        }

        static void BindColliders(sol::state& lua) {
            using namespace ECS;
            
            // ColliderComponent.Type enum
            lua.new_enum<ColliderComponent::Type>("ColliderType",
                {
                    {"Circle", ColliderComponent::Type::Circle},
                    {"Box", ColliderComponent::Type::Box},
                    {"Capsule", ColliderComponent::Type::Capsule},
                    {"Polygon", ColliderComponent::Type::Polygon},
                    {"Compound", ColliderComponent::Type::Compound}
                }
            );
            
            // ColliderComponent
            lua.new_usertype<ColliderComponent>("Collider",
                sol::constructors<ColliderComponent()>(),
                "GetType", &ColliderComponent::GetType,
                "GetOffset", &ColliderComponent::GetOffset,
                "SetOffset", &ColliderComponent::SetOffset,
                "IsTrigger", &ColliderComponent::IsTrigger,
                "SetTrigger", &ColliderComponent::SetTrigger,
                "GetDensity", &ColliderComponent::GetDensity,
                "SetDensity", &ColliderComponent::SetDensity,
                "GetFriction", &ColliderComponent::GetFriction,
                "SetFriction", &ColliderComponent::SetFriction,
                "GetRestitution", &ColliderComponent::GetRestitution,
                "SetRestitution", &ColliderComponent::SetRestitution,
                
                // Factory methods
                "CreateCircle", &ColliderComponent::CreateCircle,
                "CreateBox", &ColliderComponent::CreateBox,
                "CreateCapsule", &ColliderComponent::CreateCapsule,
                "CreatePolygon", &ColliderComponent::CreatePolygon
            );
        }

        static void BindStats(sol::state& lua) {
            using namespace ECS;
            
            lua.new_usertype<StatsComponent>("Stats",
                sol::constructors<StatsComponent()>(),
                "maxHealth", &StatsComponent::maxHealth,
                "currentHealth", &StatsComponent::currentHealth,
                "maxMana", &StatsComponent::maxMana,
                "currentMana", &StatsComponent::currentMana,
                "maxStamina", &StatsComponent::maxStamina,
                "currentStamina", &StatsComponent::currentStamina,
                "armor", &StatsComponent::armor,
                "resistance", &StatsComponent::resistance,
                "healthRegenRate", &StatsComponent::healthRegenRate,
                "manaRegenRate", &StatsComponent::manaRegenRate,
                "staminaRegenRate", &StatsComponent::staminaRegenRate,
                
                // Methods
                "IsAlive", &StatsComponent::IsAlive,
                "IsInIframes", &StatsComponent::IsInIframes,
                "GetHealthPercent", &StatsComponent::GetHealthPercent,
                "GetManaPercent", &StatsComponent::GetManaPercent,
                "GetStaminaPercent", &StatsComponent::GetStaminaPercent,
                "SetStat", &StatsComponent::SetStat,
                "GetStat", &StatsComponent::GetStat,
                "ModifyStat", &StatsComponent::ModifyStat,
                "HasStat", &StatsComponent::HasStat
            );
        }
        
        static void BindAudio(sol::state& lua) {
            using namespace ECS;
            
            lua.new_usertype<AudioComponent>("Audio",
                sol::constructors<AudioComponent()>(),
                "isListener", &AudioComponent::isListener,
                "active", &AudioComponent::active,
                "soundName", &AudioComponent::soundName,
                "volume", &AudioComponent::volume,
                "pitch", &AudioComponent::pitch,
                "looping", &AudioComponent::looping,
                "spatial", &AudioComponent::spatial,
                "playOnStart", &AudioComponent::playOnStart,
                
                // Methods
                "CreateListener", &AudioComponent::CreateListener,
                "CreateSource", &AudioComponent::CreateSource,
                "IsListener", &AudioComponent::IsListener,
                "IsSource", &AudioComponent::IsSource
            );
        }

        static void BindAnimation(sol::state& lua) {
            using namespace ECS;
            
            lua.new_usertype<AnimationComponent>("AnimationComponent",
                sol::constructors<AnimationComponent()>(),
                "currentAnimation", &AnimationComponent::currentAnimation,
                "currentFrame", &AnimationComponent::currentFrame,
                "frameTime", &AnimationComponent::frameTime,
                "isPlaying", &AnimationComponent::isPlaying,
                "loop", &AnimationComponent::loop
            );
        }
        
        static void BindNineSlice(sol::state& lua) {
            using namespace ECS;
            
            // NineSliceComponent
            lua.new_usertype<NineSliceComponent>("NineSlice",
                sol::constructors<NineSliceComponent()>(),
                "layer", &NineSliceComponent::layer,
                "opacity", &NineSliceComponent::opacity,
                "visible", &NineSliceComponent::visible,
                
                // Methods
                "SetSize", &NineSliceComponent::SetSize,
                "SetColor", &NineSliceComponent::SetColor
            );
        }
        
        static void BindScreenEffects(sol::state& lua) {
            using namespace ECS;
            
            // ScreenEffectsComponent
            lua.new_usertype<ScreenEffectsComponent>("ScreenEffects",
                sol::constructors<ScreenEffectsComponent()>(),
                "enableShake", &ScreenEffectsComponent::enableShake,
                "enableFlash", &ScreenEffectsComponent::enableFlash,
                "enableTransition", &ScreenEffectsComponent::enableTransition,
                
                // Methods
                "Shake", &ScreenEffectsComponent::Shake,
                "Flash", &ScreenEffectsComponent::Flash,
                "FadeOut", sol::overload(
                    [](ScreenEffectsComponent& self, float duration) {
                        self.FadeOut(duration);
                    }
                ),
                "FadeIn", sol::overload(
                    [](ScreenEffectsComponent& self, float duration) {
                        self.FadeIn(duration);
                    }
                ),
                "GetCameraOffset", &ScreenEffectsComponent::GetCameraOffset
            );
        }
        
        static void BindTrail(sol::state& lua) {
            using namespace ECS;
            
            // TrailComponent
            lua.new_usertype<TrailComponent>("Trail",
                sol::constructors<TrailComponent()>(),
                "enableTrail", &TrailComponent::enableTrail,
                "enableDashEffect", &TrailComponent::enableDashEffect,
                
                // Methods
                "EnableTrail", &TrailComponent::EnableTrail,
                "StartDash", &TrailComponent::StartDash,
                "StopDash", &TrailComponent::StopDash,
                "SetupTrail", &TrailComponent::SetupTrail,
                "SetupDash", &TrailComponent::SetupDash
            );
        }
    };

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    class ComponentBindings {
    public:
        static void BindAll(sol::state&) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif
