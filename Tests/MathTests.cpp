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
