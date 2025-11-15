#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Engine/Scripting/Lua/Core/LuaForward.h"
#include "EntityBindings.h"
#include "ComponentBindings.h"
#include "InputBindings.h"
#include "GraphicsBindings.h"
#include "PhysicsBindings.h"
#include "RPGBindings.h"
#include "AIBindings.h"
#include "AudioBindings.h"
#include "PerceptionBindings.h"
#include "LifecycleBindings.h"
#include "Core/Logger.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Math.h"
#include "Input/InputAction.h"
#include "Core/Color.h"
#include <cmath>
#include <memory>

namespace SAGE {
namespace Scripting {

    /**
     * @brief Lua Bindings - exposes SAGE Engine API to Lua scripts
     * 
     * Provides bindings for:
     * - Math (Vector2, Vector3, Math functions)
     * - Input (Keyboard, Mouse, Gamepad)
     * - Graphics (Color, basic rendering)
     * - Physics (RigidBody, Collider)
     * - Audio (Sound playback)
     * - Logger (Debug logging)
     */
    class LuaBindings {
    public:
#if SAGE_ENABLE_LUA
        /**
         * @brief Bind all engine systems to Lua state
         */
        static void BindAll(sol::state& lua, entt::registry* registry = nullptr, 
                           InputBridge* inputBridge = nullptr,
                           std::shared_ptr<Scripting::ScriptVariables> vars = nullptr,
                           std::shared_ptr<Scripting::ScriptLifecycle> lifecycle = nullptr,
                           std::shared_ptr<Scripting::GameStateManager> stateManager = nullptr,
                           std::shared_ptr<Scripting::SceneManager> sceneManager = nullptr) {
            BindMath(lua);
            BindLogger(lua);
            BindUtilities(lua);
            
            // Bind variable system
            if (vars) {
                Scripting::ScriptVariables::BindToLua(lua, vars);
            }
            
            // Bind lifecycle managers
            if (lifecycle && stateManager && sceneManager) {
                LifecycleBindings::BindAll(lua, lifecycle, stateManager, sceneManager);
            }
            
            // Bind via specialized classes
            if (registry) {
                EntityBindings::BindAll(lua, *registry);
                PhysicsBindings::BindAll(lua, registry);
            }
            ComponentBindings::BindAll(lua);
            InputBindings::BindAll(lua, inputBridge);
            GraphicsBindings::BindAll(lua);
            RPGBindings::BindAll(lua);
            AIBindings::BindAll(lua);  // AI system bindings
            AudioBindings::BindAll(lua);  // Audio bindings (uses ServiceLocator)
            PerceptionBindings::BindAll(lua);  // Perception bindings (vision/hearing)
            
            SAGE_INFO("All Lua bindings registered");
        }

        /**
         * @brief Bind Math types and functions
         */
        static void BindMath(sol::state& lua) {
            // Vector2
            lua.new_usertype<Vector2>("Vector2",
                sol::constructors<Vector2(), Vector2(float, float)>(),
                "x", &Vector2::x,
                "y", &Vector2::y,
                "Length", &Vector2::Length,
                "LengthSquared", &Vector2::LengthSquared,
                "Normalize", &Vector2::Normalize,
                "Normalized", &Vector2::Normalized,
                "Dot", &Vector2::Dot,
                "Distance", &Vector2::Distance,
                "DistanceSquared", &Vector2::DistanceSquared,
                // Operators
                sol::meta_function::addition, [](const Vector2& a, const Vector2& b) { return a + b; },
                sol::meta_function::subtraction, [](const Vector2& a, const Vector2& b) { return a - b; },
                sol::meta_function::multiplication, sol::overload(
                    [](const Vector2& v, float s) { return v * s; },
                    [](float s, const Vector2& v) { return v * s; }
                ),
                sol::meta_function::division, [](const Vector2& v, float s) { return v / s; },
                sol::meta_function::to_string, [](const Vector2& v) {
                    return "Vector2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
                }
            );

            // Vector2 static constants
            lua["Vector2"]["Zero"] = Vector2::Zero;
            lua["Vector2"]["One"] = Vector2::One;
            lua["Vector2"]["Up"] = Vector2::Up;
            lua["Vector2"]["Down"] = Vector2::Down;
            lua["Vector2"]["Left"] = Vector2::Left;
            lua["Vector2"]["Right"] = Vector2::Right;

            // Vector3
            lua.new_usertype<Vector3>("Vector3",
                sol::constructors<Vector3(), Vector3(float, float, float)>(),
                "x", &Vector3::x,
                "y", &Vector3::y,
                "z", &Vector3::z,
                // Operators
                sol::meta_function::addition, [](const Vector3& a, const Vector3& b) { return a + b; },
                sol::meta_function::subtraction, [](const Vector3& a, const Vector3& b) { return a - b; },
                sol::meta_function::multiplication, sol::overload(
                    [](const Vector3& v, float s) { return v * s; },
                    [](float s, const Vector3& v) { return v * s; }
                ),
                sol::meta_function::to_string, [](const Vector3& v) {
                    return "Vector3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
                }
            );

            // Math functions
            lua["Math"] = lua.create_table();
            lua["Math"]["Lerp"] = &Math::Lerp;
            lua["Math"]["Clamp"] = sol::overload(
                [](float value, float min, float max) { return Math::Clamp(value, min, max); },
                [](int value, int min, int max) { return Math::Clamp(value, min, max); }
            );
            lua["Math"]["Min"] = sol::overload(
                [](float a, float b) { return Math::Min(a, b); },
                [](int a, int b) { return Math::Min(a, b); }
            );
            lua["Math"]["Max"] = sol::overload(
                [](float a, float b) { return Math::Max(a, b); },
                [](int a, int b) { return Math::Max(a, b); }
            );
            lua["Math"]["Abs"] = sol::overload(
                [](float v) { return Math::Abs(v); },
                [](int v) { return Math::Abs(v); }
            );
            lua["Math"]["Sin"] = [](float angle) { return std::sin(angle); };
            lua["Math"]["Cos"] = [](float angle) { return std::cos(angle); };
            lua["Math"]["Tan"] = [](float angle) { return std::tan(angle); };
            lua["Math"]["Sqrt"] = [](float v) { return std::sqrt(v); };
            lua["Math"]["Pow"] = [](float base, float exp) { return std::pow(base, exp); };
            lua["Math"]["Floor"] = [](float v) { return std::floor(v); };
            lua["Math"]["Ceil"] = [](float v) { return std::ceil(v); };
            lua["Math"]["Round"] = [](float v) { return std::round(v); };
            
            // Constants
            lua["Math"]["PI"] = Math::Pi;
            lua["Math"]["TAU"] = Math::Tau;
            lua["Math"]["E"] = Math::E;
            lua["Math"]["Deg2Rad"] = Math::Deg2Rad;
            lua["Math"]["Rad2Deg"] = Math::Rad2Deg;
        }

        /**
         * @brief Bind Input system
         */
        static void BindInput(sol::state& lua) {
            // Input action types
            lua.new_enum("ActionType",
                "Press", ActionType::Press,
                "Release", ActionType::Release,
                "Hold", ActionType::Hold
            );

            // Key codes (partial list - extend as needed)
            lua["Key"] = lua.create_table();
            lua["Key"]["Space"] = SAGE_KEY_SPACE;
            lua["Key"]["Enter"] = SAGE_KEY_ENTER;
            lua["Key"]["Escape"] = SAGE_KEY_ESCAPE;
            lua["Key"]["Tab"] = SAGE_KEY_TAB;
            lua["Key"]["Backspace"] = SAGE_KEY_BACKSPACE;
            lua["Key"]["Left"] = SAGE_KEY_LEFT;
            lua["Key"]["Right"] = SAGE_KEY_RIGHT;
            lua["Key"]["Up"] = SAGE_KEY_UP;
            lua["Key"]["Down"] = SAGE_KEY_DOWN;
            lua["Key"]["A"] = SAGE_KEY_A;
            lua["Key"]["B"] = SAGE_KEY_B;
            lua["Key"]["C"] = SAGE_KEY_C;
            lua["Key"]["D"] = SAGE_KEY_D;
            lua["Key"]["E"] = SAGE_KEY_E;
            lua["Key"]["F"] = SAGE_KEY_F;
            lua["Key"]["G"] = SAGE_KEY_G;
            lua["Key"]["H"] = SAGE_KEY_H;
            lua["Key"]["I"] = SAGE_KEY_I;
            lua["Key"]["J"] = SAGE_KEY_J;
            lua["Key"]["K"] = SAGE_KEY_K;
            lua["Key"]["L"] = SAGE_KEY_L;
            lua["Key"]["M"] = SAGE_KEY_M;
            lua["Key"]["N"] = SAGE_KEY_N;
            lua["Key"]["O"] = SAGE_KEY_O;
            lua["Key"]["P"] = SAGE_KEY_P;
            lua["Key"]["Q"] = SAGE_KEY_Q;
            lua["Key"]["R"] = SAGE_KEY_R;
            lua["Key"]["S"] = SAGE_KEY_S;
            lua["Key"]["T"] = SAGE_KEY_T;
            lua["Key"]["U"] = SAGE_KEY_U;
            lua["Key"]["V"] = SAGE_KEY_V;
            lua["Key"]["W"] = SAGE_KEY_W;
            lua["Key"]["X"] = SAGE_KEY_X;
            lua["Key"]["Y"] = SAGE_KEY_Y;
            lua["Key"]["Z"] = SAGE_KEY_Z;
            lua["Key"]["Shift"] = SAGE_KEY_LEFT_SHIFT;
            lua["Key"]["Ctrl"] = SAGE_KEY_LEFT_CONTROL;
            lua["Key"]["Alt"] = SAGE_KEY_LEFT_ALT;

            // Note: Actual input polling requires integration with InputManager
            // This provides the API structure for future implementation
        }

        /**
         * @brief Bind Graphics types
         */
        static void BindGraphics(sol::state& lua) {
            // Color
            lua.new_usertype<Color>("Color",
                sol::constructors<
                    Color(),
                    Color(float, float, float),
                    Color(float, float, float, float)
                >(),
                "r", &Color::r,
                "g", &Color::g,
                "b", &Color::b,
                "a", &Color::a,
                sol::meta_function::to_string, [](const Color& c) {
                    return "Color(" + std::to_string(c.r) + ", " + std::to_string(c.g) + 
                           ", " + std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
                }
            );

            // Predefined colors
            lua["Color"]["White"] = Color::White;
            lua["Color"]["Black"] = Color::Black;
            lua["Color"]["Red"] = Color::Red;
            lua["Color"]["Green"] = Color::Green;
            lua["Color"]["Blue"] = Color::Blue;
            lua["Color"]["Yellow"] = Color::Yellow;
            lua["Color"]["Cyan"] = Color::Cyan;
            lua["Color"]["Magenta"] = Color::Magenta;
        }

        /**
         * @brief Bind Logger for debug output
         */
        static void BindLogger(sol::state& lua) {
            lua["Log"] = lua.create_table();
            lua["Log"]["Info"] = [](const std::string& msg) { SAGE_INFO("[Lua] {}", msg); };
            lua["Log"]["Warning"] = [](const std::string& msg) { SAGE_WARNING("[Lua] {}", msg); };
            lua["Log"]["Error"] = [](const std::string& msg) { SAGE_ERROR("[Lua] {}", msg); };
            lua["Log"]["Trace"] = [](const std::string& msg) { SAGE_TRACE("[Lua] {}", msg); };
        }

        /**
         * @brief Bind utility functions
         */
        static void BindUtilities(sol::state& lua) {
            lua["Engine"] = lua.create_table();
            lua["Engine"]["GetDeltaTime"] = []() -> float { return 0.016f; }; // Placeholder
            lua["Engine"]["GetTime"] = []() -> float { return 0.0f; }; // Placeholder
            lua["Engine"]["Quit"] = []() { SAGE_INFO("[Lua] Quit requested"); };
        }
#else
        static void BindAll(sol::state&, entt::registry* = nullptr,
                            InputBridge* = nullptr,
                            std::shared_ptr<Scripting::ScriptVariables> = nullptr,
                            std::shared_ptr<Scripting::ScriptLifecycle> = nullptr,
                            std::shared_ptr<Scripting::GameStateManager> = nullptr,
                            std::shared_ptr<Scripting::SceneManager> = nullptr) {}

        static void BindMath(sol::state&) {}
        static void BindInput(sol::state&) {}
        static void BindGraphics(sol::state&) {}
        static void BindLogger(sol::state&) {}
        static void BindUtilities(sol::state&) {}
#endif
    };

} // namespace Scripting
} // namespace SAGE
