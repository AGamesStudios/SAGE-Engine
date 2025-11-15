#include "TestFramework.h"

#include "Math/Random.h"

#include <cmath>

using namespace SAGE::Math;
using SAGE::Vector2;
using SAGE::Vector3;

TEST_CASE(Random_DeterministicSequence) {
    Random a(12345u);
    Random b(12345u);

    for (int i = 0; i < 16; ++i) {
        float fa = a.NextFloat();
        float fb = b.NextFloat();
        CHECK(std::abs(fa - fb) < 1e-6f);

        int ia = a.NextInt(-10, 10);
        int ib = b.NextInt(-10, 10);
        CHECK(ia == ib);
    }
}

TEST_CASE(Random_RangeClamping) {
    Random rng(777u);

    for (int i = 0; i < 64; ++i) {
        float value = rng.NextRange(5.0f, 10.0f);
        CHECK(value >= 5.0f);
        CHECK(value <= 10.0f);

        float reversed = rng.NextRange(10.0f, 5.0f);
        CHECK(reversed >= 5.0f);
        CHECK(reversed <= 10.0f);

        int integer = rng.NextInt(3, -3);
        CHECK(integer >= -3);
        CHECK(integer <= 3);
    }
}

TEST_CASE(Random_UnitVectorsHaveUnitLength) {
    Random rng(555u);

    for (int i = 0; i < 32; ++i) {
        Vector2 v2 = rng.NextUnitVector2();
        CHECK(std::abs(v2.Length() - 1.0f) < 1e-3f);

        Vector3 v3 = rng.NextUnitVector3();
        float len3 = std::sqrt(v3.x * v3.x + v3.y * v3.y + v3.z * v3.z);
        CHECK(std::abs(len3 - 1.0f) < 1e-3f);
    }
}
