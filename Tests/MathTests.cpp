#include "TestFramework.h"

#include <SAGE.h>
#include <cmath>

using namespace SAGE;

TEST_CASE(Vector2_BasicOperations) {
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 sum = a + b;
    CHECK(sum.x == 4.0f);
    CHECK(sum.y == 6.0f);

    Vector2 diff = b - a;
    CHECK(diff.x == 2.0f);
    CHECK(diff.y == 2.0f);
}

TEST_CASE(Vector2_Scaling) {
    Vector2 direction(0.5f, -1.5f);
    Vector2 scaled = direction * 4.0f;
    CHECK(std::abs(scaled.x - 2.0f) < 0.0001f);
    CHECK(std::abs(scaled.y + 6.0f) < 0.0001f);
}

TEST_CASE(Vector2_Normalization) {
    Vector2 value(3.0f, 4.0f);
    Vector2 normalized = value.Normalized();
    CHECK(std::abs(normalized.Length() - 1.0f) < 0.0001f);
    CHECK(std::abs(normalized.x - 0.6f) < 0.0001f);
    CHECK(std::abs(normalized.y - 0.8f) < 0.0001f);
}

TEST_CASE(Vector2_DotProduct) {
    Vector2 forward = Vector2::Right();
    Vector2 up = Vector2::Up();
    CHECK(std::abs(Vector2::Dot(forward, up)) < 0.0001f);

    Vector2 diag(1.0f, 1.0f);
    CHECK(std::abs(Vector2::Dot(diag, diag) - 2.0f) < 0.0001f);
}

TEST_CASE(Camera2D_WorldScreen_RoundTrip) {
    Camera2D camera; // default 800x600
    camera.SetPosition(100.0f, 50.0f);
    camera.SetZoom(2.0f);

    Vector2 worldPoint{130.0f, 70.0f};
    Vector2 screenPoint = camera.WorldToScreen(worldPoint);
    Vector2 worldFromScreen = camera.ScreenToWorld(screenPoint);

    // round-trip precision
    CHECK(std::abs(worldFromScreen.x - worldPoint.x) < 0.001f);
    CHECK(std::abs(worldFromScreen.y - worldPoint.y) < 0.001f);
}

TEST_CASE(Camera2D_RotatedZoomed_RoundTrip) {
    Camera2D camera;
    camera.SetViewportSize(1024, 768);
    camera.SetPosition(200.0f, -100.0f);
    camera.SetZoom(1.5f);
    camera.SetRotationRadians(0.785398f); // ~45 градусов

    Vector2 testPoints[] = {
        {150.0f, -50.0f},
        {250.0f, -150.0f},
        {200.0f, -100.0f}, // camera center
        {0.0f, 0.0f}
    };

    for (const auto& wp : testPoints) {
        Vector2 sp = camera.WorldToScreen(wp);
        Vector2 back = camera.ScreenToWorld(sp);
        CHECK(std::abs(back.x - wp.x) < 0.01f);
        CHECK(std::abs(back.y - wp.y) < 0.01f);
    }
}

TEST_CASE(Camera2D_ExtremeZoom_Stability) {
    Camera2D camera;
    camera.SetPosition(0.0f, 0.0f);

    // Very high zoom
    camera.SetZoom(10.0f);
    Vector2 w1{5.0f, 5.0f};
    Vector2 s1 = camera.WorldToScreen(w1);
    Vector2 b1 = camera.ScreenToWorld(s1);
    CHECK(std::abs(b1.x - w1.x) < 0.01f);
    CHECK(std::abs(b1.y - w1.y) < 0.01f);

    // Very low zoom
    camera.SetZoom(0.1f);
    Vector2 w2{500.0f, 300.0f};
    Vector2 s2 = camera.WorldToScreen(w2);
    Vector2 b2 = camera.ScreenToWorld(s2);
    CHECK(std::abs(b2.x - w2.x) < 0.5f);
    CHECK(std::abs(b2.y - w2.y) < 0.5f);
}

