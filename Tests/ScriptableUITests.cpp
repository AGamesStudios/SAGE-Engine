#include "TestFramework.h"

#include "../Engine/UI/Integration/ScriptableUI.h"
#include <string>
#include <unordered_map>
#include <vector>

using namespace SAGE::UI::Overlay;

TEST_CASE(ScriptableUI_ButtonBindingUpdatesResolvedValue) {
    ScriptableUI scriptable;
    UIElementHandle handle{ 42 };

    ScriptableButton* button = scriptable.CreateButton(handle);
    REQUIRE(button != nullptr);

    button->SetTextBinding("ui.label");

    std::unordered_map<std::string, std::string> values{
        { "ui.label", "Hello" }
    };

    struct Applied {
        UIElementHandle handle;
        std::string property;
        std::string value;
    };

    std::vector<Applied> applied;

    scriptable.UpdateBindings(
        [&](const std::string& variable) {
            auto it = values.find(variable);
            if (it != values.end()) {
                return it->second;
            }
            return std::string{};
        },
        [&](UIElementHandle h, const std::string& property, const std::string& value) {
            applied.push_back({ h, property, value });
        }
    );

    REQUIRE(applied.size() == 1);
    CHECK(applied[0].handle.value == handle.value);
    CHECK(applied[0].property == "text");
    CHECK(applied[0].value == "Hello");
}

TEST_CASE(ScriptableUI_RemoveComponentClearsBindings) {
    ScriptableUI scriptable;
    UIElementHandle handle{ 7 };

    ScriptableButton* button = scriptable.CreateButton(handle);
    REQUIRE(button != nullptr);
    button->SetTextBinding("ui.value");

    scriptable.RemoveComponent(handle);

    bool setterCalled = false;
    scriptable.UpdateBindings(
        [](const std::string&) { return std::string("ignored"); },
        [&](UIElementHandle, const std::string&, const std::string&) {
            setterCalled = true;
        }
    );

    CHECK_FALSE(setterCalled);
}

TEST_CASE(ScriptableUI_ComputedBindingEvaluatesArithmetic) {
    ScriptableUI scriptable;
    UIElementHandle handle{ 101 };

    ScriptableButton* button = scriptable.CreateButton(handle);
    REQUIRE(button != nullptr);

    button->SetTextBinding("Damage: {base * multiplier}");

    std::unordered_map<std::string, std::string> values{
        { "base", "12" },
        { "multiplier", "3" }
    };

    struct Applied {
        UIElementHandle handle;
        std::string property;
        std::string value;
    };

    std::vector<Applied> applied;

    scriptable.UpdateBindings(
        [&](const std::string& variable) {
            auto it = values.find(variable);
            if (it != values.end()) {
                return it->second;
            }
            return std::string{};
        },
        [&](UIElementHandle h, const std::string& property, const std::string& value) {
            applied.push_back({ h, property, value });
        }
    );

    REQUIRE(applied.size() == 1);
    CHECK(applied[0].handle.value == handle.value);
    CHECK(applied[0].property == "text");
    CHECK(applied[0].value == "Damage: 36");
}

TEST_CASE(ScriptableUI_ComputedBindingEvaluatesConditionals) {
    ScriptableUI scriptable;
    UIElementHandle handle{ 202 };

    ScriptableButton* button = scriptable.CreateButton(handle);
    REQUIRE(button != nullptr);

    button->SetTextBinding("Status: {hp <= 0 ? 'Dead' : 'Alive'}");

    std::unordered_map<std::string, std::string> values{
        { "hp", "0" }
    };

    std::vector<std::string> appliedValues;

    scriptable.UpdateBindings(
        [&](const std::string& variable) {
            auto it = values.find(variable);
            if (it != values.end()) {
                return it->second;
            }
            return std::string{};
        },
        [&](UIElementHandle, const std::string&, const std::string& value) {
            appliedValues.push_back(value);
        }
    );

    REQUIRE(appliedValues.size() == 1);
    CHECK(appliedValues[0] == "Status: Dead");

    values["hp"] = "42";
    appliedValues.clear();

    scriptable.UpdateBindings(
        [&](const std::string& variable) {
            auto it = values.find(variable);
            if (it != values.end()) {
                return it->second;
            }
            return std::string{};
        },
        [&](UIElementHandle, const std::string&, const std::string& value) {
            appliedValues.push_back(value);
        }
    );

    REQUIRE(appliedValues.size() == 1);
    CHECK(appliedValues[0] == "Status: Alive");
}
