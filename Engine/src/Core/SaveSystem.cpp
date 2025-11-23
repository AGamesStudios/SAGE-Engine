#include "SAGE/Core/SaveSystem.h"
#include "SAGE/Log.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cctype>

namespace SAGE {

// SaveData implementation
void SaveData::SetInt(const std::string& key, int value) {
    m_Data[key] = value;
}

void SaveData::SetFloat(const std::string& key, float value) {
    m_Data[key] = value;
}

void SaveData::SetString(const std::string& key, const std::string& value) {
    m_Data[key] = value;
}

void SaveData::SetBool(const std::string& key, bool value) {
    m_Data[key] = value;
}

void SaveData::SetVector2(const std::string& key, const Vector2& value) {
    m_Data[key] = value;
}

int SaveData::GetInt(const std::string& key, int defaultValue) const {
    auto it = m_Data.find(key);
    if (it != m_Data.end()) {
        try {
            return std::any_cast<int>(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

float SaveData::GetFloat(const std::string& key, float defaultValue) const {
    auto it = m_Data.find(key);
    if (it != m_Data.end()) {
        try {
            return std::any_cast<float>(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

std::string SaveData::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_Data.find(key);
    if (it != m_Data.end()) {
        try {
            return std::any_cast<std::string>(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

bool SaveData::GetBool(const std::string& key, bool defaultValue) const {
    auto it = m_Data.find(key);
    if (it != m_Data.end()) {
        try {
            return std::any_cast<bool>(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

Vector2 SaveData::GetVector2(const std::string& key, const Vector2& defaultValue) const {
    auto it = m_Data.find(key);
    if (it != m_Data.end()) {
        try {
            return std::any_cast<Vector2>(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

bool SaveData::HasKey(const std::string& key) const {
    return m_Data.find(key) != m_Data.end();
}

void SaveData::Remove(const std::string& key) {
    m_Data.erase(key);
}

void SaveData::Clear() {
    m_Data.clear();
}

std::vector<std::string> SaveData::GetKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_Data.size());
    for (const auto& pair : m_Data) {
        keys.push_back(pair.first);
    }
    return keys;
}

std::string SaveData::ToJson() const {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& [key, value] : m_Data) {
        if (!first) oss << ",";
        first = false;
        
        oss << "\"" << key << "\":";
        
        // Try to determine type and serialize
        if (value.type() == typeid(int)) {
            oss << std::any_cast<int>(value);
        } else if (value.type() == typeid(float)) {
            oss << std::any_cast<float>(value);
        } else if (value.type() == typeid(bool)) {
            oss << (std::any_cast<bool>(value) ? "true" : "false");
        } else if (value.type() == typeid(std::string)) {
            oss << "\"" << std::any_cast<std::string>(value) << "\"";
        } else if (value.type() == typeid(Vector2)) {
            auto v = std::any_cast<Vector2>(value);
            oss << "{\"x\":" << v.x << ",\"y\":" << v.y << "}";
        }
    }
    
    oss << "}";
    return oss.str();
}

bool SaveData::FromJson(const std::string& json) {
    Clear();
    size_t pos = 0;

    auto skipWhitespace = [&]() {
        while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
            ++pos;
        }
    };

    auto parseString = [&]() -> std::string {
        if (pos >= json.size() || json[pos] != '"') {
            return {};
        }
        ++pos; // skip "
        std::string out;
        while (pos < json.size() && json[pos] != '"') {
            out.push_back(json[pos++]);
        }
        if (pos < json.size() && json[pos] == '"') {
            ++pos;
        }
        return out;
    };

    auto parseNumber = [&](const std::string& token, bool& isFloat) -> double {
        isFloat = token.find_first_of(".eE") != std::string::npos;
        try {
            return std::stod(token);
        } catch (...) {
            return 0.0;
        }
    };

    skipWhitespace();
    if (pos >= json.size() || json[pos] != '{') {
        SAGE_ERROR("SaveData::FromJson: ожидался объект JSON");
        return false;
    }
    ++pos;

    skipWhitespace();
    while (pos < json.size() && json[pos] != '}') {
        skipWhitespace();
        std::string key = parseString();
        if (key.empty()) {
            SAGE_ERROR("SaveData::FromJson: не удалось прочитать ключ");
            return false;
        }

        skipWhitespace();
        if (pos >= json.size() || json[pos] != ':') {
            SAGE_ERROR("SaveData::FromJson: ожидался символ ':' после ключа {}", key);
            return false;
        }
        ++pos;
        skipWhitespace();

        if (pos < json.size() && json[pos] == '{') {
            // Vector2 object: {"x":..,"y":..}
            ++pos;
            skipWhitespace();
            std::string xKey = parseString();
            skipWhitespace();
            if (pos >= json.size() || json[pos] != ':') {
                SAGE_ERROR("SaveData::FromJson: ожидался ':' после 'x' в Vector2");
                return false;
            }
            ++pos;
            skipWhitespace();
            size_t start = pos;
            while (pos < json.size() && json[pos] != ',' && json[pos] != '}') ++pos;
            std::string xToken = json.substr(start, pos - start);
            bool xIsFloat = false;
            double xVal = parseNumber(xToken, xIsFloat);

            skipWhitespace();
            if (pos >= json.size() || json[pos] != ',') {
                SAGE_ERROR("SaveData::FromJson: ожидалась ',' в Vector2");
                return false;
            }
            ++pos;
            skipWhitespace();
            std::string yKey = parseString();
            skipWhitespace();
            if (pos >= json.size() || json[pos] != ':') {
                SAGE_ERROR("SaveData::FromJson: ожидался ':' после 'y' в Vector2");
                return false;
            }
            ++pos;
            skipWhitespace();
            start = pos;
            while (pos < json.size() && json[pos] != '}') ++pos;
            std::string yToken = json.substr(start, pos - start);
            bool yIsFloat = false;
            double yVal = parseNumber(yToken, yIsFloat);

            if (pos < json.size() && json[pos] == '}') {
                ++pos;
            }
            SetVector2(key, Vector2(static_cast<float>(xVal), static_cast<float>(yVal)));
        } else if (pos < json.size() && json[pos] == '"') {
            std::string value = parseString();
            SetString(key, value);
        } else if (json.compare(pos, 4, "true") == 0 || json.compare(pos, 5, "false") == 0) {
            bool v = json.compare(pos, 4, "true") == 0;
            pos += v ? 4 : 5;
            SetBool(key, v);
        } else {
            // Number
            size_t start = pos;
            while (pos < json.size() && json[pos] != ',' && json[pos] != '}') {
                ++pos;
            }
            std::string token = json.substr(start, pos - start);
            bool isFloat = false;
            double numeric = parseNumber(token, isFloat);
            if (isFloat) {
                SetFloat(key, static_cast<float>(numeric));
            } else {
                SetInt(key, static_cast<int>(numeric));
            }
        }

        skipWhitespace();
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
    }

    if (pos < json.size() && json[pos] == '}') {
        ++pos;
    }

    return true;
}

// SaveSystem implementation
SaveSystem& SaveSystem::Get() {
    static SaveSystem instance;
    return instance;
}

SaveSystem::SaveSystem() {
    // Default save directory
    m_SaveDirectory = "./saves";
}

bool SaveSystem::Save(const std::string& slotName, const SaveData& data) {
    try {
        // Ensure save directory exists
        std::filesystem::create_directories(m_SaveDirectory);
        
        std::string path = GetSavePath(slotName);
        std::ofstream file(path);
        
        if (!file.is_open()) {
            SAGE_ERROR("SaveSystem: Failed to open file for writing: {}", path);
            return false;
        }
        
        file << data.ToJson();
        file.close();
        
        SAGE_INFO("SaveSystem: Saved to slot '{}'", slotName);
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("SaveSystem: Exception while saving: {}", e.what());
        return false;
    }
}

bool SaveSystem::Load(const std::string& slotName, SaveData& outData) {
    try {
        std::string path = GetSavePath(slotName);
        
        if (!std::filesystem::exists(path)) {
            SAGE_WARN("SaveSystem: Save file does not exist: {}", path);
            return false;
        }
        
        std::ifstream file(path);
        if (!file.is_open()) {
            SAGE_ERROR("SaveSystem: Failed to open file for reading: {}", path);
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        bool success = outData.FromJson(buffer.str());
        if (success) {
            SAGE_INFO("SaveSystem: Loaded from slot '{}'", slotName);
        }
        return success;
    } catch (const std::exception& e) {
        SAGE_ERROR("SaveSystem: Exception while loading: {}", e.what());
        return false;
    }
}

bool SaveSystem::SaveExists(const std::string& slotName) const {
    return std::filesystem::exists(GetSavePath(slotName));
}

bool SaveSystem::DeleteSave(const std::string& slotName) {
    try {
        std::string path = GetSavePath(slotName);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
            SAGE_INFO("SaveSystem: Deleted save slot '{}'", slotName);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        SAGE_ERROR("SaveSystem: Exception while deleting save: {}", e.what());
        return false;
    }
}

std::vector<std::string> SaveSystem::GetSaveSlots() const {
    std::vector<std::string> slots;
    
    try {
        if (!std::filesystem::exists(m_SaveDirectory)) {
            return slots;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(m_SaveDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sav") {
                slots.push_back(entry.path().stem().string());
            }
        }
    } catch (const std::exception& e) {
        SAGE_ERROR("SaveSystem: Exception while getting save slots: {}", e.what());
    }
    
    return slots;
}

void SaveSystem::SetSaveDirectory(const std::string& directory) {
    m_SaveDirectory = directory;
}

void SaveSystem::EnableAutoSave(bool enable, float intervalSeconds) {
    m_AutoSaveEnabled = enable;
    m_AutoSaveInterval = intervalSeconds;
    m_AutoSaveTimer = 0.0f;
}

void SaveSystem::TriggerAutoSave(const SaveData& data) {
    if (m_AutoSaveEnabled) {
        Save("autosave", data);
    }
}

bool SaveSystem::QuickSave(const SaveData& data) {
    return Save("quicksave", data);
}

bool SaveSystem::QuickLoad(SaveData& outData) {
    return Load("quicksave", outData);
}

std::string SaveSystem::GetSavePath(const std::string& slotName) const {
    return m_SaveDirectory + "/" + slotName + ".sav";
}

} // namespace SAGE
