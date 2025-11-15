#include "TestFramework.h"
#include "Input/InputManager.h"
#include "Input/KeyCodes.h"

using namespace SAGE;
using namespace TestFramework;

namespace {

class ResetInputManagerScope {
public:
    ResetInputManagerScope() {
        auto& mgr = InputManager::Get();
        mgr.Shutdown();
    }

    ~ResetInputManagerScope() {
        auto& mgr = InputManager::Get();
        mgr.Shutdown();
    }
};

} // namespace

TEST(InputManager_Singleton) {
    ResetInputManagerScope reset;

    InputManager& input1 = InputManager::Get();
    InputManager& input2 = InputManager::Get();

    CHECK(&input1 == &input2);
}

TEST(InputManager_InitializationGuard) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    CHECK(!input.IsInitialized());

    // Null window should be rejected gracefully
    input.Initialize(static_cast<IWindow*>(nullptr));
    CHECK(!input.IsInitialized());

    input.Shutdown();
    CHECK(!input.IsInitialized());
}

TEST(InputManager_Update_NoInitialization) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    for (int i = 0; i < 5; ++i) {
        input.Update();
    }
    CHECK(!input.IsInitialized());
}

TEST(InputManager_DefaultKeyboardState) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    CHECK(!input.IsKeyPressed(Key::Space));
    CHECK(!input.IsKeyHeld(Key::W));
    CHECK(!input.IsKeyReleased(Key::Escape));
}

TEST(InputManager_DefaultMouseState) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    CHECK(!input.IsMouseButtonPressed(MouseButton::Left));
    CHECK(!input.IsMouseButtonHeld(MouseButton::Right));
    CHECK(!input.IsMouseButtonReleased(MouseButton::Middle));

    Vector2 pos = input.GetMousePosition();
    CHECK(pos == Vector2::Zero());
}

TEST(InputManager_DefaultGamepadState) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    CHECK(!input.IsGamepadConnected(0));
    CHECK(!input.IsGamepadButtonPressed(0, GamepadButton::A));
    CHECK(!input.IsGamepadButtonHeld(0, GamepadButton::B));
    CHECK(!input.IsGamepadButtonReleased(0, GamepadButton::X));
    CHECK_NEAR(input.GetGamepadAxis(0, GamepadAxis::LeftX), 0.0f, 1e-4f);
}

TEST(InputManager_ActionQueries_Default) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    CHECK(!input.IsActionPressed("Jump"));
    CHECK(!input.IsActionHeld("Fire"));
    CHECK(!input.IsActionReleased("Dash"));
    CHECK_NEAR(input.GetActionValue("Move"), 0.0f, 1e-4f);
}

TEST(InputManager_Shutdown_Idempotent) {
    ResetInputManagerScope reset;

    InputManager& input = InputManager::Get();
    input.Shutdown();
    CHECK(!input.IsInitialized());

    input.Shutdown();
    CHECK(!input.IsInitialized());
}
