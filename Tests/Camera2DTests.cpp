#include "catch2.hpp"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Math/Vector2.h"

using namespace SAGE;

TEST_CASE("Camera2D coordinate transformations", "[camera][graphics]") {
    Camera2D camera(800.0f, 600.0f);
    
    SECTION("Screen to world at origin") {
        camera.SetPosition({0.0f, 0.0f});
        camera.SetZoom(1.0f);
        camera.SetRotation(0.0f);
        
        // Center of screen should map to world origin
        Vector2 screenCenter{400.0f, 300.0f};
        Vector2 worldPos = camera.ScreenToWorld(screenCenter);
        
        REQUIRE(worldPos.x == Catch::Approx(0.0f).margin(1.0f));
        REQUIRE(worldPos.y == Catch::Approx(0.0f).margin(1.0f));
    }
    
    SECTION("World to screen at origin") {
        camera.SetPosition({0.0f, 0.0f});
        camera.SetZoom(1.0f);
        camera.SetRotation(0.0f);
        
        // World origin should map to screen center
        Vector2 worldOrigin{0.0f, 0.0f};
        Vector2 screenPos = camera.WorldToScreen(worldOrigin);
        
        REQUIRE(screenPos.x == Catch::Approx(400.0f).margin(1.0f));
        REQUIRE(screenPos.y == Catch::Approx(300.0f).margin(1.0f));
    }
    
    SECTION("Round-trip transformation") {
        camera.SetPosition({100.0f, 50.0f});
        camera.SetZoom(2.0f);
        
        Vector2 original{200.0f, 150.0f};
        Vector2 screen = camera.WorldToScreen(original);
        Vector2 restored = camera.ScreenToWorld(screen);
        
        REQUIRE(restored.x == Catch::Approx(original.x).margin(0.1f));
        REQUIRE(restored.y == Catch::Approx(original.y).margin(0.1f));
    }
    
    SECTION("Camera position affects transformation") {
        camera.SetPosition({100.0f, 100.0f});
        camera.SetZoom(1.0f);
        
        Vector2 worldPos{100.0f, 100.0f};
        Vector2 screenPos = camera.WorldToScreen(worldPos);
        
        // Camera is at (100, 100), so world (100, 100) should be at screen center
        REQUIRE(screenPos.x == Catch::Approx(400.0f).margin(5.0f));
        REQUIRE(screenPos.y == Catch::Approx(300.0f).margin(5.0f));
    }
    
    SECTION("Zoom affects transformation") {
        camera.SetPosition({0.0f, 0.0f});
        camera.SetZoom(2.0f);
        
        Vector2 worldPos{100.0f, 0.0f};
        Vector2 screenPos = camera.WorldToScreen(worldPos);
        
        // With 2x zoom, world positions should appear further from center
        REQUIRE(screenPos.x > 400.0f);
    }
}

TEST_CASE("Camera2D properties", "[camera][graphics]") {
    Camera2D camera(1280.0f, 720.0f);
    
    SECTION("Default state") {
        REQUIRE(camera.GetPosition().x == 0.0f);
        REQUIRE(camera.GetPosition().y == 0.0f);
        REQUIRE(camera.GetZoom() == 1.0f);
        REQUIRE(camera.GetRotation() == 0.0f);
    }
    
    SECTION("Set position") {
        camera.SetPosition({100.0f, 200.0f});
        REQUIRE(camera.GetPosition().x == 100.0f);
        REQUIRE(camera.GetPosition().y == 200.0f);
    }
    
    SECTION("Set zoom") {
        camera.SetZoom(2.5f);
        REQUIRE(camera.GetZoom() == Catch::Approx(2.5f));
    }
    
    SECTION("Zoom clamping") {
        camera.SetZoom(0.0f);
        REQUIRE(camera.GetZoom() > 0.0f); // Should be clamped to minimum
        
        camera.SetZoom(-1.0f);
        REQUIRE(camera.GetZoom() > 0.0f);
    }
    
    SECTION("Set rotation") {
        camera.SetRotation(1.57f);
        REQUIRE(camera.GetRotation() == Catch::Approx(1.57f));
    }
    
    SECTION("Viewport resize") {
        camera.SetViewportSize(1920.0f, 1080.0f);
        
        // Verify transformations still work after resize
        Vector2 worldOrigin{0.0f, 0.0f};
        Vector2 screenPos = camera.WorldToScreen(worldOrigin);
        
        // Allow for small floating-point precision errors in matrix calculations
        REQUIRE(screenPos.x == Catch::Approx(960.0f).margin(2.5f));
        REQUIRE(screenPos.y == Catch::Approx(540.0f).margin(2.5f));
    }
}
