#pragma once

#include <cmath>
#include <functional>
#include <string>
#include <type_traits>

namespace TestFramework {

    struct TestContext;

    using TestFunc = std::function<void(TestContext&)>;

    void Register(const std::string& name, TestFunc func);
    int RunAllTests();

    struct TestContext {
        void Check(bool condition, const char* expr, const char* file, int line);
        int Failures = 0;
    };

    struct Approx {
        double value;
        double epsilon;

        constexpr explicit Approx(double v, double e = 0.0001)
            : value(v), epsilon(e) {}

        constexpr Approx& margin(double e) {
            epsilon = e;
            return *this;
        }
    };

    inline constexpr Approx Approximate(double value, double epsilon = 0.0001) {
        return Approx(value, epsilon);
    }

    template<typename Arithmetic>
    inline bool ApproximatelyEqual(Arithmetic actual, const Approx& expected) {
        static_assert(std::is_arithmetic_v<Arithmetic>, "Approx comparisons require arithmetic types");
        double lhs = static_cast<double>(actual);
        return std::fabs(lhs - expected.value) <= expected.epsilon;
    }

} // namespace TestFramework

#define TEST_CASE(name) \
    static void name(TestFramework::TestContext&); \
    namespace { \
        struct name##_registrar { \
            name##_registrar() { TestFramework::Register(#name, name); } \
        }; \
        static name##_registrar name##_registrar_instance; \
    } \
    static void name(TestFramework::TestContext& ctx)

#define CHECK(expr) (ctx.Check((expr), #expr, __FILE__, __LINE__))
#define REQUIRE(expr) do { if(!(expr)) { ctx.Check(false, #expr, __FILE__, __LINE__); return; } } while(0)
#define CHECK_FALSE(expr) (ctx.Check(!(expr), "!(" #expr ")", __FILE__, __LINE__))
#define REQUIRE_FALSE(expr) do { if((expr)) { ctx.Check(false, "!(" #expr ")", __FILE__, __LINE__); return; } } while(0)

template<typename Arithmetic>
inline bool operator==(Arithmetic actual, const TestFramework::Approx& expected) {
    return TestFramework::ApproximatelyEqual(actual, expected);
}

template<typename Arithmetic>
inline bool operator==(const TestFramework::Approx& expected, Arithmetic actual) {
    return TestFramework::ApproximatelyEqual(actual, expected);
}

template<typename Arithmetic>
inline bool operator!=(Arithmetic actual, const TestFramework::Approx& expected) {
    return !TestFramework::ApproximatelyEqual(actual, expected);
}

template<typename Arithmetic>
inline bool operator!=(const TestFramework::Approx& expected, Arithmetic actual) {
    return !TestFramework::ApproximatelyEqual(actual, expected);
}

inline constexpr TestFramework::Approx Approx(double value, double epsilon = 0.0001) {
    return TestFramework::Approx(value, epsilon);
}
