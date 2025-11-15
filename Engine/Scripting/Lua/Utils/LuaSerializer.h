#pragma once

#include "Scripting/Lua/Core/LuaForward.h"
#include <string>
#include <fstream>

namespace SAGE {
namespace Scripting {

    /**
     * @brief Lua table serialization/deserialization
     * 
     * Save and load Lua tables to/from files for data persistence.
     */
    class LuaSerializer {
    public:
        /**
         * @brief Save a Lua table to a file
         * 
         * Serializes table to Lua code format:
         *   return {
         *       health = 100,
         *       position = {x = 10, y = 20}
         *   }
         */
        static bool SaveTable(const std::string& filepath, sol::table& table) {
#if SAGE_ENABLE_LUA
            try {
                std::ofstream file(filepath);
                if (!file.is_open()) return false;

                file << "return ";
                WriteTable(file, table, 0);
                file << "\n";

                return true;
            }
            catch (...) {
                return false;
            }
#else
            (void)filepath;
            (void)table;
            return false;
#endif
        }

        /**
         * @brief Load a Lua table from a file
         */
        static sol::table LoadTable(sol::state& lua, const std::string& filepath) {
#if SAGE_ENABLE_LUA
            try {
                auto result = lua.script_file(filepath);
                if (result.valid() && result.get_type() == sol::type::table) {
                    return result;
                }
            }
            catch (...) {
                return sol::nil;
            }
            return sol::nil;
#else
            (void)lua;
            (void)filepath;
            return {};
#endif
        }

    private:
        static void WriteTable(std::ofstream& file, sol::table& table, int indent);
        static void WriteIndent(std::ofstream& file, int indent);
    };

} // namespace Scripting
} // namespace SAGE
