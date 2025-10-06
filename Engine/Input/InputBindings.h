#pragma once

#include <string>
#include <vector>

#include "Input.h"

namespace SAGE {

    class InputBindings {
    public:
        struct ActionOptions {
            std::vector<int> keys;
            std::vector<int> mouseButtons;
            std::vector<GamepadButton> gamepadButtons;
            int gamepadIndex = 0;
        };

        struct AxisOptions {
            std::vector<int> positiveKeys;
            std::vector<int> negativeKeys;
            std::vector<GamepadAxis> axes;
            int gamepadIndex = 0;
            float deadzone = 0.2f;
        };

        static void RegisterAction(const std::string& name, const ActionOptions& options);
        static void RegisterAction(const std::string& name,
                                   const std::vector<int>& keys,
                                   const std::vector<GamepadButton>& gamepadButtons = {},
                                   const std::vector<int>& mouseButtons = {},
                                   int gamepadIndex = 0);

        static void RegisterAxis(const std::string& name, const AxisOptions& options);
        static void RegisterAxis(const std::string& name,
                                 const std::vector<int>& positiveKeys,
                                 const std::vector<int>& negativeKeys,
                                 const std::vector<GamepadAxis>& axes = {},
                                 int gamepadIndex = 0,
                                 float deadzone = 0.2f);

        static bool IsActionDown(const std::string& name);
        static bool IsActionPressed(const std::string& name);
        static bool IsActionReleased(const std::string& name);

        static float GetAxis(const std::string& name);

        static void RemoveAction(const std::string& name);
        static void RemoveAxis(const std::string& name);
        static void Clear();

        static void Update();
        
        // Загрузка/сохранение конфигурации (JSON формат)
        static bool LoadFromFile(const std::string& filepath);
        static bool SaveToFile(const std::string& filepath);

    private:
        static bool EvaluateAction(const std::string& name, bool& outCurrent);
    };

} // namespace SAGE
