#pragma once

#include <string>
#include <optional>
#include <stdexcept>

#if SAGE_ENABLE_LUA
    #include <sol/sol.hpp>
#else
namespace sol {
    struct nil_t {};
    inline constexpr nil_t nil{};
    inline constexpr nil_t lua_nil{};

    class state {
    public:
        state() = default;
    };

    class table {
    public:
        table() = default;
    };

    class object {
    public:
        object() = default;
        object(nil_t) {}
        bool valid() const { return false; }
    };

    class variadic_args {
    public:
        variadic_args() = default;
    };

    class protected_function_result;

    class protected_function {
    public:
        protected_function() = default;
        bool valid() const { return false; }

        template<typename... Args>
        protected_function_result operator()(Args&&...) {
            return {};
        }
    };

    class protected_function_result {
    public:
        protected_function_result() = default;
        bool valid() const { return false; }

        template<typename T>
        T get() const {
            return T{};
        }
    };

    class thread {
    public:
        thread() = default;
    };

    class coroutine {
    public:
        coroutine() = default;
        explicit operator bool() const { return false; }
    };

    template<typename... Functions>
    int overload(Functions&&...) {
        return 0;
    }

    template<typename T>
    using optional = std::optional<T>;

    class error : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        error() : std::runtime_error("Lua subsystem disabled") {}
    };
} // namespace sol
#endif
