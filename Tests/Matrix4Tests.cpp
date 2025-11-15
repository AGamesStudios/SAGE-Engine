#include "TestFramework.h"

#include <Engine/Math/Matrix4.h>

#include <algorithm>
#include <array>
#include <cmath>

using namespace SAGE;

namespace {

    constexpr float kEpsilon = 1e-5f;

    std::array<float, 16> CopyData(const Matrix4& m) {
        std::array<float, 16> out{};
        std::copy(m.Data(), m.Data() + out.size(), out.begin());
        return out;
    }

    struct Vec4 {
        float x;
        float y;
        float z;
        float w;
    };

    Vec4 Apply(const Matrix4& m, const Vec4& v) {
        const float* data = m.Data();
        return Vec4{
            data[0] * v.x + data[4] * v.y + data[8] * v.z + data[12] * v.w,
            data[1] * v.x + data[5] * v.y + data[9] * v.z + data[13] * v.w,
            data[2] * v.x + data[6] * v.y + data[10] * v.z + data[14] * v.w,
            data[3] * v.x + data[7] * v.y + data[11] * v.z + data[15] * v.w
        };
    }

} // namespace

TEST_CASE(Matrix4_Identity_IsIdentityMatrix) {
    const Matrix4 identity = Matrix4::Identity();
    const auto data = CopyData(identity);

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            const float expected = (row == col) ? 1.0f : 0.0f;
            const float actual = data[col * 4 + row];
            ASSERT_NEAR(actual, expected, kEpsilon);
        }
    }
}

TEST_CASE(Matrix4_Multiplication_ComposesTransformations) {
    const Matrix4 scale = Matrix4::Scale(2.0f, 3.0f, 4.0f);
    const Matrix4 translate = Matrix4::Translate(5.0f, 6.0f, 7.0f);

    const Matrix4 combined = scale * translate;
    const auto data = CopyData(combined);

    ASSERT_NEAR(data[0], 2.0f, kEpsilon);
    ASSERT_NEAR(data[5], 3.0f, kEpsilon);
    ASSERT_NEAR(data[10], 4.0f, kEpsilon);
    ASSERT_NEAR(data[15], 1.0f, kEpsilon);

    ASSERT_NEAR(data[12], 10.0f, kEpsilon);
    ASSERT_NEAR(data[13], 18.0f, kEpsilon);
    ASSERT_NEAR(data[14], 28.0f, kEpsilon);
}

TEST_CASE(Matrix4_OrthographicProjection_HasExpectedCoefficients) {
    const Matrix4 ortho = Matrix4::Orthographic(-10.0f, 30.0f, -5.0f, 15.0f, 1.0f, 9.0f);
    const auto data = CopyData(ortho);

    ASSERT_NEAR(data[0], 0.05f, kEpsilon);
    ASSERT_NEAR(data[5], 0.10f, kEpsilon);
    ASSERT_NEAR(data[10], -0.25f, kEpsilon);
    ASSERT_NEAR(data[15], 1.0f, kEpsilon);

    ASSERT_NEAR(data[12], -0.5f, kEpsilon);
    ASSERT_NEAR(data[13], -0.5f, kEpsilon);
    ASSERT_NEAR(data[14], -1.25f, kEpsilon);
}

TEST_CASE(Matrix4_Apply_TransformsPointCorrectly) {
    constexpr float halfPi = 3.14159265358979323846f * 0.5f;

    const Matrix4 transform = Matrix4::Translate(5.0f, -3.0f, 1.0f)
        * Matrix4::RotateZ(halfPi)
        * Matrix4::Scale(2.0f, 3.0f, 1.0f);

    const Vec4 result = Apply(transform, Vec4{ 1.0f, 2.0f, 0.0f, 1.0f });

    ASSERT_NEAR(result.x, -1.0f, kEpsilon);
    ASSERT_NEAR(result.y, -1.0f, kEpsilon);
    ASSERT_NEAR(result.z, 1.0f, kEpsilon);
    ASSERT_NEAR(result.w, 1.0f, kEpsilon);
}

TEST_CASE(Matrix4_InverseComposition_YieldsIdentity) {
    const Matrix4 transform = Matrix4::Translate(2.0f, -4.0f, 0.5f)
        * Matrix4::RotateZ(0.35f)
        * Matrix4::Scale(2.0f, 0.5f, 1.5f);

    const Matrix4 inverse = Matrix4::Scale(0.5f, 2.0f, 1.0f / 1.5f)
        * Matrix4::RotateZ(-0.35f)
        * Matrix4::Translate(-2.0f, 4.0f, -0.5f);

    const Matrix4 product = transform * inverse;
    const auto data = CopyData(product);

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            const float expected = (row == col) ? 1.0f : 0.0f;
            const float actual = data[col * 4 + row];
            ASSERT_NEAR(actual, expected, 5e-4f);
        }
    }
}

TEST_CASE(Matrix4_ZeroScale_CollapsesAxes) {
    const Matrix4 zeroScale = Matrix4::Scale(0.0f, 3.0f, 0.0f);
    const Vec4 result = Apply(zeroScale, Vec4{ 4.0f, 5.0f, 6.0f, 1.0f });

    ASSERT_NEAR(result.x, 0.0f, kEpsilon);
    ASSERT_NEAR(result.y, 15.0f, kEpsilon);
    ASSERT_NEAR(result.z, 0.0f, kEpsilon);
    ASSERT_NEAR(result.w, 1.0f, kEpsilon);
}

// Force linker to include this translation unit
namespace {
    [[maybe_unused]] int __force_matrix4_tests_registration = []() {
        return 0;
    }();
}
