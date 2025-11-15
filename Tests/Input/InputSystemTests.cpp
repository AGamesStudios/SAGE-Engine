#include "../TestFramework.h"
#include "../../Engine/Input/InputAction.h"
#include "../../Engine/Input/InputMap.h"
#include "../../Engine/Input/ActionContext.h"
#include "../../Engine/Input/InputBuffer.h"
#include "../../Engine/Input/InputConfig.h"
#include <thread>
#include <chrono>

using namespace SAGE;

// ============================================================================
// InputAction Tests
// ============================================================================

TEST_CASE(InputAction_Creation) {
    InputAction action("Jump");
    
    ASSERT(action.GetName() == "Jump");
    ASSERT(action.GetState() == ActionState::None);
    ASSERT(action.GetValue() == 0.0f);
    ASSERT(!action.IsPressed());
    ASSERT(!action.IsHeld());
    ASSERT(!action.IsReleased());
}

TEST_CASE(InputAction_StateChanges) {
    InputAction action("Jump");
    
    // Pressed
    action.SetState(ActionState::Pressed);
    ASSERT(action.IsPressed());
    ASSERT(action.IsHeld());
    
    // Held
    action.SetState(ActionState::Held);
    ASSERT(!action.IsPressed());
    ASSERT(action.IsHeld());
    
    // Released
    action.SetState(ActionState::Released);
    ASSERT(!action.IsPressed());
    ASSERT(!action.IsHeld());
    ASSERT(action.IsReleased());
}

TEST_CASE(InputAction_Bindings) {
    InputAction action("Jump");
    
    // Add keyboard binding
    InputSource spaceKey(Key::Space);
    action.AddBinding(spaceKey);
    ASSERT(action.GetBindings().size() == 1);
    
    // Add gamepad binding
    InputSource aButton(GamepadButton::A);
    action.AddBinding(aButton);
    ASSERT(action.GetBindings().size() == 2);
    
    // Remove binding
    action.RemoveBinding(spaceKey);
    ASSERT(action.GetBindings().size() == 1);
    
    // Clear all
    action.ClearBindings();
    ASSERT(action.GetBindings().empty());
}

// ============================================================================
// InputMap Tests
// ============================================================================

TEST_CASE(InputMap_ActionCreation) {
    InputMap inputMap;
    
    auto* jump = inputMap.GetOrCreateAction("Jump");
    ASSERT(jump != nullptr);
    ASSERT(jump->GetName() == "Jump");
    ASSERT(inputMap.HasAction("Jump"));
    
    // Get existing action
    auto* jumpAgain = inputMap.GetOrCreateAction("Jump");
    ASSERT(jumpAgain == jump);
}

TEST_CASE(InputMap_ActionQueries) {
    InputMap inputMap;
    
    auto* jump = inputMap.GetOrCreateAction("Jump");
    jump->SetState(ActionState::Pressed);
    
    ASSERT(inputMap.IsActionPressed("Jump"));
    ASSERT(inputMap.IsActionHeld("Jump"));
    ASSERT(!inputMap.IsActionReleased("Jump"));
    
    // Non-existent action
    ASSERT(!inputMap.IsActionPressed("NonExistent"));
}

TEST_CASE(InputMap_RemoveAction) {
    InputMap inputMap;
    
    inputMap.GetOrCreateAction("Jump");
    ASSERT(inputMap.HasAction("Jump"));
    
    inputMap.RemoveAction("Jump");
    ASSERT(!inputMap.HasAction("Jump"));
}

// ============================================================================
// ActionContext Tests
// ============================================================================

TEST_CASE(ActionContext_Creation) {
    ActionContext context("Gameplay");
    
    ASSERT(context.GetName() == "Gameplay");
    ASSERT(!context.IsActive());
    ASSERT(context.GetPriority() == 0);
}

TEST_CASE(ActionContext_Activation) {
    ActionContext context("Gameplay");
    
    context.SetActive(true);
    ASSERT(context.IsActive());
    
    context.SetActive(false);
    ASSERT(!context.IsActive());
}

TEST_CASE(ActionContextManager_MultipleContexts) {
    ActionContextManager manager;
    
    auto* gameplay = manager.CreateContext("Gameplay", 0);
    auto* menu = manager.CreateContext("Menu", 10);
    
    ASSERT(gameplay != nullptr);
    ASSERT(menu != nullptr);
    
    // Activate gameplay
    manager.ActivateContext("Gameplay");
    ASSERT(manager.GetActiveContext() == gameplay);
    
    // Activate menu (higher priority)
    manager.ActivateContext("Menu");
    ASSERT(manager.GetActiveContext() == menu);
    
    // Deactivate menu
    manager.DeactivateContext("Menu");
    ASSERT(manager.GetActiveContext() == gameplay);
}

TEST_CASE(ActionContextManager_ActionQueries) {
    ActionContextManager manager;
    
    auto* gameplay = manager.CreateContext("Gameplay", 0);
    auto* jump = gameplay->GetInputMap().GetOrCreateAction("Jump");
    jump->SetState(ActionState::Pressed);
    
    manager.ActivateContext("Gameplay");
    
    ASSERT(manager.IsActionPressed("Jump"));
    ASSERT(!manager.IsActionPressed("NonExistent"));
}

// ============================================================================
// InputBuffer Tests
// ============================================================================

TEST_CASE(InputBuffer_BasicBuffering) {
    InputBuffer buffer(200); // 200ms buffer
    
    buffer.AddInput("Jump", ActionState::Pressed);
    ASSERT(buffer.GetSize() == 1);
    
    // Check if input was pressed
    ASSERT(buffer.WasPressed("Jump", false)); // Don't consume
    ASSERT(buffer.GetSize() == 1);
    
    // Consume input
    ASSERT(buffer.WasPressed("Jump", true));
    ASSERT(buffer.GetSize() == 0);
}

TEST_CASE(InputBuffer_Timeout) {
    InputBuffer buffer(100); // 100ms buffer
    
    buffer.AddInput("Jump", ActionState::Pressed);
    ASSERT(buffer.GetSize() == 1);
    
    // Wait for buffer to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    ASSERT(!buffer.WasPressed("Jump"));
}

TEST_CASE(InputBuffer_Sequence) {
    InputBuffer buffer(500); // 500ms buffer
    
    // Simulate quarter-circle-forward + punch combo
    buffer.AddInput("Down", ActionState::Pressed);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    buffer.AddInput("DownForward", ActionState::Pressed);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    buffer.AddInput("Forward", ActionState::Pressed);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    buffer.AddInput("Punch", ActionState::Pressed);
    
    // Check sequence
    std::vector<std::string> sequence = {"Down", "DownForward", "Forward", "Punch"};
    ASSERT(buffer.CheckSequence(sequence, 500, false)); // Don't consume
    
    // Check wrong sequence
    std::vector<std::string> wrongSequence = {"Down", "Forward"};
    ASSERT(!buffer.CheckSequence(wrongSequence, 500, false));
}

TEST_CASE(InputBuffer_MaxSize) {
    InputBuffer buffer(1000, 3); // Max 3 inputs
    
    buffer.AddInput("A", ActionState::Pressed);
    buffer.AddInput("B", ActionState::Pressed);
    buffer.AddInput("C", ActionState::Pressed);
    ASSERT(buffer.GetSize() == 3);
    
    // Adding 4th should remove oldest
    buffer.AddInput("D", ActionState::Pressed);
    ASSERT(buffer.GetSize() == 3);
    ASSERT(!buffer.WasPressed("A", false)); // Oldest removed
    ASSERT(buffer.WasPressed("D", false));  // Newest present
}

// ============================================================================
// InputConfig Tests
// ============================================================================

TEST_CASE(InputConfig_SerializeInputMap) {
    InputMap inputMap;
    
    auto* jump = inputMap.GetOrCreateAction("Jump");
    jump->AddBinding(InputSource(Key::Space));
    jump->AddBinding(InputSource(GamepadButton::A));
    
    auto* move = inputMap.GetOrCreateAction("MoveRight");
    move->AddBinding(InputSource(Key::D));
    
    // Serialize
    json j = InputConfig::SerializeInputMap(inputMap);
    
    ASSERT(j.contains("actions"));
    ASSERT(j["actions"].size() == 2);
}

TEST_CASE(InputConfig_DeserializeInputMap) {
    InputMap original;
    auto* jump = original.GetOrCreateAction("Jump");
    jump->AddBinding(InputSource(Key::Space));
    
    // Serialize
    json j = InputConfig::SerializeInputMap(original);
    
    // Deserialize to new map
    InputMap loaded;
    InputConfig::DeserializeInputMap(loaded, j);
    
    ASSERT(loaded.HasAction("Jump"));
    auto* loadedJump = loaded.GetAction("Jump");
    ASSERT(loadedJump != nullptr);
    ASSERT(loadedJump->GetBindings().size() == 1);
}

TEST_CASE(InputConfig_SaveLoadFile) {
    InputMap inputMap;
    auto* jump = inputMap.GetOrCreateAction("Jump");
    jump->AddBinding(InputSource(Key::Space));
    
    // Save to file
    const std::string filepath = "test_input_config.json";
    ASSERT(InputConfig::SaveToFile(inputMap, filepath));
    
    // Load from file
    InputMap loaded;
    ASSERT(InputConfig::LoadFromFile(loaded, filepath));
    ASSERT(loaded.HasAction("Jump"));
    
    // Clean up
    std::remove(filepath.c_str());
}
