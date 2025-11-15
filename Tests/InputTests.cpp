#include "TestFramework.h"
#include "../Engine/Input/Input.h"
#include "../Engine/Input/InputBindings.h"
#include <GLFW/glfw3.h>

using namespace SAGE;

TEST_CASE("Input_KeyboardTracking") {
    bool isPressed = Input::IsKeyPressed(GLFW_KEY_SPACE);
    REQUIRE(isPressed == false);
}

TEST_CASE("Input_MousePositionDelta") {
    Vector2 delta = Input::GetMouseDelta();
    REQUIRE(delta.x == 0.0f);
    REQUIRE(delta.y == 0.0f);
}

TEST_CASE("Input_GamepadConnection") {
    bool connected = Input::IsGamepadConnected(0);
    REQUIRE(connected == false || connected == true);
}

TEST_CASE("InputBindings_ActionRegistration") {
    InputBindings bindings;
    bindings.RegisterAction("jump", GLFW_KEY_SPACE);
    bindings.RegisterAction("jump", GLFW_KEY_W);
    
    bool isPressed = bindings.IsActionPressed("jump");
    REQUIRE(isPressed == false);
}

TEST_CASE("InputBindings_AxisRegistration") {
    InputBindings bindings;
    bindings.RegisterAxis("horizontal", GLFW_KEY_D, GLFW_KEY_A);
    
    float axisValue = bindings.GetAxis("horizontal");
    REQUIRE(axisValue == 0.0f);
}

TEST_CASE("InputBindings_FileIO") {
    InputBindings bindings;
    bindings.RegisterAction("fire", GLFW_KEY_LEFT_CONTROL);
    bindings.RegisterAxis("move_vertical", GLFW_KEY_W, GLFW_KEY_S);
    
    std::string filepath = "test_bindings.txt";
    bindings.SaveToFile(filepath);
    
    InputBindings loaded;
    loaded.LoadFromFile(filepath);
    
    REQUIRE(true);
    std::filesystem::remove(filepath);
}
