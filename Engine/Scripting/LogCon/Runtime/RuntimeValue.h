#pragma once

#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <memory>

namespace SAGE::Scripting::LogCon::Runtime {

class RuntimeValue;
using ArrayStorage = std::shared_ptr<std::vector<RuntimeValue>>;

class RuntimeValue {
public:
    using Storage = std::variant<std::monostate, double, bool, std::string, ArrayStorage>;

    RuntimeValue() = default;
    RuntimeValue(double value) : m_Value(value) {}
    RuntimeValue(bool value) : m_Value(value) {}
    RuntimeValue(std::string value) : m_Value(std::move(value)) {}
    RuntimeValue(const char* value) : m_Value(std::string(value)) {}
    RuntimeValue(ArrayStorage value) : m_Value(std::move(value)) {}

    [[nodiscard]] bool IsNull() const { return std::holds_alternative<std::monostate>(m_Value); }
    [[nodiscard]] bool IsNumber() const { return std::holds_alternative<double>(m_Value); }
    [[nodiscard]] bool IsBoolean() const { return std::holds_alternative<bool>(m_Value); }
    [[nodiscard]] bool IsString() const { return std::holds_alternative<std::string>(m_Value); }
    [[nodiscard]] bool IsArray() const { return std::holds_alternative<ArrayStorage>(m_Value); }

    [[nodiscard]] double AsNumber(double defaultValue = 0.0) const;
    [[nodiscard]] bool AsBool() const;
    [[nodiscard]] std::string AsString() const; // Реализация в .cpp
    [[nodiscard]] ArrayStorage AsArray() const;

    void SetNumber(double value) { m_Value = value; }
    void SetBoolean(bool value) { m_Value = value; }
    void SetString(std::string value) { m_Value = std::move(value); }
    void SetArray(ArrayStorage value) { m_Value = std::move(value); }

    [[nodiscard]] const Storage& GetRaw() const { return m_Value; }

private:
    Storage m_Value;
};

inline double RuntimeValue::AsNumber(double defaultValue) const {
    if (IsNumber()) {
        return std::get<double>(m_Value);
    }
    if (IsBoolean()) {
        return std::get<bool>(m_Value) ? 1.0 : 0.0;
    }
    if (IsString()) {
        try {
            return std::stod(std::get<std::string>(m_Value));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

inline bool RuntimeValue::AsBool() const {
    if (IsBoolean()) {
        return std::get<bool>(m_Value);
    }
    if (IsNumber()) {
        return std::get<double>(m_Value) != 0.0;
    }
    if (IsString()) {
        const auto& value = std::get<std::string>(m_Value);
        return !value.empty() && value != "0" && value != "false" && value != "нет";
    }
    return false;
}

inline ArrayStorage RuntimeValue::AsArray() const {
    if (IsArray()) {
        return std::get<ArrayStorage>(m_Value);
    }
    return nullptr;
}

} // namespace SAGE::Scripting::LogCon::Runtime
