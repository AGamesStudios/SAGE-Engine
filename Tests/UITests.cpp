#include "TestFramework.h"

#include <SAGE.h>

#include <cmath>

namespace {
    inline bool AlmostEqual(float lhs, float rhs, float epsilon = 0.0001f) {
        return std::fabs(lhs - rhs) < epsilon;
    }
}

using namespace SAGE;

TEST_CASE(UISystem_ButtonInvokesCallbacks) {
    UI::UISystem::Init();
    UI::UISystem::Clear();
    Testing::ResetInputState();

    bool hovered = false;
    bool pressed = false;
    bool released = false;
    bool clicked = false;

    UI::ButtonConfig config{};
    config.id = "test_button";
    config.position = Vector2(100.0f, 100.0f);
    config.size = Vector2(200.0f, 60.0f);
    config.onHover = [&]() { hovered = true; };
    config.onPressed = [&]() { pressed = true; };
    config.onRelease = [&]() { released = true; };
    config.onClick = [&]() { clicked = true; };

    UI::Button* button = UI::UISystem::CreateButton(config);
    REQUIRE(button != nullptr);

    Testing::SetMousePosition(Vector2(150.0f, 120.0f));
    button->Update(0.016f);
    CHECK(hovered);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, false);
    button->Update(0.016f);
    CHECK(pressed);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, true);
    button->Update(0.016f);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, false, true);
    button->Update(0.016f);
    CHECK(released);
    CHECK(clicked);

    UI::UISystem::Clear();
    Testing::ResetInputState();

    hovered = false;
    pressed = false;
    released = false;
    clicked = false;

    UI::ButtonConfig second = config;
    second.id = "test_button_outside";
    UI::Button* secondary = UI::UISystem::CreateButton(second);
    REQUIRE(secondary != nullptr);

    Testing::SetMousePosition(Vector2(150.0f, 120.0f));
    secondary->Update(0.016f);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, false);
    secondary->Update(0.016f);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, true);
    Testing::SetMousePosition(Vector2(20.0f, 20.0f), Vector2(150.0f, 120.0f));
    secondary->Update(0.016f);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, false, true);
    secondary->Update(0.016f);
    CHECK(released);
    CHECK(!clicked);

    UI::UISystem::Clear();
    UI::UISystem::Shutdown();
    Testing::ResetInputState();
}

TEST_CASE(UISystem_ProgressBarUpdatesFromProvider) {
    UI::UISystem::Init();
    UI::UISystem::Clear();

    float providerValue = 20.0f;
    UI::ProgressBarConfig config{};
    config.id = "progress_test";
    config.minValue = 0.0f;
    config.maxValue = 100.0f;
    config.value = 10.0f;
    config.showValueLabel = true;
    config.valueProvider = [&]() { return providerValue; };

    UI::ProgressBar* bar = UI::UISystem::CreateProgressBar(config);
    REQUIRE(bar != nullptr);
    CHECK(AlmostEqual(bar->GetValue(), 10.0f));
    CHECK(AlmostEqual(bar->GetNormalizedValue(), 0.1f));

    bar->SetValue(150.0f);
    CHECK(AlmostEqual(bar->GetValue(), 100.0f));
    CHECK(AlmostEqual(bar->GetNormalizedValue(), 1.0f));

    providerValue = -50.0f;
    UI::UISystem::BeginFrame(0.016f);
    CHECK(AlmostEqual(bar->GetValue(), 0.0f));
    CHECK(AlmostEqual(bar->GetNormalizedValue(), 0.0f));

    UI::UISystem::Clear();
    UI::UISystem::Shutdown();
}

TEST_CASE(UISystem_ImageInvokesProviderAndVisibility) {
    UI::UISystem::Init();
    UI::UISystem::Clear();

    bool providerInvoked = false;
    UI::ImageConfig config{};
    config.id = "image_test";
    config.size = Vector2(32.0f, 48.0f);
    config.textureProvider = [&]() {
        providerInvoked = true;
        return Ref<Texture>();
    };

    UI::Image* image = UI::UISystem::CreateImage(config);
    REQUIRE(image != nullptr);
    CHECK(image->IsVisible());
    CHECK(AlmostEqual(image->GetSize().x, 32.0f));
    CHECK(AlmostEqual(image->GetSize().y, 48.0f));

    UI::UISystem::BeginFrame(0.016f);
    CHECK(providerInvoked);

    image->SetVisible(false);
    CHECK(!image->IsVisible());

    UI::UISystem::Clear();
    UI::UISystem::Shutdown();
}

TEST_CASE(UISystem_PanelsAvoidOverlapOnCreation) {
    UI::UISystem::Init();
    UI::UISystem::Clear();
    Testing::ResetInputState();

    auto intersects = [](const Vector2& posA, const Vector2& sizeA, const Vector2& posB, const Vector2& sizeB) {
        const bool separatedX = posA.x + sizeA.x <= posB.x || posB.x + sizeB.x <= posA.x;
        const bool separatedY = posA.y + sizeA.y <= posB.y || posB.y + sizeB.y <= posA.y;
        return !(separatedX || separatedY);
    };

    UI::PanelConfig first{};
    first.id = "panel_primary";
    first.position = Vector2(80.0f, 90.0f);
    first.size = Vector2(260.0f, 200.0f);
    first.visible = true;
    UI::Panel* panelA = UI::UISystem::CreatePanel(first);
    REQUIRE(panelA != nullptr);

    UI::PanelConfig second = first;
    second.id = "panel_secondary";
    UI::Panel* panelB = UI::UISystem::CreatePanel(second);
    REQUIRE(panelB != nullptr);

    CHECK(!intersects(panelA->GetPosition(), panelA->GetSize(), panelB->GetPosition(), panelB->GetSize()));
    CHECK(!AlmostEqual(panelA->GetPosition().x, panelB->GetPosition().x) ||
        !AlmostEqual(panelA->GetPosition().y, panelB->GetPosition().y));

    UI::PanelConfig third = first;
    third.id = "panel_third";
    UI::Panel* panelC = UI::UISystem::CreatePanel(third);
    REQUIRE(panelC != nullptr);

    CHECK(!intersects(panelB->GetPosition(), panelB->GetSize(), panelC->GetPosition(), panelC->GetSize()));

    UI::UISystem::Clear();
    UI::UISystem::Shutdown();
    Testing::ResetInputState();
}

TEST_CASE(UISystem_PanelDraggingUpdatesPosition) {
    UI::UISystem::Init();
    UI::UISystem::Clear();
    Testing::ResetInputState();

    UI::PanelConfig config{};
    config.id = "draggable_panel";
    config.position = Vector2(50.0f, 60.0f);
    config.size = Vector2(240.0f, 160.0f);
    config.draggable = true;
    config.dragHandleHeight = 70.0f;
    config.constrainDragToViewport = false;
    UI::Panel* panel = UI::UISystem::CreatePanel(config);
    REQUIRE(panel != nullptr);

    Vector2 pressPoint = config.position + Vector2(12.0f, 12.0f);
    Testing::SetMousePosition(pressPoint);
    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, false);
    UI::UISystem::BeginFrame(0.016f);

    Vector2 dragPoint(220.0f, 260.0f);
    Testing::SetMousePosition(dragPoint, pressPoint);
    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, true, true);
    UI::UISystem::BeginFrame(0.016f);

    Testing::SetMouseButtonState(SAGE_MOUSE_BUTTON_LEFT, false, true);
    UI::UISystem::BeginFrame(0.016f);

    Vector2 expected(dragPoint.x - 12.0f, dragPoint.y - 12.0f);
    CHECK(AlmostEqual(panel->GetPosition().x, expected.x));
    CHECK(AlmostEqual(panel->GetPosition().y, expected.y));

    UI::UISystem::Clear();
    UI::UISystem::Shutdown();
    Testing::ResetInputState();
}
