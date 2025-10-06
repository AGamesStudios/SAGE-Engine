#include "InputBindings.h"

#include "Input.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace SAGE {

    namespace {
        struct ActionBinding {
            std::vector<int> keys;
            std::vector<int> mouseButtons;
            std::vector<GamepadButton> gamepadButtons;
            int gamepadIndex = 0;
            bool current = false;
            bool previous = false;
        };

        struct AxisBinding {
            std::vector<int> positiveKeys;
            std::vector<int> negativeKeys;
            std::vector<GamepadAxis> axes;
            int gamepadIndex = 0;
            float deadzone = 0.2f;
        };

        std::unordered_map<std::string, ActionBinding> s_Actions;
        std::unordered_map<std::string, AxisBinding> s_Axes;

        bool EvaluateActionBinding(const ActionBinding& binding) {
            for (int key : binding.keys) {
                if (Input::IsKeyPressed(key)) {
                    return true;
                }
            }

            for (int button : binding.mouseButtons) {
                if (Input::IsMouseButtonPressed(button)) {
                    return true;
                }
            }

            for (GamepadButton button : binding.gamepadButtons) {
                if (Input::IsGamepadButtonDown(button, binding.gamepadIndex)) {
                    return true;
                }
            }

            return false;
        }

        float ApplyDeadzone(float value, float deadzone) {
            float absValue = std::fabs(value);
            if (absValue <= deadzone) {
                return 0.0f;
            }

            float adjusted = (absValue - deadzone) / (1.0f - deadzone);
            adjusted = std::clamp(adjusted, 0.0f, 1.0f);
            return adjusted * (value < 0.0f ? -1.0f : 1.0f);
        }

    } // namespace

    void InputBindings::RegisterAction(const std::string& name, const ActionOptions& options) {
        auto [it, inserted] = s_Actions.try_emplace(name);
        ActionBinding& binding = it->second;

        if (inserted) {
            binding = ActionBinding{};
        }

        binding.keys = options.keys;
        binding.mouseButtons = options.mouseButtons;
        binding.gamepadButtons = options.gamepadButtons;
        binding.gamepadIndex = options.gamepadIndex;
    }

    void InputBindings::RegisterAction(const std::string& name,
                                       const std::vector<int>& keys,
                                       const std::vector<GamepadButton>& gamepadButtons,
                                       const std::vector<int>& mouseButtons,
                                       int gamepadIndex) {
        ActionOptions options{};
        options.keys = keys;
        options.gamepadButtons = gamepadButtons;
        options.mouseButtons = mouseButtons;
        options.gamepadIndex = gamepadIndex;
        RegisterAction(name, options);
    }

    void InputBindings::RegisterAxis(const std::string& name, const AxisOptions& options) {
        AxisBinding binding;
        binding.positiveKeys = options.positiveKeys;
        binding.negativeKeys = options.negativeKeys;
        binding.axes = options.axes;
        binding.gamepadIndex = options.gamepadIndex;
        binding.deadzone = std::clamp(options.deadzone, 0.0f, 0.9f);
        s_Axes[name] = binding;
    }

    void InputBindings::RegisterAxis(const std::string& name,
                                     const std::vector<int>& positiveKeys,
                                     const std::vector<int>& negativeKeys,
                                     const std::vector<GamepadAxis>& axes,
                                     int gamepadIndex,
                                     float deadzone) {
        AxisOptions options{};
        options.positiveKeys = positiveKeys;
        options.negativeKeys = negativeKeys;
        options.axes = axes;
        options.gamepadIndex = gamepadIndex;
        options.deadzone = deadzone;
        RegisterAxis(name, options);
    }

    bool InputBindings::EvaluateAction(const std::string& name, bool& outCurrent) {
        auto it = s_Actions.find(name);
        if (it == s_Actions.end()) {
            outCurrent = false;
            return false;
        }

        outCurrent = EvaluateActionBinding(it->second);
        return true;
    }

    bool InputBindings::IsActionDown(const std::string& name) {
        auto it = s_Actions.find(name);
        if (it == s_Actions.end()) {
            return false;
        }
        return it->second.current;
    }

    bool InputBindings::IsActionPressed(const std::string& name) {
        auto it = s_Actions.find(name);
        if (it == s_Actions.end()) {
            return false;
        }
        return it->second.current && !it->second.previous;
    }

    bool InputBindings::IsActionReleased(const std::string& name) {
        auto it = s_Actions.find(name);
        if (it == s_Actions.end()) {
            return false;
        }
        return !it->second.current && it->second.previous;
    }

    float InputBindings::GetAxis(const std::string& name) {
        auto it = s_Axes.find(name);
        if (it == s_Axes.end()) {
            return 0.0f;
        }

        const AxisBinding& binding = it->second;
        float value = 0.0f;

        for (int key : binding.positiveKeys) {
            if (Input::IsKeyPressed(key)) {
                value += 1.0f;
                break;
            }
        }

        for (int key : binding.negativeKeys) {
            if (Input::IsKeyPressed(key)) {
                value -= 1.0f;
                break;
            }
        }

        for (GamepadAxis axis : binding.axes) {
            float raw = Input::GetGamepadAxis(axis, binding.gamepadIndex);
            raw = ApplyDeadzone(raw, binding.deadzone);
            value += raw;
        }

        value = std::clamp(value, -1.0f, 1.0f);
        return value;
    }

    void InputBindings::RemoveAction(const std::string& name) {
        s_Actions.erase(name);
    }

    void InputBindings::RemoveAxis(const std::string& name) {
        s_Axes.erase(name);
    }

    void InputBindings::Clear() {
        s_Actions.clear();
        s_Axes.clear();
    }

    void InputBindings::Update() {
        for (auto& [_, binding] : s_Actions) {
            binding.previous = binding.current;
            binding.current = EvaluateActionBinding(binding);
        }
    }

    bool InputBindings::LoadFromFile(const std::string& filepath) {
        // Простая реализация - текстовый формат
        // Формат: <type> <name> <keys/buttons...>
        // Например:
        // action Jump 32 (SPACE)
        // axis Horizontal 68,-65 (D positive, A negative)
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        Clear();
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue; // Пропускаем комментарии
            
            std::istringstream iss(line);
            std::string type, name;
            iss >> type >> name;
            
            if (type == "action") {
                ActionOptions options;
                int key;
                while (iss >> key) {
                    options.keys.push_back(key);
                }
                RegisterAction(name, options);
            }
            else if (type == "axis") {
                AxisOptions options;
                std::string keysStr;
                iss >> keysStr;
                
                size_t commaPos = keysStr.find(',');
                if (commaPos != std::string::npos) {
                    int positive = std::stoi(keysStr.substr(0, commaPos));
                    int negative = std::stoi(keysStr.substr(commaPos + 1));
                    options.positiveKeys.push_back(positive);
                    options.negativeKeys.push_back(negative);
                }
                RegisterAxis(name, options);
            }
        }
        
        return true;
    }

    bool InputBindings::SaveToFile(const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        file << "# SAGE Engine Input Bindings\n";
        file << "# Format: <type> <name> <keys...>\n\n";
        
        for (const auto& [name, binding] : s_Actions) {
            file << "action " << name;
            for (int key : binding.keys) {
                file << " " << key;
            }
            file << "\n";
        }
        
        file << "\n";
        
        for (const auto& [name, binding] : s_Axes) {
            file << "axis " << name;
            if (!binding.positiveKeys.empty() && !binding.negativeKeys.empty()) {
                file << " " << binding.positiveKeys[0] << "," << binding.negativeKeys[0];
            }
            file << "\n";
        }
        
        return true;
    }

} // namespace SAGE
