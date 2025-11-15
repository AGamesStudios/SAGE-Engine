#pragma once

#include <cmath>
#include <functional>
#include <iosfwd>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace TestFramework {

    struct TestContext;
    struct TestCase;

    using TestFunc = std::function<void(TestContext&)>;

    void Register(const std::string& name, TestFunc func);
    int RunAllTests(const std::string& filter = {});
    void ListTests(std::ostream& stream);
    const std::vector<TestCase>& GetTests();
    bool MatchesFilter(const std::string& name, const std::string& filter);

    struct FailureRecord {
        std::string Expression;
        std::string File;
        int Line = 0;
    };

    struct TestContext {
        void Check(bool condition, const char* expr, const char* file, int line);
        void Fail(const char* message, const char* file, int line);
        void SetImmediateFailureOutput(bool enabled);

        int Failures = 0;
        std::vector<FailureRecord> FailureDetails;

    private:
        bool m_ImmediateFailureOutput = true;
    };

    struct TestCase {
        std::string Name;
        TestFunc Function;
    };

    namespace detail {

        inline std::string BuildOptionalMessage() {
            return {};
        }

        template<typename First, typename... Rest>
        inline std::string BuildOptionalMessage(First&& first, Rest&&... rest) {
            std::ostringstream stream;
            stream << std::forward<First>(first);
            ((stream << std::forward<Rest>(rest)), ...);
            return stream.str();
        }

        template<typename... MessageParts>
        inline void ReportFailure(TestContext& ctx, const char* expr, const char* file, int line, MessageParts&&... parts) {
            if constexpr (sizeof...(parts) == 0) {
                ctx.Fail(expr, file, line);
            } else {
                std::string message = BuildOptionalMessage(std::forward<MessageParts>(parts)...);
                std::ostringstream stream;
                if (expr && expr[0] != '\0') {
                    stream << expr;
                }
                if (!message.empty()) {
                    if (stream.tellp() > 0) {
                        stream << " | ";
                    }
                    stream << message;
                }
                const std::string combined = stream.str();
                ctx.Fail(combined.empty() ? expr : combined.c_str(), file, line);
            }
        }

        template<typename... MessageParts>
        inline void AssertCondition(TestContext& ctx, bool condition, const char* expr, const char* file, int line, MessageParts&&... parts) {
            if (condition) {
                return;
            }
            ReportFailure(ctx, expr, file, line, std::forward<MessageParts>(parts)...);
        }

        template<typename Expected, typename Actual, typename... MessageParts>
        inline void AssertEqual(TestContext& ctx, const Expected& expected, const Actual& actual, const char* expr, const char* file, int line, MessageParts&&... parts) {
            if (expected == actual) {
                return;
            }
            std::ostringstream stream;
            stream << "expected: " << expected << ", actual: " << actual;
            if constexpr (sizeof...(parts) > 0) {
                stream << " | ";
                (stream << ... << std::forward<MessageParts>(parts));
            }
            ReportFailure(ctx, expr, file, line, stream.str());
        }

        template<typename Expected, typename Actual, typename... MessageParts>
        inline void AssertNotEqual(TestContext& ctx, const Expected& expected, const Actual& actual, const char* expr, const char* file, int line, MessageParts&&... parts) {
            if (expected != actual) {
                return;
            }
            std::ostringstream stream;
            stream << "values unexpectedly equal: " << expected;
            if constexpr (sizeof...(parts) > 0) {
                stream << " | ";
                (stream << ... << std::forward<MessageParts>(parts));
            }
            ReportFailure(ctx, expr, file, line, stream.str());
        }

        template<typename LHS, typename RHS, typename Epsilon, typename... MessageParts>
        inline void AssertNear(TestContext& ctx, const LHS& lhs, const RHS& rhs, const Epsilon& epsilon, const char* expr, const char* file, int line, MessageParts&&... parts) {
            static_assert(std::is_arithmetic_v<std::decay_t<LHS>>, "ASSERT_NEAR expects arithmetic types");
            static_assert(std::is_arithmetic_v<std::decay_t<RHS>>, "ASSERT_NEAR expects arithmetic types");
            static_assert(std::is_arithmetic_v<std::decay_t<Epsilon>>, "ASSERT_NEAR epsilon expects an arithmetic type");

            const double lhsValue = static_cast<double>(lhs);
            const double rhsValue = static_cast<double>(rhs);
            const double margin = std::fabs(static_cast<double>(epsilon));
            const double diff = std::fabs(lhsValue - rhsValue);

            if (diff <= margin) {
                return;
            }

            std::ostringstream stream;
            stream << "|lhs - rhs| = " << diff << ", allowed = " << margin;
            if constexpr (sizeof...(parts) > 0) {
                stream << " | ";
                (stream << ... << std::forward<MessageParts>(parts));
            }
            ReportFailure(ctx, expr, file, line, stream.str());
        }

    } // namespace detail

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

#define TEST(name) TEST_CASE(name)

#define ASSERT(condition, ...) do { \
    const bool sage_tf_condition = static_cast<bool>(condition); \
    TestFramework::detail::AssertCondition(ctx, sage_tf_condition, #condition, __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define ASSERT_TRUE(condition, ...) ASSERT((condition), ##__VA_ARGS__)
#define ASSERT_FALSE(condition, ...) ASSERT(!(condition), ##__VA_ARGS__)

#define ASSERT_EQ(expected, actual, ...) do { \
    auto sage_tf_expected = (expected); \
    auto sage_tf_actual = (actual); \
    TestFramework::detail::AssertEqual(ctx, sage_tf_expected, sage_tf_actual, "ASSERT_EQ(" #expected ", " #actual ")", __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define ASSERT_NE(expected, actual, ...) do { \
    auto sage_tf_expected = (expected); \
    auto sage_tf_actual = (actual); \
    TestFramework::detail::AssertNotEqual(ctx, sage_tf_expected, sage_tf_actual, "ASSERT_NE(" #expected ", " #actual ")", __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define ASSERT_NEAR(a, b, epsilon, ...) do { \
    auto sage_tf_lhs = (a); \
    auto sage_tf_rhs = (b); \
    auto sage_tf_eps = (epsilon); \
    TestFramework::detail::AssertNear(ctx, sage_tf_lhs, sage_tf_rhs, sage_tf_eps, "ASSERT_NEAR(" #a ", " #b ", " #epsilon ")", __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define ASSERT_NOT_NULL(value, ...) do { \
    const auto& sage_tf_val = (value); \
    const bool sage_tf_is_null = (sage_tf_val == nullptr || !sage_tf_val); \
    TestFramework::detail::AssertCondition(ctx, !sage_tf_is_null, "ASSERT_NOT_NULL(" #value ")", __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define ASSERT_NULL(value, ...) do { \
    auto* sage_tf_ptr = (value); \
    TestFramework::detail::AssertCondition(ctx, sage_tf_ptr == nullptr, "ASSERT_NULL(" #value ")", __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define CHECK_EQ(expected, actual, ...) ASSERT_EQ(expected, actual, ##__VA_ARGS__)
#define CHECK_NE(expected, actual, ...) ASSERT_NE(expected, actual, ##__VA_ARGS__)
#define CHECK_NEAR(a, b, epsilon, ...) ASSERT_NEAR(a, b, epsilon, ##__VA_ARGS__)
#define CHECK_NULL(value, ...) ASSERT_NULL(value, ##__VA_ARGS__)
#define CHECK_NOT_NULL(value, ...) ASSERT_NOT_NULL(value, ##__VA_ARGS__)

#define CHECK(expr) ASSERT(expr)
#define CHECK_NOTHROW(expr) do { bool threw = false; try { expr; } catch (...) { threw = true; } ctx.Check(!threw, #expr " threw an exception", __FILE__, __LINE__); } while(0)
#define REQUIRE(expr) do { if(!(expr)) { ctx.Check(false, #expr, __FILE__, __LINE__); return; } } while(0)
#define CHECK_FALSE(expr) ASSERT(!(expr))
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
