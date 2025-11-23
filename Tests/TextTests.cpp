#include "catch2.hpp"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Renderer.h"

using namespace SAGE;

TEST_CASE("Font System Robustness", "[Graphics][Text]") {
    SECTION("Font Load handles missing file gracefully") {
        auto font = std::make_shared<Font>();
        bool result = font->Load("non_existent_font.ttf");
        REQUIRE_FALSE(result);
    }

    SECTION("Font Load handles missing OpenGL context gracefully (or fails safely)") {
        // This test assumes no OpenGL context is active in the test runner
        auto font = std::make_shared<Font>();
        // We expect this to fail either because of file not found or GL init failure
        // We just want to ensure it doesn't crash
        bool result = font->Load("assets/fonts/default.ttf"); // Assuming this file might exist or not
        REQUIRE_FALSE(result); 
    }
}
