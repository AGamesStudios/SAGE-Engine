#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <fstream>
#include "ThirdParty/json/json.hpp"

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Данные сохранения
 */
struct SaveData {
    std::string slotName;
    std::string sceneName;
    float playtime = 0.0f;
    std::string timestamp;
    int version = 1;
    
    // Кастомные данные игры
    json gameData;
    
    SaveData() = default;
    
    /**
     * @brief Установить значение
     */
    template<typename T>
    void Set(const std::string& key, const T& value) {
        gameData[key] = value;
    }
    
    /**
     * @brief Получить значение
     */
    template<typename T>
    T Get(const std::string& key, const T& defaultValue = T()) const {
        if (gameData.contains(key)) {
            return gameData[key].get<T>();
        }
        return defaultValue;
    }
    
    /**
     * @brief Проверить наличие ключа
     */
    bool Has(const std::string& key) const {
        return gameData.contains(key);
    }
    
    /**
     * @brief Конвертировать в JSON
     */
    json ToJson() const {
        json j;
        j["slotName"] = slotName;
        j["sceneName"] = sceneName;
        j["playtime"] = playtime;
        j["timestamp"] = timestamp;
        j["version"] = version;
        j["gameData"] = gameData;
        return j;
    }
    
    /**
     * @brief Загрузить из JSON
     */
    void FromJson(const json& j) {
        slotName = j.value("slotName", "");
        sceneName = j.value("sceneName", "");
        playtime = j.value("playtime", 0.0f);
        timestamp = j.value("timestamp", "");
        version = j.value("version", 1);
        
        if (j.contains("gameData")) {
            gameData = j["gameData"];
        }
    }
};

/**
 * @brief Checkpoint (точка сохранения)
 */
struct Checkpoint {
    std::string id;
    std::string sceneName;
    float x = 0.0f;
    float y = 0.0f;
    json data;
    
    json ToJson() const {
        json j;
        j["id"] = id;
        j["sceneName"] = sceneName;
        j["x"] = x;
        j["y"] = y;
        j["data"] = data;
        return j;
    }
    
    void FromJson(const json& j) {
        id = j.value("id", "");
        sceneName = j.value("sceneName", "");
        x = j.value("x", 0.0f);
        y = j.value("y", 0.0f);
        if (j.contains("data")) {
            data = j["data"];
        }
    }
};

/**
 * @brief Система сохранений
 */
class SaveSystem {
public:
    SaveSystem();
    ~SaveSystem() = default;
    
    // === Основные операции ===
    
    /**
     * @brief Сохранить игру в слот
     */
    bool Save(const std::string& slotName, const SaveData& data);
    
    /**
     * @brief Загрузить игру из слота
     */
    bool Load(const std::string& slotName, SaveData& outData);
    
    /**
     * @brief Удалить слот сохранения
     */
    bool DeleteSave(const std::string& slotName);
    
    /**
     * @brief Проверить существование сохранения
     */
    bool SaveExists(const std::string& slotName) const;
    
    /**
     * @brief Получить список всех слотов
     */
    std::vector<std::string> GetSaveSlots() const;
    
    /**
     * @brief Получить информацию о сохранении (без полной загрузки)
     */
    bool GetSaveInfo(const std::string& slotName, SaveData& outInfo);
    
    // === Автосохранение ===
    
    /**
     * @brief Включить автосохранение
     */
    void EnableAutoSave(bool enable, float interval = 300.0f);
    
    /**
     * @brief Обновить автосохранение (вызывать каждый фрейм)
     */
    void UpdateAutoSave(float deltaTime);
    
    /**
     * @brief Выполнить автосохранение сейчас
     */
    void AutoSaveNow();
    
    // === Checkpoint система ===
    
    /**
     * @brief Сохранить checkpoint
     */
    void SaveCheckpoint(const Checkpoint& checkpoint);
    
    /**
     * @brief Загрузить последний checkpoint
     */
    bool LoadLastCheckpoint(Checkpoint& outCheckpoint);
    
    /**
     * @brief Очистить все checkpoints
     */
    void ClearCheckpoints();
    
    // === Настройки ===
    
    /**
     * @brief Установить директорию сохранений
     */
    void SetSaveDirectory(const std::string& directory);
    
    /**
     * @brief Получить директорию сохранений
     */
    std::string GetSaveDirectory() const { return m_SaveDirectory; }
    
    /**
     * @brief Установить колбэк для сериализации игровых данных
     */
    void SetSerializeCallback(std::function<void(SaveData&)> callback) {
        m_OnSerialize = callback;
    }
    
    /**
     * @brief Установить колбэк для десериализации игровых данных
     */
    void SetDeserializeCallback(std::function<void(const SaveData&)> callback) {
        m_OnDeserialize = callback;
    }
    
    /**
     * @brief Получить текущий активный слот
     */
    std::string GetActiveSlot() const { return m_ActiveSlot; }
    
    /**
     * @brief Установить активный слот
     */
    void SetActiveSlot(const std::string& slotName) { m_ActiveSlot = slotName; }
    
private:
    std::string m_SaveDirectory = "saves/";
    std::string m_ActiveSlot = "slot1";
    
    // Автосохранение
    bool m_AutoSaveEnabled = false;
    float m_AutoSaveInterval = 300.0f;  // 5 минут
    float m_AutoSaveTimer = 0.0f;
    
    // Checkpoints
    std::vector<Checkpoint> m_Checkpoints;
    
    // Callbacks
    std::function<void(SaveData&)> m_OnSerialize;
    std::function<void(const SaveData&)> m_OnDeserialize;
    
    /**
     * @brief Получить полный путь к файлу сохранения
     */
    std::string GetSaveFilePath(const std::string& slotName) const;
    
    /**
     * @brief Получить текущий timestamp
     */
    std::string GetCurrentTimestamp() const;
};

} // namespace SAGE
