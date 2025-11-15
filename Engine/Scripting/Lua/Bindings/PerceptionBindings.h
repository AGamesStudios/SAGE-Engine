#pragma once

#include "AI/Perception.h"
#include "Math/Vector2.h"
#include "Core/Logger.h"
#include "Scripting/Lua/Core/LuaForward.h"

namespace SAGE {
    
    // Forward declarations
    class GameObject;

    /**
     * @brief Perception system Lua bindings
     * 
     * Exposes vision cones, hearing, and perception events to Lua scripts.
     */
    #if SAGE_ENABLE_LUA

    class PerceptionBindings {
    public:
        static void BindAll(sol::state& lua) {
            BindStructs(lua);
            BindPerceptionComponent(lua);
            BindEvents(lua);
            
            SAGE_INFO("Perception bindings registered");
        }

    private:
        static void BindStructs(sol::state& lua) {
            // PerceptionSettings
            lua.new_usertype<PerceptionSettings>("PerceptionSettings",
                sol::constructors<PerceptionSettings()>(),
                
                // Vision
                "visionRange", &PerceptionSettings::visionRange,
                "visionAngle", &PerceptionSettings::visionAngle,
                "peripheralVisionAngle", &PerceptionSettings::peripheralVisionAngle,
                "useLineOfSight", &PerceptionSettings::useLineOfSight,
                
                // Hearing
                "hearingRange", &PerceptionSettings::hearingRange,
                "hearingSensitivity", &PerceptionSettings::hearingSensitivity,
                
                // Timing
                "updateInterval", &PerceptionSettings::updateInterval,
                "targetMemoryDuration", &PerceptionSettings::targetMemoryDuration
            );

            // PerceivedTarget
            lua.new_usertype<PerceivedTarget>("PerceivedTarget",
                sol::constructors<PerceivedTarget()>(),
                "target", &PerceivedTarget::target,
                "lastSeenPosition", &PerceivedTarget::lastSeenPosition,
                "lastSeenTime", &PerceivedTarget::lastSeenTime,
                "confidence", &PerceivedTarget::confidence,
                "inSight", &PerceivedTarget::inSight,
                "inHearing", &PerceivedTarget::inHearing
            );
        }

        static void BindPerceptionComponent(sol::state& lua) {
            lua.new_usertype<PerceptionComponent>("PerceptionComponent",
                sol::constructors<PerceptionComponent()>(),
                
                // Settings
                "settings", &PerceptionComponent::settings,
                "enabled", &PerceptionComponent::enabled,
                "debugDraw", &PerceptionComponent::debugDraw,
                
                // Targets
                "perceivedTargets", &PerceptionComponent::perceivedTargets,
                "GetNearestVisibleTarget", &PerceptionComponent::GetNearestVisibleTarget,
                "FindTarget", &PerceptionComponent::FindTarget,
                "ForgetTarget", &PerceptionComponent::ForgetTarget,
                "ClearTargets", &PerceptionComponent::ClearTargets,
                
                // Vision/Hearing checks
                "IsInVisionCone", &PerceptionComponent::IsInVisionCone,
                "CanHearSound", &PerceptionComponent::CanHearSound
            );
        }

        static void BindEvents(sol::state& lua) {
            // TargetSpottedEvent
            lua.new_usertype<TargetSpottedEvent>("TargetSpottedEvent",
                sol::constructors<TargetSpottedEvent(GameObject*, GameObject*, const Vector2&, float)>(),
                "observer", &TargetSpottedEvent::observer,
                "target", &TargetSpottedEvent::target,
                "targetPosition", &TargetSpottedEvent::targetPosition,
                "distance", &TargetSpottedEvent::distance,
                "GetName", &TargetSpottedEvent::GetName
            );

            // TargetLostEvent
            lua.new_usertype<TargetLostEvent>("TargetLostEvent",
                sol::constructors<TargetLostEvent(GameObject*, GameObject*, const Vector2&)>(),
                "observer", &TargetLostEvent::observer,
                "target", &TargetLostEvent::target,
                "lastKnownPosition", &TargetLostEvent::lastKnownPosition,
                "GetName", &TargetLostEvent::GetName
            );

            // SoundHeardEvent
            lua.new_usertype<SoundHeardEvent>("SoundHeardEvent",
                sol::constructors<SoundHeardEvent(GameObject*, const Vector2&, float, const std::string&)>(),
                "listener", &SoundHeardEvent::listener,
                "soundPosition", &SoundHeardEvent::soundPosition,
                "soundVolume", &SoundHeardEvent::soundVolume,
                "soundType", &SoundHeardEvent::soundType,
                "GetName", &SoundHeardEvent::GetName
            );
        }
    };

    #else

    class PerceptionBindings {
    public:
        static void BindAll(sol::state&) {}
    };

    #endif

} // namespace SAGE
