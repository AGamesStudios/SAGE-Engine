#include "InputConfig.h"
#include <fstream>

namespace SAGE {

json InputConfig::SerializeInputSource(const InputSource& source) {
    json j;
    
    switch (source.type) {
        case InputSource::Type::Keyboard:
            j["type"] = "keyboard";
            j["key"] = static_cast<int>(std::get<Key>(source.source));
            break;
        case InputSource::Type::Mouse:
            j["type"] = "mouse";
            j["button"] = static_cast<int>(std::get<MouseButton>(source.source));
            break;
        case InputSource::Type::GamepadButton:
            j["type"] = "gamepad_button";
            j["button"] = static_cast<int>(std::get<GamepadButton>(source.source));
            break;
        case InputSource::Type::GamepadAxis:
            j["type"] = "gamepad_axis";
            j["axis"] = static_cast<int>(std::get<GamepadAxis>(source.source));
            j["threshold"] = source.axisThreshold;
            break;
    }
    
    return j;
}

InputSource InputConfig::DeserializeInputSource(const json& j) {
    std::string type = j["type"];
    
    if (type == "keyboard") {
        return InputSource(static_cast<Key>(j["key"].get<int>()));
    } else if (type == "mouse") {
        return InputSource(static_cast<MouseButton>(j["button"].get<int>()));
    } else if (type == "gamepad_button") {
        return InputSource(static_cast<GamepadButton>(j["button"].get<int>()));
    } else if (type == "gamepad_axis") {
        float threshold = j.contains("threshold") ? j["threshold"].get<float>() : 0.5f;
        return InputSource(static_cast<GamepadAxis>(j["axis"].get<int>()), threshold);
    }
    
    // Default to Space key
    return InputSource(Key::Space);
}

json InputConfig::SerializeInputAction(const InputAction& action) {
    json j;
    j["name"] = action.GetName();
    
    json bindings = json::array();
    for (const auto& binding : action.GetBindings()) {
        bindings.push_back(SerializeInputSource(binding));
    }
    j["bindings"] = bindings;
    
    return j;
}

std::unique_ptr<InputAction> InputConfig::DeserializeInputAction(const json& j) {
    auto action = std::make_unique<InputAction>(j["name"]);
    
    if (j.contains("bindings")) {
        for (const auto& bindingJson : j["bindings"]) {
            action->AddBinding(DeserializeInputSource(bindingJson));
        }
    }
    
    return action;
}

json InputConfig::SerializeInputMap(const InputMap& inputMap) {
    json j;
    j["actions"] = json::array();
    
    for (const auto& [name, action] : inputMap.GetActions()) {
        j["actions"].push_back(SerializeInputAction(*action));
    }
    
    return j;
}

void InputConfig::DeserializeInputMap(InputMap& inputMap, const json& j) {
    inputMap.Clear();
    
    if (j.contains("actions")) {
        for (const auto& actionJson : j["actions"]) {
            auto action = DeserializeInputAction(actionJson);
            std::string name = action->GetName();
            
            auto* newAction = inputMap.GetOrCreateAction(name);
            for (const auto& binding : action->GetBindings()) {
                newAction->AddBinding(binding);
            }
        }
    }
}

bool InputConfig::SaveToFile(const InputMap& inputMap, const std::string& filepath) {
    try {
        json j = SerializeInputMap(inputMap);
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4); // Pretty print with 4-space indent
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool InputConfig::LoadFromFile(InputMap& inputMap, const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json j = json::parse(file);
        DeserializeInputMap(inputMap, j);
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

json InputConfig::SerializeContext(const ActionContext& context) {
    json j;
    j["name"] = context.GetName();
    j["priority"] = context.GetPriority();
    j["active"] = context.IsActive();
    j["input_map"] = SerializeInputMap(context.GetInputMap());
    return j;
}

void InputConfig::DeserializeContext(ActionContext& context, const json& j) {
    if (j.contains("priority")) {
        context.SetPriority(j["priority"]);
    }
    if (j.contains("active")) {
        context.SetActive(j["active"]);
    }
    if (j.contains("input_map")) {
        InputMap& inputMap = const_cast<InputMap&>(context.GetInputMap());
        DeserializeInputMap(inputMap, j["input_map"]);
    }
}

json InputConfig::SerializeContextManager([[maybe_unused]] const ActionContextManager& manager) {
    json j;
    j["contexts"] = json::array();
    
    // Note: Cannot iterate contexts directly, so this is a placeholder
    // In real implementation, would need friend access or getter
    // Suppress unused parameter warning
    // (void)manager; // attribute already applied
    return j;
}

void InputConfig::DeserializeContextManager(ActionContextManager& manager, const json& j) {
    manager.DeactivateAll();
    
    if (j.contains("contexts")) {
        for (const auto& contextJson : j["contexts"]) {
            std::string name = contextJson["name"];
            int priority = contextJson.contains("priority") ? contextJson["priority"].get<int>() : 0;
            
            auto* context = manager.CreateContext(name, priority);
            DeserializeContext(*context, contextJson);
        }
    }
}

} // namespace SAGE
