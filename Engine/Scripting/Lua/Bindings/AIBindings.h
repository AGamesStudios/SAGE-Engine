#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "AI/Pathfinder.h"
#include "AI/AIBlackboard.h"
#include "AI/SteeringBehaviors.h"
#include "AI/BehaviorTree.h"
#include "Math/Vector2.h"
#include "Engine/Scripting/Lua/Core/LuaForward.h"

namespace SAGE {
namespace Scripting {

    /**
     * @brief AI System bindings for Lua
     * 
     * Binds Pathfinding, Steering Behaviors, Blackboard, and Behavior Trees.
     */
    class AIBindings {
    public:
#if SAGE_ENABLE_LUA
        static void BindAll(sol::state& lua) {
            BindPathfinder(lua);
            BindSteeringBehaviors(lua);
            BindBlackboard(lua);
            BindBehaviorTree(lua);
        }

    private:
        static void BindPathfinder(sol::state& lua) {
            // Pathfinder::Path
            lua.new_usertype<Pathfinder::Path>("Path",
                "waypoints", &Pathfinder::Path::waypoints,
                "totalCost", &Pathfinder::Path::totalCost,
                "found", &Pathfinder::Path::found,
                "IsEmpty", &Pathfinder::Path::IsEmpty,
                "Size", &Pathfinder::Path::Size
            );

            // Pathfinder
            lua.new_usertype<Pathfinder>("Pathfinder",
                sol::constructors<Pathfinder(int, int, float)>(),
                
                // Grid setup
                "SetWalkable", &Pathfinder::SetWalkable,
                "IsWalkable", &Pathfinder::IsWalkable,
                "SetWalkableRegion", &Pathfinder::SetWalkableRegion,
                "Clear", &Pathfinder::Clear,
                
                // Pathfinding
                "FindPath", sol::overload(
                    [](Pathfinder& pf, const Vector2& start, const Vector2& goal) {
                        return pf.FindPath(start, goal);
                    },
                    [](Pathfinder& pf, const Vector2& start, const Vector2& goal, bool diagonal) {
                        return pf.FindPath(start, goal, diagonal);
                    },
                    [](Pathfinder& pf, const Vector2& start, const Vector2& goal, bool diagonal, bool smooth) {
                        return pf.FindPath(start, goal, diagonal, smooth);
                    }
                ),
                
                // Utilities
                "WorldToGrid", &Pathfinder::WorldToGrid,
                "GridToWorld", &Pathfinder::GridToWorld,
                "GetGridWidth", &Pathfinder::GetGridWidth,
                "GetGridHeight", &Pathfinder::GetGridHeight,
                "GetTileSize", &Pathfinder::GetTileSize
            );
        }

        static void BindSteeringBehaviors(sol::state& lua) {
            // SteeringOutput
            lua.new_usertype<SteeringBehaviors::SteeringOutput>("SteeringOutput",
                sol::constructors<SteeringBehaviors::SteeringOutput(), 
                                 SteeringBehaviors::SteeringOutput(const Vector2&, float)>(),
                "linear", &SteeringBehaviors::SteeringOutput::linear,
                "angular", &SteeringBehaviors::SteeringOutput::angular
            );

            // Agent
            lua.new_usertype<SteeringBehaviors::Agent>("SteeringAgent",
                sol::constructors<SteeringBehaviors::Agent()>(),
                "position", &SteeringBehaviors::Agent::position,
                "velocity", &SteeringBehaviors::Agent::velocity,
                "rotation", &SteeringBehaviors::Agent::rotation,
                "maxSpeed", &SteeringBehaviors::Agent::maxSpeed,
                "maxAcceleration", &SteeringBehaviors::Agent::maxAcceleration,
                "maxAngularSpeed", &SteeringBehaviors::Agent::maxAngularSpeed
            );

            // Circle (for obstacle avoidance)
            lua.new_usertype<SteeringBehaviors::Circle>("SteeringCircle",
                sol::constructors<SteeringBehaviors::Circle(const Vector2&, float)>(),
                "center", &SteeringBehaviors::Circle::center,
                "radius", &SteeringBehaviors::Circle::radius
            );

            // SteeringBehaviors static methods
            auto steering = lua.create_table();
            
            // Basic behaviors
            steering["Seek"] = &SteeringBehaviors::Seek;
            
            steering["Flee"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const Vector2& target) {
                    return SteeringBehaviors::Flee(agent, target);
                },
                [](const SteeringBehaviors::Agent& agent, const Vector2& target, float panicDistance) {
                    return SteeringBehaviors::Flee(agent, target, panicDistance);
                }
            );
            
            steering["Wander"] = &SteeringBehaviors::Wander;
            
            steering["Arrival"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const Vector2& target) {
                    return SteeringBehaviors::Arrival(agent, target);
                },
                [](const SteeringBehaviors::Agent& agent, const Vector2& target, float slowRadius) {
                    return SteeringBehaviors::Arrival(agent, target, slowRadius);
                },
                [](const SteeringBehaviors::Agent& agent, const Vector2& target, float slowRadius, float stopRadius) {
                    return SteeringBehaviors::Arrival(agent, target, slowRadius, stopRadius);
                }
            );

            // Advanced behaviors
            steering["Pursue"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos, const Vector2& targetVel) {
                    return SteeringBehaviors::Pursue(agent, targetPos, targetVel);
                },
                [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos, const Vector2& targetVel, float maxPrediction) {
                    return SteeringBehaviors::Pursue(agent, targetPos, targetVel, maxPrediction);
                }
            );

            steering["Evade"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos, const Vector2& targetVel) {
                    return SteeringBehaviors::Evade(agent, targetPos, targetVel);
                },
                [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos, const Vector2& targetVel, float maxPrediction) {
                    return SteeringBehaviors::Evade(agent, targetPos, targetVel, maxPrediction);
                }
            );

            steering["AvoidObstacles"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const std::vector<SteeringBehaviors::Circle>& obstacles) {
                    return SteeringBehaviors::AvoidObstacles(agent, obstacles);
                },
                [](const SteeringBehaviors::Agent& agent, const std::vector<SteeringBehaviors::Circle>& obstacles, float avoidDistance) {
                    return SteeringBehaviors::AvoidObstacles(agent, obstacles, avoidDistance);
                }
            );

            steering["Separation"] = sol::overload(
                [](const SteeringBehaviors::Agent& agent, const std::vector<SteeringBehaviors::Agent>& neighbors) {
                    return SteeringBehaviors::Separation(agent, neighbors);
                },
                [](const SteeringBehaviors::Agent& agent, const std::vector<SteeringBehaviors::Agent>& neighbors, float separationRadius) {
                    return SteeringBehaviors::Separation(agent, neighbors, separationRadius);
                }
            );

            steering["Face"] = &SteeringBehaviors::Face;

            // Utility functions
            steering["Combine"] = &SteeringBehaviors::Combine;
            steering["ApplySteering"] = &SteeringBehaviors::ApplySteering;
            
            lua["Steering"] = steering;
        }

        static void BindBlackboard(sol::state& lua) {
            // Basic Blackboard
            lua.new_usertype<Blackboard>("Blackboard",
                sol::constructors<Blackboard()>(),
                "Has", &Blackboard::Has,
                "Clear", &Blackboard::Clear,
                "Remove", &Blackboard::Remove,
                
                // Generic get/set via tables
                "SetFloat", [](Blackboard& bb, const std::string& key, float value) {
                    bb.Set(key, value);
                },
                "GetFloat", [](Blackboard& bb, const std::string& key, float defaultVal) {
                    return bb.Get<float>(key, defaultVal);
                },
                "SetInt", [](Blackboard& bb, const std::string& key, int value) {
                    bb.Set(key, value);
                },
                "GetInt", [](Blackboard& bb, const std::string& key, int defaultVal) {
                    return bb.Get<int>(key, defaultVal);
                },
                "SetBool", [](Blackboard& bb, const std::string& key, bool value) {
                    bb.Set(key, value);
                },
                "GetBool", [](Blackboard& bb, const std::string& key, bool defaultVal) {
                    return bb.Get<bool>(key, defaultVal);
                },
                "SetString", [](Blackboard& bb, const std::string& key, const std::string& value) {
                    bb.Set(key, value);
                },
                "GetString", [](Blackboard& bb, const std::string& key, const std::string& defaultVal) {
                    return bb.Get<std::string>(key, defaultVal);
                },
                "SetVector2", [](Blackboard& bb, const std::string& key, const Vector2& value) {
                    bb.Set(key, value);
                },
                "GetVector2", [](Blackboard& bb, const std::string& key, const Vector2& defaultVal) {
                    return bb.Get<Vector2>(key, defaultVal);
                }
            );

            // AIBlackboard (extended)
            lua.new_usertype<AIBlackboard>("AIBlackboard",
                sol::constructors<AIBlackboard()>(),
                sol::base_classes, sol::bases<Blackboard>(),
                
                // Target tracking
                "HasTarget", &AIBlackboard::HasTarget,
                "SetLastKnownTargetPosition", &AIBlackboard::SetLastKnownTargetPosition,
                "GetLastKnownTargetPosition", &AIBlackboard::GetLastKnownTargetPosition,
                
                // Threat
                "SetThreatLevel", &AIBlackboard::SetThreatLevel,
                "GetThreatLevel", &AIBlackboard::GetThreatLevel,
                
                // Patrol
                "AddPatrolPoint", &AIBlackboard::AddPatrolPoint,
                "GetPatrolPoint", &AIBlackboard::GetPatrolPoint,
                "GetPatrolPointCount", &AIBlackboard::GetPatrolPointCount,
                "SetCurrentPatrolIndex", &AIBlackboard::SetCurrentPatrolIndex,
                "GetCurrentPatrolIndex", &AIBlackboard::GetCurrentPatrolIndex,
                "NextPatrolPoint", &AIBlackboard::NextPatrolPoint,
                "PreviousPatrolPoint", &AIBlackboard::PreviousPatrolPoint,
                
                // State
                "SetState", &AIBlackboard::SetState,
                "GetState", &AIBlackboard::GetState,
                
                // Health
                "SetHealth", &AIBlackboard::SetHealth,
                "GetHealth", &AIBlackboard::GetHealth,
                "IsLowHealth", &AIBlackboard::IsLowHealth
            );
        }

        static void BindBehaviorTree(sol::state& lua) {
            // BehaviorStatus enum
            lua.new_enum("BehaviorStatus",
                "Success", BehaviorStatus::Success,
                "Failure", BehaviorStatus::Failure,
                "Running", BehaviorStatus::Running
            );

            // BehaviorNode (base) - read-only access for Lua
            lua.new_usertype<BehaviorNode>("BehaviorNode",
                "Tick", &BehaviorNode::Tick,
                "Reset", &BehaviorNode::Reset,
                "SetName", &BehaviorNode::SetName,
                "GetName", &BehaviorNode::GetName
            );

            // SequenceNode
            lua.new_usertype<SequenceNode>("SequenceNode",
                sol::constructors<SequenceNode()>(),
                sol::base_classes, sol::bases<BehaviorNode>(),
                "AddChild", &SequenceNode::AddChild
            );

            // SelectorNode  
            lua.new_usertype<SelectorNode>("SelectorNode",
                sol::constructors<SelectorNode()>(),
                sol::base_classes, sol::bases<BehaviorNode>(),
                "AddChild", &SelectorNode::AddChild
            );
        }
#else
        static void BindAll(sol::state&) {}
#endif
    };

} // namespace Scripting
} // namespace SAGE
