#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Core/Color.h"
#include "Input/Input.h"
#include "Input/Key.h"
#include "Input/MouseButton.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "Dialogue/DialogueManager.h"
#include "Inventory/ItemDatabase.h"
#include "Quests/QuestManager.h"
#include "AI/SteeringBehaviors.h"
#include "AI/Pathfinder.h"
#include "AI/AIBlackboard.h"
#include <entt/entt.hpp>

namespace SAGE {

#if SAGE_ENABLE_LUA

    /**
     * @brief LuaBindings - expose C++ API to Lua scripts
     * 
     * Binds core engine systems to Lua for scripting gameplay logic.
     * Call RegisterBindings() once at engine startup.
     * 
     * Usage in Lua:
     *   -- Math
     *   local pos = Float2.new(100, 200)
     *   pos.x = pos.x + 10
     *   
     *   -- Input
     *   if Input.IsKeyPressed(Key.Space) then
     *       player:Jump()
     *   end
     *   
     *   -- Entity
     *   local transform = entity:GetTransform()
     *   transform.position.y = transform.position.y + speed * dt
     */
    class LuaBindings {
    public:
        static void RegisterBindings(LuaVM& vm) {
            auto& lua = vm.GetState();
            
            RegisterMathTypes(lua);
            RegisterInput(lua);
            RegisterEntity(lua);
            RegisterComponents(lua);
            RegisterDialogue(lua);
            RegisterInventory(lua);
            RegisterQuests(lua);
            RegisterAI(lua);
            
            SAGE_INFO("Lua API bindings registered");
        }
        
    private:
        static void RegisterMathTypes(sol::state& lua) {
            // Float2
            lua.new_usertype<Float2>("Float2",
                sol::constructors<Float2(), Float2(float, float)>(),
                "x", &Float2::x,
                "y", &Float2::y,
                "Length", &Float2::Length,
                "Normalize", &Float2::Normalize,
                sol::meta_function::addition, [](const Float2& a, const Float2& b) { return a + b; },
                sol::meta_function::subtraction, [](const Float2& a, const Float2& b) { return a - b; },
                sol::meta_function::multiplication, sol::overload(
                    [](const Float2& a, float b) { return a * b; },
                    [](const Float2& a, const Float2& b) { return Float2{a.x * b.x, a.y * b.y}; }
                )
            );
            
            // Color
            lua.new_usertype<Color>("Color",
                sol::constructors<Color(), Color(float, float, float, float)>(),
                "r", &Color::r,
                "g", &Color::g,
                "b", &Color::b,
                "a", &Color::a,
                "White", &Color::White,
                "Black", &Color::Black,
                "Red", &Color::Red,
                "Green", &Color::Green,
                "Blue", &Color::Blue
            );
            
            // Rect
            lua.new_usertype<Rect>("Rect",
                sol::constructors<Rect(), Rect(float, float, float, float)>(),
                "x", &Rect::x,
                "y", &Rect::y,
                "width", &Rect::width,
                "height", &Rect::height
            );
        }
        
        static void RegisterInput(sol::state& lua) {
            // Key enum
            lua.new_enum("Key",
                "Space", Key::Space,
                "Enter", Key::Enter,
                "Escape", Key::Escape,
                "W", Key::W,
                "A", Key::A,
                "S", Key::S,
                "D", Key::D,
                "Up", Key::Up,
                "Down", Key::Down,
                "Left", Key::Left,
                "Right", Key::Right
            );
            
            // MouseButton enum
            lua.new_enum("MouseButton",
                "Left", MouseButton::Left,
                "Right", MouseButton::Right,
                "Middle", MouseButton::Middle
            );
            
            // Input static functions
            auto inputTable = lua.create_table();
            inputTable["IsKeyPressed"] = &Input::IsKeyPressed;
            inputTable["IsKeyJustPressed"] = &Input::IsKeyJustPressed;
            inputTable["IsKeyJustReleased"] = &Input::IsKeyJustReleased;
            inputTable["IsMouseButtonPressed"] = &Input::IsMouseButtonPressed;
            inputTable["IsMouseButtonJustPressed"] = &Input::IsMouseButtonJustPressed;
            inputTable["GetMousePosition"] = &Input::GetMousePosition;
            inputTable["GetMouseDelta"] = &Input::GetMouseDelta;
            lua["Input"] = inputTable;
        }
        
        static void RegisterEntity(sol::state& lua) {
            // GameObject
            lua.new_usertype<GameObject>("GameObject",
                "GetName", &GameObject::GetName,
                "SetName", &GameObject::SetName,
                "IsActive", &GameObject::IsActive,
                "SetActive", &GameObject::SetActive,
                "GetTag", &GameObject::GetTag,
                "SetTag", &GameObject::SetTag,
                "GetLayer", &GameObject::GetLayer,
                "SetLayer", &GameObject::SetLayer
            );
        }
        
        static void RegisterComponents(sol::state& lua) {
            // TransformComponent
            lua.new_usertype<TransformComponent>("TransformComponent",
                "position", &TransformComponent::position,
                "rotation", &TransformComponent::rotation,
                "scale", &TransformComponent::scale
            );
            
            // SpriteComponent
            lua.new_usertype<SpriteComponent>("SpriteComponent",
                "color", &SpriteComponent::color,
                "flipX", &SpriteComponent::flipX,
                "flipY", &SpriteComponent::flipY,
                "layer", &SpriteComponent::layer
            );
            
            // StatsComponent
            lua.new_usertype<StatsComponent>("Stats",
                "maxHealth", &StatsComponent::maxHealth,
                "currentHealth", &StatsComponent::currentHealth,
                "maxMana", &StatsComponent::maxMana,
                "currentMana", &StatsComponent::currentMana,
                "maxStamina", &StatsComponent::maxStamina,
                "currentStamina", &StatsComponent::currentStamina,
                "armor", &StatsComponent::armor,
                "resistance", &StatsComponent::resistance,
                "IsAlive", &StatsComponent::IsAlive,
                "GetHealthPercent", &StatsComponent::GetHealthPercent,
                "GetManaPercent", &StatsComponent::GetManaPercent,
                "GetStaminaPercent", &StatsComponent::GetStaminaPercent,
                "SetStat", &StatsComponent::SetStat,
                "GetStat", &StatsComponent::GetStat
            );
        }
        
        static void RegisterDialogue(sol::state& lua) {
            // DialogueManager
            auto dialogueTable = lua.create_table();
            dialogueTable["LoadDialogue"] = [](const std::string& name, const std::string& path) {
                return DialogueManager::Get().LoadDialogue(name, path);
            };
            dialogueTable["StartDialogue"] = [](const std::string& name) {
                return DialogueManager::Get().StartDialogue(name);
            };
            dialogueTable["EndDialogue"] = []() {
                DialogueManager::Get().EndDialogue();
            };
            dialogueTable["SelectChoice"] = [](int index) {
                return DialogueManager::Get().SelectChoice(index);
            };
            dialogueTable["IsDialogueActive"] = []() {
                return DialogueManager::Get().IsDialogueActive();
            };
            dialogueTable["SetBool"] = [](const std::string& key, bool value) {
                DialogueManager::Get().GetVariables().SetBool(key, value);
            };
            dialogueTable["GetBool"] = [](const std::string& key) {
                return DialogueManager::Get().GetVariables().GetBool(key);
            };
            dialogueTable["SetInt"] = [](const std::string& key, int value) {
                DialogueManager::Get().GetVariables().SetInt(key, value);
            };
            dialogueTable["GetInt"] = [](const std::string& key) {
                return DialogueManager::Get().GetVariables().GetInt(key);
            };
            lua["Dialogue"] = dialogueTable;
        }
        
        static void RegisterInventory(sol::state& lua) {
            // ItemType enum
            lua.new_enum("ItemType",
                "Consumable", ItemType::Consumable,
                "Equipment", ItemType::Equipment,
                "QuestItem", ItemType::QuestItem,
                "Material", ItemType::Material,
                "Misc", ItemType::Misc
            );
            
            // ItemRarity enum
            lua.new_enum("ItemRarity",
                "Common", ItemRarity::Common,
                "Uncommon", ItemRarity::Uncommon,
                "Rare", ItemRarity::Rare,
                "Epic", ItemRarity::Epic,
                "Legendary", ItemRarity::Legendary
            );
            
            // Item (read-only access)
            lua.new_usertype<Item>("Item",
                "id", sol::readonly(&Item::id),
                "name", sol::readonly(&Item::name),
                "description", sol::readonly(&Item::description),
                "type", sol::readonly(&Item::type),
                "rarity", sol::readonly(&Item::rarity),
                "value", sol::readonly(&Item::value),
                "attackBonus", sol::readonly(&Item::attackBonus),
                "defenseBonus", sol::readonly(&Item::defenseBonus)
            );
            
            // ItemDatabase
            auto itemDBTable = lua.create_table();
            itemDBTable["LoadFromFile"] = [](const std::string& path) {
                return ItemDatabase::Get().LoadFromFile(path);
            };
            itemDBTable["GetItem"] = [](const std::string& id) {
                return ItemDatabase::Get().GetItem(id);
            };
            itemDBTable["HasItem"] = [](const std::string& id) {
                return ItemDatabase::Get().HasItem(id);
            };
            lua["ItemDatabase"] = itemDBTable;
            
            // Inventory
            lua.new_usertype<Inventory>("Inventory",
                "AddItem", &Inventory::AddItem,
                "RemoveItem", &Inventory::RemoveItem,
                "HasItem", &Inventory::HasItem,
                "GetItemCount", &Inventory::GetItemCount,
                "GetEmptySlotCount", &Inventory::GetEmptySlotCount,
                "GetCurrentWeight", &Inventory::GetCurrentWeight,
                "GetTotalValue", &Inventory::GetTotalValue,
                "SortByType", &Inventory::SortByType,
                "SortByRarity", &Inventory::SortByRarity
            );
        }
        
        static void RegisterQuests(sol::state& lua) {
            using namespace Quests;
            
            // ObjectiveType enum
            lua.new_enum("ObjectiveType",
                "Kill", ObjectiveType::Kill,
                "Collect", ObjectiveType::Collect,
                "TalkTo", ObjectiveType::TalkTo,
                "Reach", ObjectiveType::Reach,
                "Trigger", ObjectiveType::Trigger,
                "Custom", ObjectiveType::Custom
            );
            
            // QuestState enum
            lua.new_enum("QuestState",
                "NotStarted", QuestState::NotStarted,
                "InProgress", QuestState::InProgress,
                "Completed", QuestState::Completed,
                "Failed", QuestState::Failed,
                "TurnedIn", QuestState::TurnedIn
            );
            
            // QuestObjective (read-only for scripting safety)
            lua.new_usertype<QuestObjective>("QuestObjective",
                "GetDescription", &QuestObjective::GetDescription,
                "GetType", &QuestObjective::GetType,
                "GetState", &QuestObjective::GetState,
                "GetCurrentCount", &QuestObjective::GetCurrentCount,
                "GetRequiredCount", &QuestObjective::GetRequiredCount,
                "GetProgress", &QuestObjective::GetProgress,
                "IsCompleted", &QuestObjective::IsCompleted
            );
            
            // Quest (read-only queries)
            lua.new_usertype<Quest>("Quest",
                "GetID", &Quest::GetID,
                "GetTitle", &Quest::GetTitle,
                "GetDescription", &Quest::GetDescription,
                "GetState", &Quest::GetState,
                "GetLevel", &Quest::GetLevel,
                "GetProgress", &Quest::GetProgress,
                "IsActive", &Quest::IsActive,
                "IsCompleted", &Quest::IsCompleted,
                "IsFailed", &Quest::IsFailed
            );
            
            // QuestManager singleton
            auto questMgr = lua.create_table();
            
            questMgr["StartQuest"] = [](const std::string& questID) {
                return QuestManager::Get().StartQuest(questID);
            };
            
            questMgr["CompleteQuest"] = [](const std::string& questID) {
                return QuestManager::Get().CompleteQuest(questID);
            };
            
            questMgr["FailQuest"] = [](const std::string& questID) {
                QuestManager::Get().FailQuest(questID);
            };
            
            questMgr["TurnInQuest"] = [](const std::string& questID) {
                return QuestManager::Get().TurnInQuest(questID);
            };
            
            questMgr["IsQuestActive"] = [](const std::string& questID) {
                return QuestManager::Get().IsQuestActive(questID);
            };
            
            questMgr["IsQuestCompleted"] = [](const std::string& questID) {
                return QuestManager::Get().IsQuestCompleted(questID);
            };
            
            questMgr["GetQuest"] = [](const std::string& questID) -> Quest* {
                return QuestManager::Get().GetQuest(questID);
            };
            
            // Objective tracking helpers
            questMgr["OnEnemyKilled"] = [](const std::string& enemyType) {
                QuestManager::Get().OnEnemyKilled(enemyType);
            };
            
            questMgr["OnItemCollected"] = [](const std::string& itemID) {
                QuestManager::Get().OnItemCollected(itemID);
            };
            
            questMgr["OnNPCTalkedTo"] = [](const std::string& npcID) {
                QuestManager::Get().OnNPCTalkedTo(npcID);
            };
            
            questMgr["OnLocationReached"] = [](const std::string& locationID) {
                QuestManager::Get().OnLocationReached(locationID);
            };
            
            lua["Quests"] = questMgr;
        }

        // ====================================================================
        // AI System Bindings
        // ====================================================================
        static void RegisterAI(sol::state& lua) {
            // Vector2 for AI (if not already registered)
            // Assuming Vector2 registered in RegisterMathTypes

            // ================================================================
            // Steering Behaviors
            // ================================================================
            
            // SteeringAgent
            lua.new_usertype<SteeringBehaviors::Agent>("SteeringAgent",
                sol::constructors<SteeringBehaviors::Agent()>(),
                "position", &SteeringBehaviors::Agent::position,
                "velocity", &SteeringBehaviors::Agent::velocity,
                "rotation", &SteeringBehaviors::Agent::rotation,
                "maxSpeed", &SteeringBehaviors::Agent::maxSpeed,
                "maxAcceleration", &SteeringBehaviors::Agent::maxAcceleration,
                "maxAngularSpeed", &SteeringBehaviors::Agent::maxAngularSpeed
            );

            // SteeringOutput
            lua.new_usertype<SteeringBehaviors::SteeringOutput>("SteeringOutput",
                sol::constructors<
                    SteeringBehaviors::SteeringOutput(),
                    SteeringBehaviors::SteeringOutput(const Vector2&, float)
                >(),
                "linear", &SteeringBehaviors::SteeringOutput::linear,
                "angular", &SteeringBehaviors::SteeringOutput::angular
            );

            // Steering behaviors as global functions
            auto steering = lua.create_table();
            
            steering["Seek"] = [](const SteeringBehaviors::Agent& agent, const Vector2& target) {
                return SteeringBehaviors::Seek(agent, target);
            };
            
            steering["Flee"] = [](const SteeringBehaviors::Agent& agent, const Vector2& target, float panicDist) {
                return SteeringBehaviors::Flee(agent, target, panicDist);
            };
            
            steering["Wander"] = [](const SteeringBehaviors::Agent& agent, float radius, float distance, 
                                   float jitter, Vector2& wanderTarget) {
                return SteeringBehaviors::Wander(agent, radius, distance, jitter, wanderTarget);
            };
            
            steering["Arrival"] = [](const SteeringBehaviors::Agent& agent, const Vector2& target,
                                    float slowRadius, float stopRadius) {
                return SteeringBehaviors::Arrival(agent, target, slowRadius, stopRadius);
            };
            
            steering["Pursue"] = [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos,
                                   const Vector2& targetVel, float maxPrediction) {
                return SteeringBehaviors::Pursue(agent, targetPos, targetVel, maxPrediction);
            };
            
            steering["Evade"] = [](const SteeringBehaviors::Agent& agent, const Vector2& targetPos,
                                  const Vector2& targetVel, float maxPrediction) {
                return SteeringBehaviors::Evade(agent, targetPos, targetVel, maxPrediction);
            };
            
            steering["ApplySteering"] = [](SteeringBehaviors::Agent& agent, 
                                          const SteeringBehaviors::SteeringOutput& steering, 
                                          float deltaTime) {
                SteeringBehaviors::ApplySteering(agent, steering, deltaTime);
            };
            
            lua["Steering"] = steering;

            // ================================================================
            // Pathfinding
            // ================================================================
            
            // Pathfinder::Path
            lua.new_usertype<Pathfinder::Path>("Path",
                "waypoints", &Pathfinder::Path::waypoints,
                "totalCost", &Pathfinder::Path::totalCost,
                "found", &Pathfinder::Path::found,
                "IsEmpty", &Pathfinder::Path::IsEmpty,
                "Size", &Pathfinder::Path::Size
            );

            // Pathfinder (single global instance expected)
            lua.new_usertype<Pathfinder>("Pathfinder",
                sol::constructors<Pathfinder(int, int, float)>(),
                "SetWalkable", &Pathfinder::SetWalkable,
                "IsWalkable", &Pathfinder::IsWalkable,
                "FindPath", &Pathfinder::FindPath,
                "WorldToGrid", [](Pathfinder& pf, const Vector2& worldPos) {
                    int x, y;
                    pf.WorldToGrid(worldPos, x, y);
                    return std::make_tuple(x, y);
                },
                "GridToWorld", &Pathfinder::GridToWorld,
                "Clear", &Pathfinder::Clear,
                "GetWidth", &Pathfinder::GetWidth,
                "GetHeight", &Pathfinder::GetHeight,
                "GetTileSize", &Pathfinder::GetTileSize
            );

            // ================================================================
            // AI Blackboard
            // ================================================================
            
            lua.new_usertype<AIBlackboard>("AIBlackboard",
                sol::constructors<AIBlackboard()>(),
                
                // Target tracking
                "SetTarget", &AIBlackboard::SetTarget,
                "GetTarget", &AIBlackboard::GetTarget,
                "HasTarget", &AIBlackboard::HasTarget,
                "SetLastKnownTargetPosition", &AIBlackboard::SetLastKnownTargetPosition,
                "GetLastKnownTargetPosition", &AIBlackboard::GetLastKnownTargetPosition,
                
                // Threat
                "SetThreatLevel", &AIBlackboard::SetThreatLevel,
                "GetThreatLevel", &AIBlackboard::GetThreatLevel,
                "IsInDanger", &AIBlackboard::IsInDanger,
                
                // Patrol
                "SetPatrolPoints", &AIBlackboard::SetPatrolPoints,
                "GetPatrolPoints", &AIBlackboard::GetPatrolPoints,
                "GetCurrentPatrolPoint", &AIBlackboard::GetCurrentPatrolPoint,
                "NextPatrolPoint", &AIBlackboard::NextPatrolPoint,
                "GetPatrolIndex", &AIBlackboard::GetPatrolIndex,
                "SetPatrolIndex", &AIBlackboard::SetPatrolIndex,
                
                // Path
                "SetPath", &AIBlackboard::SetPath,
                "GetPath", &AIBlackboard::GetPath,
                "GetCurrentWaypoint", &AIBlackboard::GetCurrentWaypoint,
                "NextWaypoint", &AIBlackboard::NextWaypoint,
                "IsPathComplete", &AIBlackboard::IsPathComplete,
                "ClearPath", &AIBlackboard::ClearPath,
                
                // Agent
                "SetAgent", &AIBlackboard::SetAgent,
                "GetAgent", &AIBlackboard::GetAgent,
                
                // Wander
                "SetWanderTarget", &AIBlackboard::SetWanderTarget,
                "GetWanderTarget", &AIBlackboard::GetWanderTarget,
                
                // Flags & timers
                "SetFlag", &AIBlackboard::SetFlag,
                "GetFlag", &AIBlackboard::GetFlag,
                "SetTimer", &AIBlackboard::SetTimer,
                "GetTimer", &AIBlackboard::GetTimer,
                "DecrementTimer", &AIBlackboard::DecrementTimer,
                
                // Combat
                "SetAttackCooldown", &AIBlackboard::SetAttackCooldown,
                "GetAttackCooldown", &AIBlackboard::GetAttackCooldown,
                "CanAttack", &AIBlackboard::CanAttack,
                "SetAttackRange", &AIBlackboard::SetAttackRange,
                "GetAttackRange", &AIBlackboard::GetAttackRange
            );
        }
    };

} // namespace SAGE

#else

namespace SAGE {

    class LuaBindings {
    public:
        static void RegisterBindings(LuaVM&) {
            SAGE_WARN("Lua bindings are disabled (SAGE_ENABLE_LUA=OFF)");
        }
    };

} // namespace SAGE

#endif
