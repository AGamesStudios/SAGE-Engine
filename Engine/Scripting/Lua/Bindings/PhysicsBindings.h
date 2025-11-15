#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Math/Vector2.h"
#include "Scripting/Lua/Core/LuaForward.h"
#include <entt/entt.hpp>
#include <functional>

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief Collision information passed to Lua callbacks
     */
    struct CollisionInfo {
        entt::entity entityA;
        entt::entity entityB;
        Vector2 normal;
        Vector2 contactPoint;
        float penetration;
        bool isTrigger;
    };

    /**
     * @brief Physics event bindings for Lua
     * 
     * Provides collision callbacks: OnCollisionEnter, OnCollisionStay, OnCollisionExit
     */
    class PhysicsBindings {
    public:
        static void BindAll(sol::state& lua, entt::registry* registry = nullptr) {
            s_Registry = registry;
            
            BindCollisionInfo(lua);
            BindPhysicsCallbacks(lua);
        }

        /**
         * @brief Register Lua callback for collision events
         */
        static void RegisterCollisionCallback(entt::entity entity, const std::string& eventType, sol::function callback) {
            if (!s_Registry) return;

            auto& callbacks = GetCallbacksFor(entity);
            
            if (eventType == "OnCollisionEnter") {
                callbacks.onCollisionEnter = callback;
            }
            else if (eventType == "OnCollisionStay") {
                callbacks.onCollisionStay = callback;
            }
            else if (eventType == "OnCollisionExit") {
                callbacks.onCollisionExit = callback;
            }
            else if (eventType == "OnTriggerEnter") {
                callbacks.onTriggerEnter = callback;
            }
            else if (eventType == "OnTriggerExit") {
                callbacks.onTriggerExit = callback;
            }
        }

        /**
         * @brief Invoke collision callback from C++ physics system
         */
        static void InvokeCollisionEnter(entt::entity entityA, entt::entity entityB, const CollisionInfo& info) {
            InvokeCallback(entityA, "enter", info);
            InvokeCallback(entityB, "enter", info);
        }

        static void InvokeCollisionStay(entt::entity entityA, entt::entity entityB, const CollisionInfo& info) {
            InvokeCallback(entityA, "stay", info);
            InvokeCallback(entityB, "stay", info);
        }

        static void InvokeCollisionExit(entt::entity entityA, entt::entity entityB, const CollisionInfo& info) {
            InvokeCallback(entityA, "exit", info);
            InvokeCallback(entityB, "exit", info);
        }

    private:
        struct EntityCallbacks {
            sol::function onCollisionEnter;
            sol::function onCollisionStay;
            sol::function onCollisionExit;
            sol::function onTriggerEnter;
            sol::function onTriggerExit;
        };

        static entt::registry* s_Registry;
        static std::unordered_map<entt::entity, EntityCallbacks> s_CollisionCallbacks;

        static EntityCallbacks& GetCallbacksFor(entt::entity entity) {
            return s_CollisionCallbacks[entity];
        }

        static void InvokeCallback(entt::entity entity, const std::string& type, const CollisionInfo& info) {
            auto it = s_CollisionCallbacks.find(entity);
            if (it == s_CollisionCallbacks.end()) return;

            auto& callbacks = it->second;
            sol::function* callback = nullptr;

            if (type == "enter") {
                callback = info.isTrigger ? &callbacks.onTriggerEnter : &callbacks.onCollisionEnter;
            }
            else if (type == "stay") {
                callback = &callbacks.onCollisionStay;
            }
            else if (type == "exit") {
                callback = info.isTrigger ? &callbacks.onTriggerExit : &callbacks.onCollisionExit;
            }

            if (callback && callback->valid()) {
                try {
                    (*callback)(info);
                }
                catch (const sol::error& e) {
                    SAGE_ERROR("Lua collision callback error: {}", e.what());
                }
            }
        }

        static void BindCollisionInfo(sol::state& lua) {
            lua.new_usertype<CollisionInfo>("CollisionInfo",
                "entityA", &CollisionInfo::entityA,
                "entityB", &CollisionInfo::entityB,
                "normal", &CollisionInfo::normal,
                "contactPoint", &CollisionInfo::contactPoint,
                "penetration", &CollisionInfo::penetration,
                "isTrigger", &CollisionInfo::isTrigger
            );
        }

        static void BindPhysicsCallbacks(sol::state& lua) {
            // Physics callback registration API
            auto physics = lua.create_table();
            
            physics["RegisterCollisionEnter"] = [](entt::entity entity, sol::function callback) {
                RegisterCollisionCallback(entity, "OnCollisionEnter", callback);
            };
            
            physics["RegisterCollisionStay"] = [](entt::entity entity, sol::function callback) {
                RegisterCollisionCallback(entity, "OnCollisionStay", callback);
            };
            
            physics["RegisterCollisionExit"] = [](entt::entity entity, sol::function callback) {
                RegisterCollisionCallback(entity, "OnCollisionExit", callback);
            };
            
            physics["RegisterTriggerEnter"] = [](entt::entity entity, sol::function callback) {
                RegisterCollisionCallback(entity, "OnTriggerEnter", callback);
            };
            
            physics["RegisterTriggerExit"] = [](entt::entity entity, sol::function callback) {
                RegisterCollisionCallback(entity, "OnTriggerExit", callback);
            };
            
            lua["Physics"] = physics;
        }
    };

    // Static members
    entt::registry* PhysicsBindings::s_Registry = nullptr;
    std::unordered_map<entt::entity, PhysicsBindings::EntityCallbacks> PhysicsBindings::s_CollisionCallbacks;

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    struct CollisionInfo {
        entt::entity entityA{};
        entt::entity entityB{};
        Vector2 normal{};
        Vector2 contactPoint{};
        float penetration = 0.0f;
        bool isTrigger = false;
    };

    class PhysicsBindings {
    public:
        static void BindAll(sol::state&, entt::registry* = nullptr) {}
        static void RegisterCollisionCallback(entt::entity, const std::string&, sol::function) {}
        static void InvokeCollisionEnter(entt::entity, entt::entity, const CollisionInfo&) {}
        static void InvokeCollisionStay(entt::entity, entt::entity, const CollisionInfo&) {}
        static void InvokeCollisionExit(entt::entity, entt::entity, const CollisionInfo&) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif
