#include "TestFramework.h"
#include "Engine/Graphics/Core/Resources/Sprite.h"
#include "Engine/Graphics/Core/Resources/Texture.h"

using namespace SAGE;

TEST_CASE("Sprite_Creation") {
    Ref<Texture> texture = CreateRef<Texture>(128, 128);
    Sprite sprite(texture);
    
    Rect region = sprite.GetTextureRegion();
    REQUIRE(region.x == 0.0f);
    REQUIRE(region.y == 0.0f);
    REQUIRE(region.width == 128.0f);
    REQUIRE(region.height == 128.0f);
}

TEST_CASE("Sprite_CustomRegion") {
    Ref<Texture> texture = CreateRef<Texture>(128, 128);
    Rect customRegion = {32.0f, 32.0f, 64.0f, 64.0f};
    
    Sprite sprite(texture, customRegion);
    Rect region = sprite.GetTextureRegion();
    
    REQUIRE(region.x == customRegion.x);
    REQUIRE(region.y == customRegion.y);
    REQUIRE(region.width == customRegion.width);
    REQUIRE(region.height == customRegion.height);
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

TEST_CASE("Sprite_FramesAnimation") {
    Ref<Texture> texture = CreateRef<Texture>(64, 32);
    std::vector<Rect> frames = {
        Rect(0,   0, 32, 32),
        Rect(32,  0, 32, 32)
    };
    Sprite sprite(texture);
    sprite.SetFrames(frames);
    REQUIRE(sprite.GetFrameCount() == 2);
    REQUIRE(sprite.GetFrameIndex() == 0);
    // Advance one frame
    sprite.AdvanceFrame();
    REQUIRE(sprite.GetFrameIndex() == 1);
    // Region must reflect second frame
    Rect r = sprite.GetTextureRegion();
    REQUIRE(r.x == 32.0f);
    REQUIRE(r.y == 0.0f);
    REQUIRE(r.width == 32.0f);
    REQUIRE(r.height == 32.0f);
    // Loop back
    sprite.AdvanceFrame();
    REQUIRE(sprite.GetFrameIndex() == 0);
}

    TEST_CASE("Sprite basic constructors") {
        Ref<Texture> tex = CreateTestTexture(64, 32);
        Sprite s(tex);
        CHECK(s.HasTexture());
        CHECK(s.GetSize().x == 64);
        CHECK(s.GetSize().y == 32);
        CHECK(!s.IsSolidColor());
    
        Sprite colorOnly(Float2(10,20), 0.2f,0.3f,0.4f,1.0f);
        CHECK(colorOnly.IsSolidColor());
    }

    TEST_CASE("Sprite per-effect draw") {
        Ref<Texture> tex = CreateTestTexture(32, 32);
        Sprite s(tex);
        QuadEffect eff; eff.Type = QuadEffectType::Tint; eff.Data0 = Float4(0.2f,0.7f,0.9f,1.0f);
        s.SetPosition(Float2(10,10));
        s.SetEffect(eff);
        CHECK(s.Draw());
        // After draw effect should be popped automatically -> subsequent draw without effect should still succeed
        QuadEffect none; none.Type = QuadEffectType::None; s.SetEffect(none);
        CHECK(s.Draw());
    }

    TEST_CASE("Sprite pivot presets") {
        Ref<Texture> tex = CreateTestTexture(16, 16);
        Sprite s(tex);
        s.SetPivotPreset(Sprite::PivotPreset::TopLeft); CHECK(s.GetOrigin().x == Approx(0.0f)); CHECK(s.GetOrigin().y == Approx(0.0f));
        s.SetPivotPreset(Sprite::PivotPreset::Center); CHECK(s.GetOrigin().x == Approx(0.5f)); CHECK(s.GetOrigin().y == Approx(0.5f));
        s.SetPivotPreset(Sprite::PivotPreset::BottomRight); CHECK(s.GetOrigin().x == Approx(1.0f)); CHECK(s.GetOrigin().y == Approx(1.0f));
        s.SetPivotPixels(8,8); CHECK(s.GetOrigin().x == Approx(0.5f)); CHECK(s.GetOrigin().y == Approx(0.5f));
    }
