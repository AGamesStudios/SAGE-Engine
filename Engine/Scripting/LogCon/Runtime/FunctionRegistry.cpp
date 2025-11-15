#include "FunctionRegistry.h"
#include <algorithm>

namespace SAGE::Scripting::LogCon::Runtime {

FunctionRegistry& FunctionRegistry::Get() {
    static FunctionRegistry instance;
    return instance;
}

bool FunctionRegistry::RegisterFunction(const FunctionInfo& info) {
    if (info.canonicalName.empty() || !info.implementation) {
        return false;
    }
    
    // Create shared pointer for this function
    auto funcPtr = std::make_shared<FunctionInfo>(info);
    
    // Register canonical name
    m_CanonicalFunctions[info.canonicalName] = funcPtr;
    
    // Register all aliases
    m_Functions[info.canonicalName] = funcPtr;
    for (const auto& alias : info.aliases) {
        if (alias.empty()) continue;
        
        // Check for conflicts
        if (m_Functions.count(alias) > 0) {
            // Allow re-registration of same function
            if (m_Functions[alias]->canonicalName != info.canonicalName) {
                return false; // Name conflict with different function
            }
        }
        m_Functions[alias] = funcPtr;
    }
    
    // Add to category
    m_Categories[info.category].push_back(funcPtr);
    
    return true;
}

bool FunctionRegistry::RegisterFunction(
    const std::string& name,
    NativeFunction func,
    const std::string& category
) {
    FunctionInfo info;
    info.canonicalName = name;
    info.implementation = std::move(func);
    info.category = category;
    return RegisterFunction(info);
}

bool FunctionRegistry::RegisterFunction(
    const std::vector<std::string>& names,
    NativeFunction func,
    const std::string& category
) {
    if (names.empty()) return false;
    
    FunctionInfo info;
    info.canonicalName = names[0];
    info.implementation = std::move(func);
    info.category = category;
    
    // Add other names as aliases
    for (size_t i = 1; i < names.size(); ++i) {
        info.aliases.push_back(names[i]);
    }
    
    return RegisterFunction(info);
}

const FunctionInfo* FunctionRegistry::FindFunction(const std::string& name) const {
    auto it = m_Functions.find(name);
    if (it != m_Functions.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::optional<RuntimeValue> FunctionRegistry::CallFunction(
    const std::string& name,
    const std::vector<RuntimeValue>& arguments,
    GameObject* gameObject
) const {
    const auto* info = FindFunction(name);
    if (!info || !info->implementation) {
        return std::nullopt;
    }
    
    // Check argument count
    if (arguments.size() < info->minArgs || arguments.size() > info->maxArgs) {
        return std::nullopt;
    }
    
    return info->implementation(arguments, gameObject);
}

std::vector<const FunctionInfo*> FunctionRegistry::GetFunctionsByCategory(const std::string& category) const {
    std::vector<const FunctionInfo*> result;
    
    auto it = m_Categories.find(category);
    if (it != m_Categories.end()) {
        result.reserve(it->second.size());
        for (const auto& funcPtr : it->second) {
            result.push_back(funcPtr.get());
        }
    }
    
    return result;
}

std::vector<std::string> FunctionRegistry::GetAllFunctionNames() const {
    std::vector<std::string> names;
    names.reserve(m_Functions.size());
    
    for (const auto& [name, _] : m_Functions) {
        names.push_back(name);
    }
    
    std::sort(names.begin(), names.end());
    return names;
}

bool FunctionRegistry::HasFunction(const std::string& name) const {
    return m_Functions.count(name) > 0;
}

bool FunctionRegistry::UnregisterFunction(const std::string& name) {
    auto it = m_Functions.find(name);
    if (it == m_Functions.end()) {
        return false;
    }
    
    auto funcPtr = it->second;
    const std::string& canonical = funcPtr->canonicalName;
    
    // Remove all aliases
    m_Functions.erase(canonical);
    for (const auto& alias : funcPtr->aliases) {
        m_Functions.erase(alias);
    }
    
    // Remove from canonical map
    m_CanonicalFunctions.erase(canonical);
    
    // Remove from category
    auto catIt = m_Categories.find(funcPtr->category);
    if (catIt != m_Categories.end()) {
        auto& vec = catIt->second;
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                [&](const auto& f) { return f->canonicalName == canonical; }),
            vec.end()
        );
    }
    
    return true;
}

void FunctionRegistry::ClearCategory(const std::string& category) {
    auto it = m_Categories.find(category);
    if (it == m_Categories.end()) return;
    
    // Collect all functions to remove
    std::vector<std::string> toRemove;
    for (const auto& funcPtr : it->second) {
        toRemove.push_back(funcPtr->canonicalName);
    }
    
    // Remove each function
    for (const auto& name : toRemove) {
        UnregisterFunction(name);
    }
    
    m_Categories.erase(category);
}

void FunctionRegistry::Clear() {
    m_Functions.clear();
    m_CanonicalFunctions.clear();
    m_Categories.clear();
}

// FunctionRegistrar implementation
FunctionRegistrar& FunctionRegistrar::Add(
    const std::vector<std::string>& names,
    NativeFunction func
) {
    FunctionRegistry::Get().RegisterFunction(names, std::move(func), m_Category);
    return *this;
}

FunctionRegistrar& FunctionRegistrar::Add(
    const std::string& name,
    NativeFunction func
) {
    FunctionRegistry::Get().RegisterFunction(name, std::move(func), m_Category);
    return *this;
}

} // namespace SAGE::Scripting::LogCon::Runtime
