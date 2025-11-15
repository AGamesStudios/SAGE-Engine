#pragma once

#include "Core/Logger.h"
#include <json/json.hpp>
#include <fstream>
#include <string>
#include <unordered_map>

namespace SAGE {

/// @brief Configuration system for engine and game settings
/// Supports JSON-based configuration files with type-safe accessors
/// Automatically saves changes and provides default values
class Config {
public:
    /// @brief Create config system
    /// @param filename Path to config file (default: "settings.json")
    explicit Config(const std::string& filename = "settings.json")
        : m_Filename(filename)
    {
        Load();
    }

    ~Config() {
        Save();
    }

    // Non-copyable, movable
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) noexcept = default;
    Config& operator=(Config&&) noexcept = default;

    /// @brief Load configuration from file
    /// Creates default config if file doesn't exist
    void Load() {
        std::ifstream file(m_Filename);
        
        if (!file.is_open()) {
            SAGE_WARNING("Config file not found: {}, creating default", m_Filename);
            CreateDefaultConfig();
            Save();
            return;
        }

        file >> m_Data;
        SAGE_INFO("Configuration loaded from: {}", m_Filename);
    }

    /// @brief Save configuration to file
    void Save() {
        std::ofstream file(m_Filename);
        
        if (!file.is_open()) {
            SAGE_ERROR("Failed to save config file: {}", m_Filename);
            return;
        }

        file << m_Data.dump(4); // Pretty-print with 4 spaces
        SAGE_INFO("Configuration saved to: {}", m_Filename);
    }

    /// @brief Get value from config
    /// @tparam T Value type (int, float, bool, std::string, etc.)
    /// @param key JSON path (e.g., "graphics.width" or "audio.volume")
    /// @param defaultValue Value to return if key doesn't exist
    /// @return Config value or default
    template<typename T>
    T Get(const std::string& key, T defaultValue = T{}) {
        try {
            auto value = GetValueByPath(key);
            if (value.is_null()) {
                Set(key, defaultValue); // Auto-create missing keys
                return defaultValue;
            }
            return value.get<T>();
        } catch (...) {
            SAGE_WARNING("Failed to get config key '{}', using default", key);
            return defaultValue;
        }
    }

    /// @brief Set value in config
    /// @tparam T Value type
    /// @param key JSON path (e.g., "graphics.fullscreen")
    /// @param value Value to set
    template<typename T>
    void Set(const std::string& key, T value) {
        SetValueByPath(key, value);
        m_Modified = true;
    }

    /// @brief Check if key exists
    bool Has(const std::string& key) const {
        return !GetValueByPath(key).is_null();
    }

    /// @brief Get raw JSON data (advanced usage)
    nlohmann::json& GetRawData() { return m_Data; }
    const nlohmann::json& GetRawData() const { return m_Data; }

    /// @brief Reload from file (discards unsaved changes)
    void Reload() {
        m_Modified = false;
        Load();
    }

    /// @brief Check if config has unsaved changes
    bool IsModified() const { return m_Modified; }

private:
    void CreateDefaultConfig() {
        m_Data = nlohmann::json::object();

        auto& engine = m_Data["engine"];
        engine["version"] = "0.1.0-alpha";
        engine["logLevel"] = "info";
        engine["logToFile"] = true;

        auto& graphics = m_Data["graphics"];
        graphics["width"] = 1920;
        graphics["height"] = 1080;
        graphics["fullscreen"] = false;
        graphics["vsync"] = true;
        graphics["msaa"] = 4;
        graphics["maxFPS"] = 144;

        auto& audio = m_Data["audio"];
        audio["masterVolume"] = 0.8f;
        audio["musicVolume"] = 0.7f;
        audio["sfxVolume"] = 1.0f;
        audio["muted"] = false;

        auto& input = m_Data["input"];
        input["mouseSensitivity"] = 1.0f;
        input["invertY"] = false;
    }

    nlohmann::json GetValueByPath(const std::string& path) const {
        std::vector<std::string> keys = SplitPath(path);
        nlohmann::json current = m_Data;

        for (const auto& key : keys) {
            if (!current.contains(key)) {
                return nlohmann::json(); // null
            }
            current = current[key];
        }

        return current;
    }

    void SetValueByPath(const std::string& path, const nlohmann::json& value) {
        std::vector<std::string> keys = SplitPath(path);
        nlohmann::json* current = &m_Data;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (!current->contains(keys[i])) {
                (*current)[keys[i]] = nlohmann::json::object();
            }
            current = &(*current)[keys[i]];
        }

        (*current)[keys.back()] = value;
    }

    static std::vector<std::string> SplitPath(const std::string& path) {
        std::vector<std::string> result;
        std::string current;

        for (char c : path) {
            if (c == '.') {
                if (!current.empty()) {
                    result.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            result.push_back(current);
        }

        return result;
    }

    std::string m_Filename;
    nlohmann::json m_Data;
    bool m_Modified = false;
};

} // namespace SAGE
