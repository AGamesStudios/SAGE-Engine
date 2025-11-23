#include "catch2.hpp"
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Matrix3.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Math/Rect.h"

using namespace SAGE;

TEST_CASE("Vector2 operations", "[math][vector2]") {
    SECTION("Default construction") {
        Vector2 v;
        REQUIRE(v.x == 0.0f);
        REQUIRE(v.y == 0.0f);
    }

    SECTION("Value construction") {
        Vector2 v{3.0f, 4.0f};
        REQUIRE(v.x == 3.0f);
        REQUIRE(v.y == 4.0f);
    }

    SECTION("Addition") {
        Vector2 a{1.0f, 2.0f};
        Vector2 b{3.0f, 4.0f};
        Vector2 c = a + b;
        REQUIRE(c.x == 4.0f);
        REQUIRE(c.y == 6.0f);
    }

    SECTION("Subtraction") {
        Vector2 a{5.0f, 7.0f};
        Vector2 b{2.0f, 3.0f};
        Vector2 c = a - b;
        REQUIRE(c.x == 3.0f);
        REQUIRE(c.y == 4.0f);
    }

    SECTION("Scalar multiplication") {
        Vector2 v{2.0f, 3.0f};
        Vector2 result = v * 2.0f;
        REQUIRE(result.x == 4.0f);
        REQUIRE(result.y == 6.0f);
    }

    SECTION("Length") {
        Vector2 v{3.0f, 4.0f};
        REQUIRE(v.Length() == Catch::Approx(5.0f));
    }

    SECTION("Normalize") {
        Vector2 v{3.0f, 4.0f};
        Vector2 n = v.Normalized();
        REQUIRE(n.Length() == Catch::Approx(1.0f));
        REQUIRE(n.x == Catch::Approx(0.6f));
        REQUIRE(n.y == Catch::Approx(0.8f));
    }

    SECTION("Dot product") {
        Vector2 a{1.0f, 0.0f};
        Vector2 b{0.0f, 1.0f};
        REQUIRE(Vector2::Dot(a, b) == Catch::Approx(0.0f));
        
        Vector2 c{1.0f, 1.0f};
        Vector2 d{1.0f, 1.0f};
        REQUIRE(Vector2::Dot(c, d) == Catch::Approx(2.0f));
    }

    SECTION("Static constructors") {
        REQUIRE(Vector2::Zero().x == 0.0f);
        REQUIRE(Vector2::Zero().y == 0.0f);
        REQUIRE(Vector2::One().x == 1.0f);
        REQUIRE(Vector2::One().y == 1.0f);
    }
}

TEST_CASE("Matrix3 operations", "[math][matrix3]") {
    SECTION("Identity matrix") {
        Matrix3 m = Matrix3::Identity();
        REQUIRE(m.m[0] == 1.0f);
        REQUIRE(m.m[4] == 1.0f);
        REQUIRE(m.m[8] == 1.0f);
        REQUIRE(m.m[1] == 0.0f);
    }

    SECTION("Translation") {
        Matrix3 t = Matrix3::Translation({10.0f, 20.0f});
        Vector2 p = t.TransformPoint({0.0f, 0.0f});
        REQUIRE(p.x == Catch::Approx(10.0f));
        REQUIRE(p.y == Catch::Approx(20.0f));
    }

    SECTION("Scale") {
        Matrix3 s = Matrix3::Scale({2.0f, 3.0f});
        Vector2 p = s.TransformPoint({1.0f, 1.0f});
        REQUIRE(p.x == Catch::Approx(2.0f));
        REQUIRE(p.y == Catch::Approx(3.0f));
    }

    SECTION("Rotation 90 degrees") {
        Matrix3 r = Matrix3::Rotation(1.5707963f); // 90 degrees
        Vector2 p = r.TransformPoint({1.0f, 0.0f});
        REQUIRE(p.x == Catch::Approx(0.0f).margin(0.001f));
        REQUIRE(p.y == Catch::Approx(1.0f).margin(0.001f));
    }

    SECTION("Matrix multiplication") {
        Matrix3 t = Matrix3::Translation({10.0f, 20.0f});
        Matrix3 s = Matrix3::Scale({2.0f, 2.0f});
        Matrix3 combined = t * s;
        
        Vector2 p = combined.TransformPoint({1.0f, 1.0f});
        REQUIRE(p.x == Catch::Approx(12.0f));
        REQUIRE(p.y == Catch::Approx(22.0f));
    }

    SECTION("Matrix inverse") {
        Matrix3 t = Matrix3::Translation({10.0f, 20.0f});
        Matrix3 inv = t.Inverse();
        
        Vector2 original{5.0f, 7.0f};
        Vector2 transformed = t.TransformPoint(original);
        Vector2 restored = inv.TransformPoint(transformed);
        
        REQUIRE(restored.x == Catch::Approx(original.x));
        REQUIRE(restored.y == Catch::Approx(original.y));
    }

    SECTION("Orthographic projection") {
        Matrix3 ortho = Matrix3::Ortho(0.0f, 800.0f, 0.0f, 600.0f);
        
        // Center should map to origin
        Vector2 center = ortho.TransformPoint({400.0f, 300.0f});
        REQUIRE(center.x == Catch::Approx(0.0f).margin(0.01f));
        REQUIRE(center.y == Catch::Approx(0.0f).margin(0.01f));
    }
}

TEST_CASE("Color operations", "[math][color]") {
    SECTION("Default construction") {
        Color c;
        REQUIRE(c.r == 1.0f);  // Default is white
        REQUIRE(c.g == 1.0f);
        REQUIRE(c.b == 1.0f);
        REQUIRE(c.a == 1.0f);
    }

    SECTION("FromRGBA") {
        Color c = Color::FromRGBA(255, 128, 64, 32);
        REQUIRE(c.r == Catch::Approx(1.0f).margin(0.01f));
        REQUIRE(c.g == Catch::Approx(0.5f).margin(0.01f));
        REQUIRE(c.b == Catch::Approx(0.25f).margin(0.01f));
        REQUIRE(c.a == Catch::Approx(0.125f).margin(0.01f));
    }

    SECTION("Predefined colors") {
        Color white = Color::White();
        REQUIRE(white.r == 1.0f);
        REQUIRE(white.g == 1.0f);
        REQUIRE(white.b == 1.0f);
        REQUIRE(white.a == 1.0f);

        Color black = Color::Black();
        REQUIRE(black.r == 0.0f);
        REQUIRE(black.g == 0.0f);
        REQUIRE(black.b == 0.0f);
    }
}

TEST_CASE("Rect operations", "[math][rect]") {
    SECTION("Construction") {
        Rect r{10.0f, 20.0f, 100.0f, 50.0f};
        REQUIRE(r.x == 10.0f);
        REQUIRE(r.y == 20.0f);
        REQUIRE(r.width == 100.0f);
        REQUIRE(r.height == 50.0f);
    }

    SECTION("Contains point") {
        Rect r{0.0f, 0.0f, 100.0f, 100.0f};
        REQUIRE(r.Contains({50.0f, 50.0f}));
        REQUIRE_FALSE(r.Contains({150.0f, 50.0f}));
    }

    SECTION("Intersection") {
        Rect a{0.0f, 0.0f, 100.0f, 100.0f};
        Rect b{50.0f, 50.0f, 100.0f, 100.0f};
        REQUIRE(a.Intersects(b));

        Rect c{200.0f, 200.0f, 50.0f, 50.0f};
        REQUIRE_FALSE(a.Intersects(c));
    }

    SECTION("Bounds") {
        Rect r{10.0f, 20.0f, 100.0f, 50.0f};
        REQUIRE(r.Left() == 10.0f);
        REQUIRE(r.Right() == 110.0f);
        REQUIRE(r.Bottom() == 20.0f);
        REQUIRE(r.Top() == 70.0f);
    }
}
