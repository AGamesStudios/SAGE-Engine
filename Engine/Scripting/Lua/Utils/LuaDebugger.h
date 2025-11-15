#pragma once

#include "Core/Logger.h"
#include "Scripting/Lua/Core/LuaForward.h"
#include <string>
#include <chrono>
#include <unordered_map>

namespace SAGE {
namespace Scripting {

    /**
     * @brief Lua debugging and profiling utilities
     */
    class LuaDebugger {
    public:
        /**
         * @brief Measure execution time of a Lua function
         */
        template<typename... Args>
        static double ProfileFunction(sol::state& lua, const std::string& funcName, Args&&... args) {
#if SAGE_ENABLE_LUA
            auto start = std::chrono::high_resolution_clock::now();

            sol::protected_function func = lua[funcName];
            if (func.valid()) {
                func(std::forward<Args>(args)...);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            return duration.count();
#else
            (void)lua;
            (void)funcName;
            return 0.0;
#endif
        }

        /**
         * @brief Print all global variables in Lua state
         */
        static void PrintGlobals(sol::state& lua) {
#if SAGE_ENABLE_LUA
            for (const auto& pair : lua.globals()) {
                try {
                    SAGE_INFO("Lua global: {0}", pair.first.as<std::string>());
                } catch (...) {
                    // ignore non-string keys
                }
            }
#else
            (void)lua;
#endif
        }

        /**
         * @brief Get memory usage of Lua state (in KB)
         */
        static double GetMemoryUsage(sol::state& lua) {
    #if SAGE_ENABLE_LUA
            return lua_gc(lua.lua_state(), LUA_GCCOUNT, 0);
    #else
            (void)lua;
            return 0.0;
    #endif
        }

        /**
         * @brief Force garbage collection
         */
        static void ForceGC(sol::state& lua) {
    #if SAGE_ENABLE_LUA
            lua_gc(lua.lua_state(), LUA_GCCOLLECT, 0);
    #else
            (void)lua;
    #endif
        }

        /**
         * @brief Enable/disable Lua debug hooks
         */
        static void EnableDebugHooks(sol::state& lua, bool enable) {
#if SAGE_ENABLE_LUA
            if (enable) {
                lua.set_exception_handler([](lua_State*, sol::optional<const std::exception&> maybe_exception, sol::string_view) -> int {
                    if (maybe_exception) {
                        SAGE_ERROR("Lua exception: {0}", maybe_exception->what());
                    }
                    return 0;
                });
            } else {
                lua.set_exception_handler(nullptr);
            }
#else
            (void)lua;
            (void)enable;
#endif
        }

        /**
         * @brief Dump Lua stack for debugging
         */
        static std::string DumpStack(sol::state& lua) {
#if SAGE_ENABLE_LUA
            std::string dump;
            auto L = lua.lua_state();
            int top = lua_gettop(L);
            for (int i = 1; i <= top; ++i) {
                dump += std::to_string(i) + ": " + luaL_typename(L, i);
                if (i < top) {
                    dump += "\n";
                }
            }
            return dump;
#else
            (void)lua;
            return {};
#endif
        }
    };

} // namespace Scripting
} // namespace SAGE
