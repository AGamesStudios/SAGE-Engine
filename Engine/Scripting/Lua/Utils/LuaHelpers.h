#pragma once

#include "Scripting/Lua/Core/LuaForward.h"
#include <string>
#include <optional>

namespace SAGE {
namespace Scripting {

    /**
     * @brief Lua Helper utilities for common operations
     * 
     * Provides convenience functions for working with sol2 and Lua types.
     */
    class LuaHelpers {
    public:
        /**
         * @brief Safely get a global variable from Lua
         */
        template<typename T>
        static std::optional<T> SafeGetGlobal(sol::state& lua, const std::string& name) {
#if SAGE_ENABLE_LUA
            try {
                sol::object obj = lua[name];
                if (obj.valid() && obj.is<T>()) {
                    return obj.as<T>();
                }
            }
            catch (...) {
                return std::nullopt;
            }
#else
            (void)lua;
            (void)name;
#endif
            return std::nullopt;
        }

        /**
         * @brief Safely call a Lua function
         */
        template<typename... Args>
        static bool SafeCallFunction(sol::state& lua, const std::string& funcName, Args&&... args) {
#if SAGE_ENABLE_LUA
            try {
                sol::protected_function func = lua[funcName];
                if (!func.valid()) return false;

                auto result = func(std::forward<Args>(args)...);
                return result.valid();
            }
            catch (const sol::error&) {
                return false;
            }
#else
            (void)lua;
            (void)funcName;
            return false;
#endif
        }

        /**
         * @brief Safely call a Lua function with return value
         */
        template<typename R, typename... Args>
        static std::optional<R> SafeCallFunctionWithReturn(sol::state& lua, const std::string& funcName, Args&&... args) {
#if SAGE_ENABLE_LUA
            try {
                sol::protected_function func = lua[funcName];
                if (!func.valid()) return std::nullopt;

                auto result = func(std::forward<Args>(args)...);
                if (!result.valid()) return std::nullopt;

                return result.get<R>();
            }
            catch (const sol::error&) {
                return std::nullopt;
            }
#else
            (void)lua;
            (void)funcName;
            return std::nullopt;
#endif
        }

        /**
         * @brief Check if a table has a specific key
         */
        static bool TableHasKey(sol::table& table, const std::string& key) {
#if SAGE_ENABLE_LUA
            return table[key].valid();
#else
            (void)table;
            (void)key;
            return false;
#endif
        }

        /**
         * @brief Get table size
         */
        static size_t GetTableSize(sol::table& table) {
#if SAGE_ENABLE_LUA
            return table.size();
#else
            (void)table;
            return 0;
#endif
        }

        /**
         * @brief Pretty print Lua table (for debugging)
         */
        static std::string TableToString(sol::table& table, int indent = 0);
    };

} // namespace Scripting
} // namespace SAGE
