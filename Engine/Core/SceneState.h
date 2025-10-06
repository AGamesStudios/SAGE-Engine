#pragma once

#include <any>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace SAGE {

    struct SceneState {
        std::unordered_map<std::string, std::any> Values;

        [[nodiscard]] bool Empty() const { return Values.empty(); }
        void Clear() { Values.clear(); }

        template<typename T>
        void Set(const std::string& key, T&& value) {
            Values[key] = std::forward<T>(value);
        }

        template<typename T>
        std::optional<T> Get(const std::string& key) const {
            auto it = Values.find(key);
            if (it == Values.end()) {
                return std::nullopt;
            }

            try {
                return std::any_cast<T>(it->second);
            }
            catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
    };

} // namespace SAGE
