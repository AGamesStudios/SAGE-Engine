#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <memory>
#include "SAGE/Math/Vector2.h"

namespace SAGE {

// Save data container
class SaveData {
public:
    SaveData() = default;
    
    // Set values
    void SetInt(const std::string& key, int value);
    void SetFloat(const std::string& key, float value);
    void SetString(const std::string& key, const std::string& value);
    void SetBool(const std::string& key, bool value);
    void SetVector2(const std::string& key, const Vector2& value);
    
    // Get values with defaults
    int GetInt(const std::string& key, int defaultValue = 0) const;
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    bool GetBool(const std::string& key, bool defaultValue = false) const;
    Vector2 GetVector2(const std::string& key, const Vector2& defaultValue = {0.0f, 0.0f}) const;
    
    // Check if key exists
    bool HasKey(const std::string& key) const;
    
    // Remove key
    void Remove(const std::string& key);
    
    // Clear all data
    void Clear();
    
    // Get all keys
    std::vector<std::string> GetKeys() const;
    
    // Serialize to JSON string
    std::string ToJson() const;
    
    // Deserialize from JSON string
    bool FromJson(const std::string& json);

private:
    std::unordered_map<std::string, std::any> m_Data;
};

// Save/Load system
class SaveSystem {
public:
    static SaveSystem& Get();
    
    // Save data to file
    bool Save(const std::string& slotName, const SaveData& data);
    
    // Load data from file
    bool Load(const std::string& slotName, SaveData& outData);
    
    // Check if save exists
    bool SaveExists(const std::string& slotName) const;
    
    // Delete save
    bool DeleteSave(const std::string& slotName);
    
    // Get all save slots
    std::vector<std::string> GetSaveSlots() const;
    
    // Set save directory
    void SetSaveDirectory(const std::string& directory);
    std::string GetSaveDirectory() const { return m_SaveDirectory; }
    
    // Auto-save
    void EnableAutoSave(bool enable, float intervalSeconds = 300.0f);
    bool IsAutoSaveEnabled() const { return m_AutoSaveEnabled; }
    void TriggerAutoSave(const SaveData& data);
    
    // Quick save/load
    bool QuickSave(const SaveData& data);
    bool QuickLoad(SaveData& outData);

private:
    SaveSystem();
    
    std::string GetSavePath(const std::string& slotName) const;
    
    std::string m_SaveDirectory;
    bool m_AutoSaveEnabled = false;
    float m_AutoSaveInterval = 300.0f;
    float m_AutoSaveTimer = 0.0f;
};

} // namespace SAGE
