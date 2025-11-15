#pragma once

namespace SAGE::Modding {

/**
 * @brief Register Lua bindings for mod system
 * 
 * Allows Lua scripts to:
 * - Query loaded mods
 * - Get mod information
 * - Request mod assets
 * - Listen to mod events
 */
void RegisterLuaBindings();

/**
 * @brief Register LogCon functions for mod system
 * 
 * Adds functions to LogCon:
 * - мод_загружен/mod_loaded(modId) -> bool
 * - мод_инфо/mod_info(modId) -> table
 * - мод_ассет/mod_asset(modId, assetPath) -> string
 * - моды_список/mods_list() -> array
 */
void RegisterLogConFunctions();

} // namespace SAGE::Modding
