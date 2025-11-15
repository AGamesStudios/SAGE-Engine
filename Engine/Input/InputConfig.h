#pragma once

#include "InputMap.h"
#include "ActionContext.h"
#include <nlohmann/json.hpp>
#include <string>

/**
 * @file InputConfig.h
 * @brief JSON serialization/deserialization for input bindings
 */

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Input configuration serialization
 * 
 * Save/load key bindings to/from JSON files.
 * Supports rebindable controls, user preferences, etc.
 */
class InputConfig {
public:
    /**
     * @brief Serialize InputMap to JSON
     */
    static json SerializeInputMap(const InputMap& inputMap);
    
    /**
     * @brief Deserialize InputMap from JSON
     */
    static void DeserializeInputMap(InputMap& inputMap, const json& j);
    
    /**
     * @brief Save InputMap to file
     */
    static bool SaveToFile(const InputMap& inputMap, const std::string& filepath);
    
    /**
     * @brief Load InputMap from file
     */
    static bool LoadFromFile(InputMap& inputMap, const std::string& filepath);
    
    /**
     * @brief Serialize ActionContext to JSON
     */
    static json SerializeContext(const ActionContext& context);
    
    /**
     * @brief Deserialize ActionContext from JSON
     */
    static void DeserializeContext(ActionContext& context, const json& j);
    
    /**
     * @brief Serialize ActionContextManager to JSON
     */
    static json SerializeContextManager(const ActionContextManager& manager);
    
    /**
     * @brief Deserialize ActionContextManager from JSON
     */
    static void DeserializeContextManager(ActionContextManager& manager, const json& j);
    
private:
    // Helper: Serialize InputSource
    static json SerializeInputSource(const InputSource& source);
    
    // Helper: Deserialize InputSource
    static InputSource DeserializeInputSource(const json& j);
    
    // Helper: Serialize InputAction
    static json SerializeInputAction(const InputAction& action);
    
    // Helper: Deserialize InputAction
    static std::unique_ptr<InputAction> DeserializeInputAction(const json& j);
};

} // namespace SAGE
