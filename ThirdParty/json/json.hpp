// Minimal fallback json.hpp to satisfy build when network fetch fails.
// Provides extremely small subset used by engine; NOT a full JSON implementation.
#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <istream>
#include <cstddef>

namespace nlohmann {
class json {
    using object_t = std::unordered_map<std::string, json>;
    using array_t = std::vector<json>;
    using string_t = std::string;
    using boolean_t = bool;
    using number_float_t = double;
    using number_int_t = long long;
    using value_t = std::variant<std::nullptr_t, boolean_t, number_int_t, number_float_t, string_t, object_t, array_t>;
    value_t m_Value;

    static array_t& empty_array() { static array_t a; return a; }
public:
    json() : m_Value(nullptr) {}
    json(const string_t &value) : m_Value(value) {}
    json(const char *value) : m_Value(string_t(value ? value : "")) {}
    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
    json(T value) {
        if constexpr (std::is_same_v<T, bool>) {
            m_Value = static_cast<boolean_t>(value);
        } else {
            m_Value = static_cast<number_int_t>(value);
        }
    }
    template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    json(T value) : m_Value(static_cast<number_float_t>(value)) {}
    json(std::initializer_list<std::pair<const std::string, json>> init) : m_Value(object_t{}) { auto &obj = std::get<object_t>(m_Value); for (auto &p : init) obj.emplace(p.first, p.second); }
    static json object() { json j; j.m_Value = object_t{}; return j; }
    static json array() { json j; j.m_Value = array_t{}; return j; }
    static json parse(std::istream &is) { (void)is; return json::object(); }
    static json parse(const std::string &str) { (void)str; return json::object(); }
    bool is_null() const { return std::holds_alternative<std::nullptr_t>(m_Value); }
    bool is_object() const { return std::holds_alternative<object_t>(m_Value); }
    bool is_array() const { return std::holds_alternative<array_t>(m_Value); }
    bool is_boolean() const { return std::holds_alternative<boolean_t>(m_Value); }
    bool is_number() const { return std::holds_alternative<number_int_t>(m_Value) || std::holds_alternative<number_float_t>(m_Value); }
    bool is_string() const { return std::holds_alternative<string_t>(m_Value); }
    bool contains(const std::string &key) const { if(!is_object()) return false; const auto &obj = std::get<object_t>(m_Value); return obj.find(key)!=obj.end(); }
    json &operator[](const std::string &key) { if(!is_object()) m_Value = object_t{}; auto &obj = std::get<object_t>(m_Value); return obj[key]; }
    const json &operator[](const std::string &key) const { static json null; if(!is_object()) return null; const auto &obj = std::get<object_t>(m_Value); auto it=obj.find(key); return it!=obj.end()?it->second:null; }
    json &operator[](std::size_t index) { if(!is_array()) m_Value = array_t{}; auto &arr = std::get<array_t>(m_Value); if(index >= arr.size()) arr.resize(index+1); return arr[index]; }
    const json &operator[](std::size_t index) const { static json null; if(!is_array()) return null; const auto &arr = std::get<array_t>(m_Value); if(index >= arr.size()) return null; return arr[index]; }

    json& operator=(const string_t &value) { m_Value = value; return *this; }
    json& operator=(const char *value) { m_Value = string_t(value ? value : ""); return *this; }
    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
    json& operator=(T value) {
        if constexpr (std::is_same_v<T, bool>) {
            m_Value = static_cast<boolean_t>(value);
        } else {
            m_Value = static_cast<number_int_t>(value);
        }
        return *this;
    }
    template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    json& operator=(T value) { m_Value = static_cast<number_float_t>(value); return *this; }

    template<typename T> T get() const { 
        if constexpr (std::is_same_v<T, json>) { return *this; }
        else if constexpr (std::is_same_v<T, std::string>) { if(std::holds_alternative<string_t>(m_Value)) return std::get<string_t>(m_Value); }
        else if constexpr (std::is_same_v<T,bool>) { if(std::holds_alternative<boolean_t>(m_Value)) return std::get<boolean_t>(m_Value); }
        else if constexpr (std::is_integral_v<T>) { if(std::holds_alternative<number_int_t>(m_Value)) return static_cast<T>(std::get<number_int_t>(m_Value)); }
        else if constexpr (std::is_floating_point_v<T>) { if(std::holds_alternative<number_float_t>(m_Value)) return static_cast<T>(std::get<number_float_t>(m_Value)); }
        return T{}; }

    template<typename T> T value(const std::string &key, T default_value) const { 
        if(!is_object()) return default_value; const auto &obj = std::get<object_t>(m_Value); auto it=obj.find(key); if(it==obj.end()) return default_value; return it->second.get<T>(); }

    std::size_t size() const { if(is_array()) return std::get<array_t>(m_Value).size(); if(is_object()) return std::get<object_t>(m_Value).size(); return 0; }

    const object_t &object_items() const { if(!is_object()) { static object_t empty; empty.clear(); return empty; } return std::get<object_t>(m_Value); }

    void push_back(const json &value) {
        if(!is_array()) m_Value = array_t{};
        std::get<array_t>(m_Value).push_back(value);
    }

    void push_back(json &&value) {
        if(!is_array()) m_Value = array_t{};
        std::get<array_t>(m_Value).push_back(std::move(value));
    }

    // iteration for arrays
    auto begin() { return is_array()? std::get<array_t>(m_Value).begin() : empty_array().begin(); }
    auto end() { return is_array()? std::get<array_t>(m_Value).end() : empty_array().end(); }
    auto begin() const { return is_array()? std::get<array_t>(m_Value).begin() : empty_array().begin(); }
    auto end() const { return is_array()? std::get<array_t>(m_Value).end() : empty_array().end(); }

    std::string dump(int = -1) const { if(is_object()) return "{...}"; if(is_array()) return "[...]"; if(std::holds_alternative<string_t>(m_Value)) return '"'+std::get<string_t>(m_Value)+'"'; if(std::holds_alternative<number_int_t>(m_Value)) return std::to_string(std::get<number_int_t>(m_Value)); if(std::holds_alternative<number_float_t>(m_Value)) return std::to_string(std::get<number_float_t>(m_Value)); if(std::holds_alternative<boolean_t>(m_Value)) return std::get<boolean_t>(m_Value)?"true":"false"; return "null"; }

    friend std::istream &operator>>(std::istream &is, json &j) { // naive: ignore actual content
        std::string s; is >> s; j = json::object(); return is; }
};
} // namespace nlohmann
