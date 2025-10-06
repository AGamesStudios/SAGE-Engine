#include "TestFramework.h"

#include <SAGE.h>
#include <cmath>

using namespace SAGE;

TEST_CASE(InputBindings_ActionStates) {
    InputBindings::Clear();
    Testing::ResetInputState();

    InputBindings::RegisterAction("Jump", { SAGE_KEY_SPACE });

    InputBindings::Update();
    CHECK(!InputBindings::IsActionDown("Jump"));
    CHECK(!InputBindings::IsActionPressed("Jump"));
    CHECK(!InputBindings::IsActionReleased("Jump"));

    Testing::SetKeyState(SAGE_KEY_SPACE, true, false);
    InputBindings::Update();
    CHECK(InputBindings::IsActionDown("Jump"));
    CHECK(InputBindings::IsActionPressed("Jump"));
    CHECK(!InputBindings::IsActionReleased("Jump"));

    Testing::SetKeyState(SAGE_KEY_SPACE, true, true);
    InputBindings::Update();
    CHECK(InputBindings::IsActionDown("Jump"));
    CHECK(!InputBindings::IsActionPressed("Jump"));
    CHECK(!InputBindings::IsActionReleased("Jump"));

    Testing::SetKeyState(SAGE_KEY_SPACE, false, true);
    InputBindings::Update();
    CHECK(!InputBindings::IsActionDown("Jump"));
    CHECK(!InputBindings::IsActionPressed("Jump"));
    CHECK(InputBindings::IsActionReleased("Jump"));

    InputBindings::Clear();
    Testing::ResetInputState();
}

TEST_CASE(InputBindings_ActionRebindMaintainsState) {
    InputBindings::Clear();
    Testing::ResetInputState();

    InputBindings::RegisterAction("Jump", { SAGE_KEY_SPACE });

    InputBindings::Update();

    Testing::SetKeyState(SAGE_KEY_SPACE, true, false);
    InputBindings::Update();

    Testing::SetKeyState(SAGE_KEY_SPACE, true, true);
    InputBindings::Update();

    CHECK(InputBindings::IsActionDown("Jump"));
    CHECK(!InputBindings::IsActionPressed("Jump"));

    InputBindings::RegisterAction("Jump", { SAGE_KEY_SPACE });

    Testing::SetKeyState(SAGE_KEY_SPACE, true, true);
    InputBindings::Update();

    CHECK(InputBindings::IsActionDown("Jump"));
    CHECK(!InputBindings::IsActionPressed("Jump"));

    InputBindings::Clear();
    Testing::ResetInputState();
}

TEST_CASE(InputBindings_GamepadAxisDeadzone) {
    InputBindings::Clear();
    Testing::ResetInputState();

    InputBindings::AxisOptions options{};
    options.axes = { GamepadAxis::LeftX };
    options.deadzone = 0.2f;
    InputBindings::RegisterAxis("MoveX", options);

    Testing::SetGamepadAxisValue(GamepadAxis::LeftX, 0.1f);
    float axis = InputBindings::GetAxis("MoveX");
    CHECK(std::abs(axis) < 0.0001f);

    Testing::SetGamepadAxisValue(GamepadAxis::LeftX, 0.5f);
    axis = InputBindings::GetAxis("MoveX");
    CHECK(std::abs(axis - 0.375f) < 0.0001f);

    Testing::SetGamepadAxisValue(GamepadAxis::LeftX, -1.0f);
    axis = InputBindings::GetAxis("MoveX");
    CHECK(std::abs(axis + 1.0f) < 0.0001f);

    InputBindings::Clear();
    Testing::ResetInputState();
}
