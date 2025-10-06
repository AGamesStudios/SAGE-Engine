#include "TestFramework.h"
#include "../Engine/Graphics/Sprite.h"
#include "../Engine/Graphics/Texture.h"

using namespace SAGE;

TEST_CASE("Sprite_Creation") {
    Ref<Texture> texture = CreateRef<Texture>(128, 128);
    Sprite sprite(texture);
    
    Rect region = sprite.GetTextureRegion();
    REQUIRE(region.x == 0.0f);
    REQUIRE(region.y == 0.0f);
    REQUIRE(region.w == 128.0f);
    REQUIRE(region.h == 128.0f);
}

TEST_CASE("Sprite_CustomRegion") {
    Ref<Texture> texture = CreateRef<Texture>(128, 128);
    Rect customRegion = {32.0f, 32.0f, 64.0f, 64.0f};
    
    Sprite sprite(texture, customRegion);
    Rect region = sprite.GetTextureRegion();
    
    REQUIRE(region.x == customRegion.x);
    REQUIRE(region.y == customRegion.y);
    REQUIRE(region.w == customRegion.w);
    REQUIRE(region.h == customRegion.h);
}

TEST_CASE("Sprite_TexCoords") {
    Ref<Texture> texture = CreateRef<Texture>(100, 100);
    Rect region = {0.0f, 0.0f, 50.0f, 50.0f};
    Sprite sprite(texture, region);
    
    auto [uvMin, uvMax] = sprite.GetUVCoords();
    
    REQUIRE(uvMin.x >= 0.0f && uvMin.x <= 1.0f);
    REQUIRE(uvMin.y >= 0.0f && uvMin.y <= 1.0f);
    REQUIRE(uvMax.x >= 0.0f && uvMax.x <= 1.0f);
    REQUIRE(uvMax.y >= 0.0f && uvMax.y <= 1.0f);
}
