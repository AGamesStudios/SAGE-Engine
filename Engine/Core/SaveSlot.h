#pragma once

#include <string>
#include <ctime>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Метаданные одного сохранения
 */
struct SaveSlot {
    int slotIndex = 0;
    std::string slotName;
    
    // Metadata
    std::time_t saveTime = 0;
    std::string playerName;
    int saveVersion = 1;
    uint32_t checksum = 0; // CRC32 checksum for integrity verification
    
    // Game-specific data
    std::string currentLevel;
    int playerLevel = 1;
    float playtimeSeconds = 0.0f;
    
    // Screenshot thumbnail path (optional)
    std::string thumbnailPath;
    
    // Custom metadata (game-specific)
    json customData;
    
    bool isEmpty = true;

    SaveSlot() = default;
    
    explicit SaveSlot(int index) : slotIndex(index) {
        slotName = "Save " + std::to_string(index);
    }

    // Serialize metadata only
    json ToJson() const {
        json j;
        j["slotIndex"] = slotIndex;
        j["slotName"] = slotName;
        j["saveTime"] = saveTime;
        j["playerName"] = playerName;
        j["saveVersion"] = saveVersion;
        j["checksum"] = checksum;
        j["currentLevel"] = currentLevel;
        j["playerLevel"] = playerLevel;
        j["playtimeSeconds"] = playtimeSeconds;
        j["thumbnailPath"] = thumbnailPath;
        j["customData"] = customData;
        j["isEmpty"] = isEmpty;
        return j;
    }

    void FromJson(const json& j) {
        slotIndex = j.value("slotIndex", 0);
        slotName = j.value("slotName", "");
        saveTime = j.value("saveTime", 0);
        playerName = j.value("playerName", "");
        saveVersion = j.value("saveVersion", 1);
        checksum = j.value("checksum", 0u);
        currentLevel = j.value("currentLevel", "");
        playerLevel = j.value("playerLevel", 1);
        playtimeSeconds = j.value("playtimeSeconds", 0.0f);
        thumbnailPath = j.value("thumbnailPath", "");
        customData = j.value("customData", json::object());
        isEmpty = j.value("isEmpty", true);
    }

    std::string GetFormattedTime() const {
        if (saveTime == 0) return "Never";
        
        char buffer[80];
        #ifdef _MSC_VER
        std::tm timeinfo;
        localtime_s(&timeinfo, &saveTime);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        #else
        std::tm* timeinfo = std::localtime(&saveTime);
        if (timeinfo) {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        } else {
            return "Invalid Time";
        }
        #endif
        return std::string(buffer);
    }
};

} // namespace SAGE
