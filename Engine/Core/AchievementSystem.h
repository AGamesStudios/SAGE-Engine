#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "nlohmann/json.hpp"

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Тип достижения
 */
enum class AchievementType {
    OneTime,        ///< Одноразовое (убить босса)
    Incremental,    ///< Накопительное (убить 100 врагов)
    Progress        ///< Прогрессивное (собрать 10/50/100 монет)
};

/**
 * @brief Достижение
 */
struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;
    AchievementType type = AchievementType::OneTime;
    
    // Для Incremental/Progress типов
    int targetValue = 0;
    int currentValue = 0;
    
    // Награда
    int rewardXP = 0;
    int rewardCoins = 0;
    
    // Состояние
    bool unlocked = false;
    std::string unlockedTimestamp;
    
    // Скрытое достижение
    bool hidden = false;
    
    Achievement() = default;
    
    /**
     * @brief Проверить, выполнено ли достижение
     */
    bool IsCompleted() const {
        if (unlocked) return true;
        
        switch (type) {
            case AchievementType::OneTime:
                return false;  // Unlocked вручную
                
            case AchievementType::Incremental:
            case AchievementType::Progress:
                return currentValue >= targetValue;
                
            default:
                return false;
        }
    }
    
    /**
     * @brief Получить прогресс (0.0 - 1.0)
     */
    float GetProgress() const {
        if (type == AchievementType::OneTime) {
            return unlocked ? 1.0f : 0.0f;
        }
        
        if (targetValue == 0) return 0.0f;
        
        return std::min(1.0f, static_cast<float>(currentValue) / static_cast<float>(targetValue));
    }
    
    json ToJson() const {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["iconPath"] = iconPath;
        j["type"] = static_cast<int>(type);
        j["targetValue"] = targetValue;
        j["currentValue"] = currentValue;
        j["rewardXP"] = rewardXP;
        j["rewardCoins"] = rewardCoins;
        j["unlocked"] = unlocked;
        j["unlockedTimestamp"] = unlockedTimestamp;
        j["hidden"] = hidden;
        return j;
    }
    
    void FromJson(const json& j) {
        id = j.value("id", "");
        name = j.value("name", "");
        description = j.value("description", "");
        iconPath = j.value("iconPath", "");
        type = static_cast<AchievementType>(j.value("type", 0));
        targetValue = j.value("targetValue", 0);
        currentValue = j.value("currentValue", 0);
        rewardXP = j.value("rewardXP", 0);
        rewardCoins = j.value("rewardCoins", 0);
        unlocked = j.value("unlocked", false);
        unlockedTimestamp = j.value("unlockedTimestamp", "");
        hidden = j.value("hidden", false);
    }
};

/**
 * @brief Статистика игрока
 */
struct PlayerStats {
    std::unordered_map<std::string, int> intStats;
    std::unordered_map<std::string, float> floatStats;
    
    /**
     * @brief Установить int статистику
     */
    void SetInt(const std::string& key, int value) {
        intStats[key] = value;
    }
    
    /**
     * @brief Получить int статистику
     */
    int GetInt(const std::string& key, int defaultValue = 0) const {
        auto it = intStats.find(key);
        return (it != intStats.end()) ? it->second : defaultValue;
    }
    
    /**
     * @brief Увеличить int статистику
     */
    void IncrementInt(const std::string& key, int amount = 1) {
        intStats[key] += amount;
    }
    
    /**
     * @brief Установить float статистику
     */
    void SetFloat(const std::string& key, float value) {
        floatStats[key] = value;
    }
    
    /**
     * @brief Получить float статистику
     */
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
        auto it = floatStats.find(key);
        return (it != floatStats.end()) ? it->second : defaultValue;
    }
    
    json ToJson() const {
        json j;
        j["intStats"] = intStats;
        j["floatStats"] = floatStats;
        return j;
    }
    
    void FromJson(const json& j) {
        if (j.contains("intStats")) {
            intStats = j["intStats"].get<std::unordered_map<std::string, int>>();
        }
        if (j.contains("floatStats")) {
            floatStats = j["floatStats"].get<std::unordered_map<std::string, float>>();
        }
    }
};

/**
 * @brief Система достижений
 */
class AchievementSystem {
public:
    AchievementSystem();
    ~AchievementSystem() = default;
    
    // === Управление достижениями ===
    
    /**
     * @brief Зарегистрировать достижение
     */
    void RegisterAchievement(const Achievement& achievement);
    
    /**
     * @brief Разблокировать достижение
     */
    bool UnlockAchievement(const std::string& id);
    
    /**
     * @brief Увеличить прогресс достижения
     */
    bool IncrementAchievement(const std::string& id, int amount = 1);
    
    /**
     * @brief Установить прогресс достижения
     */
    bool SetAchievementProgress(const std::string& id, int value);
    
    /**
     * @brief Получить достижение
     */
    Achievement* GetAchievement(const std::string& id);
    
    /**
     * @brief Получить все достижения
     */
    std::vector<Achievement*> GetAllAchievements();
    
    /**
     * @brief Получить разблокированные достижения
     */
    std::vector<Achievement*> GetUnlockedAchievements();
    
    /**
     * @brief Получить процент завершения (0-100)
     */
    float GetCompletionPercentage() const;
    
    // === Статистика ===
    
    /**
     * @brief Получить статистику игрока
     */
    PlayerStats& GetStats() { return m_Stats; }
    const PlayerStats& GetStats() const { return m_Stats; }
    
    /**
     * @brief Трекинг статистики (автоматическая проверка достижений)
     */
    void TrackStat(const std::string& statName, int value);
    
    // === Сохранение/Загрузка ===
    
    /**
     * @brief Сохранить достижения
     */
    bool Save(const std::string& filepath);
    
    /**
     * @brief Загрузить достижения
     */
    bool Load(const std::string& filepath);
    
    // === Callbacks ===
    
    /**
     * @brief Установить callback при разблокировке
     */
    void SetOnUnlockCallback(std::function<void(const Achievement&)> callback) {
        m_OnUnlock = callback;
    }
    
    /**
     * @brief Установить callback при обновлении прогресса
     */
    void SetOnProgressCallback(std::function<void(const Achievement&)> callback) {
        m_OnProgress = callback;
    }
    
private:
    std::unordered_map<std::string, Achievement> m_Achievements;
    PlayerStats m_Stats;
    
    std::function<void(const Achievement&)> m_OnUnlock;
    std::function<void(const Achievement&)> m_OnProgress;
    
    /**
     * @brief Проверить и разблокировать достижение, если выполнено
     */
    void CheckAndUnlock(Achievement& achievement);
    
    /**
     * @brief Получить текущий timestamp
     */
    std::string GetCurrentTimestamp() const;
};

} // namespace SAGE
