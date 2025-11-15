#pragma once

#include "RuntimeValue.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SAGE {
class GameObject;
}

namespace SAGE::Scripting::LogCon::Runtime {

// Function signature for native functions
using NativeFunction = std::function<RuntimeValue(
    const std::vector<RuntimeValue>& arguments,
    GameObject* gameObject
)>;

// Function metadata
struct FunctionInfo {
    std::string canonicalName;          // Canonical name (e.g., "random")
    std::vector<std::string> aliases;   // All language aliases
    NativeFunction implementation;
    std::string category;               // e.g., "math", "string", "rpg"
    std::string description;
    size_t minArgs = 0;
    size_t maxArgs = SIZE_MAX;
    
    FunctionInfo() = default;
    
    FunctionInfo(std::string name, NativeFunction func)
        : canonicalName(std::move(name))
        , implementation(std::move(func))
    {}
};

/**
 * @brief Central registry for all built-in and custom functions
 * 
 * This allows:
 * 1. Easy registration of new functions without modifying Interpreter.cpp
 * 2. Function overloading by language
 * 3. Dynamic library loading
 * 4. Plugin system support
 */
class FunctionRegistry {
public:
    static FunctionRegistry& Get();
    
    /**
     * @brief Register a native function with all its language aliases
     * @param info Function information including name, aliases, implementation
     * @return true if registered successfully, false if name conflict
     */
    bool RegisterFunction(const FunctionInfo& info);
    
    /**
     * @brief Register a simple function with one name
     */
    bool RegisterFunction(
        const std::string& name,
        NativeFunction func,
        const std::string& category = "general"
    );
    
    /**
     * @brief Register a function with multiple language aliases
     */
    bool RegisterFunction(
        const std::vector<std::string>& names,
        NativeFunction func,
        const std::string& category = "general"
    );
    
    /**
     * @brief Find a function by any of its names/aliases
     * @param name Function name in any supported language
     * @return Pointer to FunctionInfo if found, nullptr otherwise
     */
    const FunctionInfo* FindFunction(const std::string& name) const;
    
    /**
     * @brief Call a function by name
     */
    std::optional<RuntimeValue> CallFunction(
        const std::string& name,
        const std::vector<RuntimeValue>& arguments,
        GameObject* gameObject = nullptr
    ) const;
    
    /**
     * @brief Get all functions in a category
     */
    std::vector<const FunctionInfo*> GetFunctionsByCategory(const std::string& category) const;
    
    /**
     * @brief Get all registered function names (all aliases)
     */
    std::vector<std::string> GetAllFunctionNames() const;
    
    /**
     * @brief Check if a function exists
     */
    bool HasFunction(const std::string& name) const;
    
    /**
     * @brief Unregister a function (useful for hot-reload)
     */
    bool UnregisterFunction(const std::string& name);
    
    /**
     * @brief Clear all functions in a category
     */
    void ClearCategory(const std::string& category);
    
    /**
     * @brief Clear all registered functions
     */
    void Clear();

private:
    FunctionRegistry() = default;
    
    // Map from function name/alias to canonical function info
    std::unordered_map<std::string, std::shared_ptr<FunctionInfo>> m_Functions;
    
    // Map from canonical name to shared info (for deduplication)
    std::unordered_map<std::string, std::shared_ptr<FunctionInfo>> m_CanonicalFunctions;
    
    // Functions grouped by category
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionInfo>>> m_Categories;
};

/**
 * @brief Helper class for easy function registration
 * 
 * Example:
 *   FunctionRegistrar("math")
 *       .Add({"sqrt", "корень", "raiz"}, MySqrtFunc)
 *       .Add({"abs", "модуль", "valor_absoluto"}, MyAbsFunc);
 */
class FunctionRegistrar {
public:
    explicit FunctionRegistrar(std::string category)
        : m_Category(std::move(category))
    {}
    
    FunctionRegistrar& Add(
        const std::vector<std::string>& names,
        NativeFunction func
    );
    
    FunctionRegistrar& Add(
        const std::string& name,
        NativeFunction func
    );
    
private:
    std::string m_Category;
};

// Register all built-in functions (math, string, arrays, game functions)
void RegisterBuiltinFunctions();

} // namespace SAGE::Scripting::LogCon::Runtime
