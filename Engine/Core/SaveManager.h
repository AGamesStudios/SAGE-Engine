#pragma once

#include "SaveSlot.h"
#include "SceneSerializer.h"
#include "Logger.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
// Optional zlib usage (CRC32). If zlib is unavailable, fallback to dummy checksum.
#if __has_include(<zlib.h>)
#include <zlib.h>
#define SAGE_HAVE_ZLIB 1
#else
#define SAGE_HAVE_ZLIB 0
#endif

namespace SAGE {

// Save file version - increment when format changes
constexpr int CURRENT_SAVE_VERSION = 1;

/**
 * @brief Save Manager с поддержкой слотов, автосохранения, версионирования
 * 
 * Особенности:
 * - Множественные слоты сохранений
 * - Автосохранение с настраиваемым интервалом
 * - Metadata для каждого слота (время, имя игрока, уровень)
 * - Версионирование сохранений
 * - Облачное сохранение (interface для интеграции)
 * 
 * Структура директории:
 * saves/
 *   slot_0.json      - Полные данные слота 0
 *   slot_1.json      - Полные данные слота 1
 *   autosave.json    - Автосохранение
 *   metadata.json    - Метаданные всех слотов
 */
class SaveManager {
public:
    SaveManager() {
        SetSaveDirectory("saves");
    }

    /**
     * @brief Установить директорию сохранений
     */
    void SetSaveDirectory(const std::string& dir) {
        m_SaveDirectory = dir;
        
        // Create directory if not exists
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
        
        LoadMetadata();
    }

    /**
     * @brief Сохранить игру в слот с CRC проверкой
     */
    bool SaveToSlot(int slotIndex, const Scene* scene, const SaveSlot& metadata) {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) {
            SAGE_ERROR("SaveManager: Invalid slot index {}", slotIndex);
            return false;
        }

        std::string filePath = GetSlotFilePath(slotIndex);
        
        // Save scene data
        if (!SceneSerializer::SaveToFile(scene, filePath)) {
            return false;
        }

    // Calculate CRC32 checksum (or fallback if zlib missing)
    uint32_t checksum = CalculateFileCRC32(filePath);

        // Update metadata
        SaveSlot slot = metadata;
        slot.slotIndex = slotIndex;
        slot.saveTime = std::time(nullptr);
        slot.isEmpty = false;
        slot.saveVersion = CURRENT_SAVE_VERSION;
        slot.checksum = checksum;
        
        m_Slots[slotIndex] = slot;
        SaveMetadata();

        SAGE_INFO("SaveManager: Saved to slot {} (CRC32: {:08X})", slotIndex, checksum);
        return true;
    }

    /**
     * @brief Загрузить игру из слота с проверкой целостности
     */
    bool LoadFromSlot(int slotIndex, Scene* scene) {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) {
            SAGE_ERROR("SaveManager: Invalid slot index {}", slotIndex);
            return false;
        }

        if (m_Slots[slotIndex].isEmpty) {
            SAGE_WARN("SaveManager: Slot {} is empty", slotIndex);
            return false;
        }

        std::string filePath = GetSlotFilePath(slotIndex);
        
        // Verify file integrity with CRC32
        if (!VerifyFileIntegrity(filePath, m_Slots[slotIndex].checksum)) {
            SAGE_ERROR("SaveManager: Save file corrupted (slot {})", slotIndex);
            return false;
        }

        // Check version and migrate if needed
        if (m_Slots[slotIndex].saveVersion < CURRENT_SAVE_VERSION) {
            SAGE_INFO("SaveManager: Migrating save from v{} to v{}", 
                     m_Slots[slotIndex].saveVersion, CURRENT_SAVE_VERSION);
            if (!MigrateSaveFile(filePath, m_Slots[slotIndex].saveVersion)) {
                SAGE_ERROR("SaveManager: Failed to migrate save file");
                return false;
            }
        }
        
        if (!SceneSerializer::LoadFromFile(scene, filePath)) {
            return false;
        }

        m_CurrentSlot = slotIndex;
        SAGE_INFO("SaveManager: Loaded from slot {}", slotIndex);
        return true;
    }

    /**
     * @brief Удалить сохранение из слота
     */
    bool DeleteSlot(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) {
            return false;
        }

        std::string filePath = GetSlotFilePath(slotIndex);
        
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
        }

        m_Slots[slotIndex] = SaveSlot(slotIndex);
        m_Slots[slotIndex].isEmpty = true;
        SaveMetadata();

        SAGE_INFO("SaveManager: Deleted slot {}", slotIndex);
        return true;
    }

    /**
     * @brief Автосохранение
     */
    bool AutoSave(const Scene* scene, const SaveSlot& metadata) {
        std::string filePath = m_SaveDirectory + "/autosave.json";
        
        if (!SceneSerializer::SaveToFile(scene, filePath)) {
            return false;
        }

        // Update autosave metadata
        m_AutoSaveSlot = metadata;
        m_AutoSaveSlot.slotIndex = -1; // Special index for autosave
        m_AutoSaveSlot.slotName = "AutoSave";
        m_AutoSaveSlot.saveTime = std::time(nullptr);
        m_AutoSaveSlot.isEmpty = false;
        
        SaveMetadata();

        m_LastAutoSaveTime = std::time(nullptr);
        SAGE_INFO("SaveManager: AutoSave completed");
        return true;
    }

    /**
     * @brief Загрузить автосохранение
     */
    bool LoadAutoSave(Scene* scene) {
        std::string filePath = m_SaveDirectory + "/autosave.json";
        
        if (!std::filesystem::exists(filePath)) {
            SAGE_WARN("SaveManager: No autosave found");
            return false;
        }

        if (!SceneSerializer::LoadFromFile(scene, filePath)) {
            return false;
        }

        SAGE_INFO("SaveManager: Loaded autosave");
        return true;
    }

    /**
     * @brief Update автосохранения (вызывать каждый кадр)
     */
    void UpdateAutoSave(float deltaTime) {
        if (!m_AutoSaveEnabled) return;

        m_AutoSaveTimer += deltaTime;
        
        if (m_AutoSaveTimer >= m_AutoSaveInterval) {
            m_AutoSaveTimer = 0.0f;
            m_NeedsAutoSave = true;
        }
    }

    /**
     * @brief Проверить нужно ли автосохранение
     */
    bool NeedsAutoSave() const { return m_NeedsAutoSave; }
    void ClearAutoSaveFlag() { m_NeedsAutoSave = false; }

    /**
     * @brief Настройки автосохранения
     */
    void EnableAutoSave(bool enabled) { m_AutoSaveEnabled = enabled; }
    void SetAutoSaveInterval(float seconds) { m_AutoSaveInterval = seconds; }
    float GetAutoSaveInterval() const { return m_AutoSaveInterval; }

    /**
     * @brief Получить метаданные слота
     */
    const SaveSlot& GetSlotMetadata(int slotIndex) const {
        static SaveSlot empty;
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) return empty;
        return m_Slots[slotIndex];
    }

    const SaveSlot& GetAutoSaveMetadata() const { return m_AutoSaveSlot; }

    /**
     * @brief Получить все слоты
     */
    const std::vector<SaveSlot>& GetAllSlots() const { return m_Slots; }

    /**
     * @brief Экспорт сохранения (для облачного хранения)
     */
    nlohmann::json ExportSaveData(int slotIndex) const {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) return nlohmann::json();
        
        std::string filePath = GetSlotFilePath(slotIndex);
        if (!std::filesystem::exists(filePath)) return nlohmann::json();

        std::ifstream file(filePath);
        nlohmann::json data;
        file >> data;
        return data;
    }

    /**
     * @brief Импорт сохранения (из облака)
     */
    bool ImportSaveData(int slotIndex, const nlohmann::json& data, const SaveSlot& metadata) {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) return false;

        std::string filePath = GetSlotFilePath(slotIndex);
        
        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        
        file << data.dump(4);
        file.close();

        m_Slots[slotIndex] = metadata;
        m_Slots[slotIndex].slotIndex = slotIndex;
        SaveMetadata();

        return true;
    }

    /**
     * @brief Проверить существует ли сохранение
     */
    bool HasSave(int slotIndex) const {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) return false;
        return !m_Slots[slotIndex].isEmpty;
    }

    int GetCurrentSlot() const { return m_CurrentSlot; }
    int GetMaxSlots() const { return m_MaxSlots; }

    std::string ResolveSlotFilePath(int slotIndex) const {
        if (slotIndex < 0 || slotIndex >= m_MaxSlots) {
            return {};
        }
        return GetSlotFilePath(slotIndex);
    }

private:
    /**
     * @brief Calculate CRC32 checksum for file
     */
    uint32_t CalculateFileCRC32(const std::string& filepath) const {
#if SAGE_HAVE_ZLIB
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) return 0;
        uLong crc = crc32(0L, Z_NULL, 0);
        const size_t BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
            crc = crc32(crc, reinterpret_cast<const Bytef*>(buffer), static_cast<uInt>(file.gcount()));
        }
        return static_cast<uint32_t>(crc);
#else
        // Fallback: simple accumulating hash (NOT cryptographically secure)
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) return 0;
        uint32_t hash = 2166136261u; // FNV offset basis
        char c;
        while (file.get(c)) {
            hash ^= static_cast<unsigned char>(c);
            hash *= 16777619u; // FNV prime
        }
        return hash;
#endif
    }

    /**
     * @brief Verify file integrity using CRC32
     */
    bool VerifyFileIntegrity(const std::string& filepath, uint32_t expectedCRC) const {
        if (expectedCRC == 0) {
            // No checksum stored (legacy save), skip verification
            SAGE_WARN("SaveManager: No checksum for verification (legacy save)");
            return true;
        }

        uint32_t actualCRC = CalculateFileCRC32(filepath);
        if (actualCRC != expectedCRC) {
            SAGE_ERROR("SaveManager: CRC mismatch! Expected {:08X}, got {:08X}", 
                      expectedCRC, actualCRC);
            return false;
        }

        return true;
    }

    /**
     * @brief Migrate save file from old version to current
     */
    bool MigrateSaveFile(const std::string& filepath, int fromVersion) {
        try {
            std::ifstream file(filepath);
            nlohmann::json data;
            file >> data;
            file.close();

            // Apply migrations sequentially
            for (int v = fromVersion; v < CURRENT_SAVE_VERSION; ++v) {
                SAGE_INFO("SaveManager: Applying migration v{} -> v{}", v, v + 1);
                
                switch (v) {
                    case 0: // v0 -> v1
                        // Example migration: add new fields
                        if (!data.contains("inventory")) {
                            data["inventory"] = nlohmann::json::array();
                        }
                        if (!data.contains("quests")) {
                            data["quests"] = nlohmann::json::object();
                        }
                        break;
                    
                    // Add more migrations as versions increase
                    default:
                        break;
                }
            }

            // Save migrated data
            data["saveVersion"] = CURRENT_SAVE_VERSION;
            
            std::ofstream outFile(filepath);
            outFile << data.dump(4);
            outFile.close();

            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("SaveManager: Migration failed: {}", e.what());
            return false;
        }
    }

    std::string GetSlotFilePath(int slotIndex) const {
        return m_SaveDirectory + "/slot_" + std::to_string(slotIndex) + ".json";
    }

    std::string GetMetadataFilePath() const {
        return m_SaveDirectory + "/metadata.json";
    }

    void SaveMetadata() {
        nlohmann::json j;
        j["version"] = 1;
        
        nlohmann::json slotsArray = nlohmann::json::array();
        for (const auto& slot : m_Slots) {
            slotsArray.push_back(slot.ToJson());
        }
        j["slots"] = slotsArray;
        j["autosave"] = m_AutoSaveSlot.ToJson();

        std::ofstream file(GetMetadataFilePath());
        if (file.is_open()) {
            file << j.dump(4);
        }
    }

    void LoadMetadata() {
        // Initialize empty slots
        m_Slots.clear();
        for (int i = 0; i < m_MaxSlots; ++i) {
            m_Slots.emplace_back(i);
        }

        std::string metaPath = GetMetadataFilePath();
        if (!std::filesystem::exists(metaPath)) return;

        try {
            std::ifstream file(metaPath);
            nlohmann::json j;
            file >> j;

            if (j.contains("slots")) {
                auto slotsArray = j["slots"];
                size_t index = 0;
                for (const auto& slotJson : slotsArray) {
                    if (index < m_Slots.size()) {
                        m_Slots[index].FromJson(slotJson);
                    }
                    ++index;
                }
            }

            if (j.contains("autosave")) {
                m_AutoSaveSlot.FromJson(j["autosave"]);
            }

        } catch (const std::exception& e) {
            SAGE_ERROR("SaveManager: Failed to load metadata: {}", e.what());
        }
    }

    std::string m_SaveDirectory = "saves";
    std::vector<SaveSlot> m_Slots;
    SaveSlot m_AutoSaveSlot;
    
    int m_MaxSlots = 5;
    int m_CurrentSlot = -1;

    // AutoSave settings
    bool m_AutoSaveEnabled = true;
    float m_AutoSaveInterval = 300.0f; // 5 minutes
    float m_AutoSaveTimer = 0.0f;
    bool m_NeedsAutoSave = false;
    std::time_t m_LastAutoSaveTime = 0;
};

} // namespace SAGE
