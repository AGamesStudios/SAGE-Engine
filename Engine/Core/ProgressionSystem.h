#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

// Тип разблокируемого контента
enum class UnlockableType {
    Item,       // Предмет
    Ability,    // Способность
    Character,  // Персонаж
    Level,      // Уровень
    Feature,    // Игровая функция
    Custom      // Пользовательский
};

// Разблокируемый контент
struct Unlockable {
    std::string id;
    std::string name;
    std::string description;
    UnlockableType type = UnlockableType::Custom;
    int requiredLevel = 1;
    int requiredXP = 0;
    bool unlocked = false;
    
    json ToJson() const;
    void FromJson(const json& j);
};

// Навык в дереве навыков
struct Skill {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;
    
    int maxLevel = 1;           // Максимальный уровень навыка
    int currentLevel = 0;       // Текущий уровень
    
    int requiredLevel = 1;      // Требуемый уровень игрока
    int pointsPerLevel = 1;     // Очков навыков на уровень
    
    std::vector<std::string> prerequisites;  // ID требуемых навыков
    
    bool unlocked = false;
    
    bool CanUnlock(int playerLevel, const std::unordered_map<std::string, Skill>& allSkills) const;
    bool CanLevelUp() const { return currentLevel < maxLevel; }
    bool IsMaxLevel() const { return currentLevel >= maxLevel; }
    
    json ToJson() const;
    void FromJson(const json& j);
};

// Дерево навыков
class SkillTree {
public:
    SkillTree() = default;
    
    void AddSkill(const Skill& skill);
    bool UnlockSkill(const std::string& skillId, int playerLevel);
    bool LevelUpSkill(const std::string& skillId);
    
    Skill* GetSkill(const std::string& id);
    std::vector<Skill*> GetAllSkills();
    std::vector<Skill*> GetUnlockedSkills();
    std::vector<Skill*> GetAvailableSkills(int playerLevel);
    
    int GetTotalPointsSpent() const;
    
    json ToJson() const;
    void FromJson(const json& j);
    
private:
    std::unordered_map<std::string, Skill> m_Skills;
};

// Система прогрессии
class ProgressionSystem {
public:
    ProgressionSystem();
    
    // XP и уровни
    void AddXP(int amount);
    void SetXP(int xp);
    int GetXP() const { return m_CurrentXP; }
    
    void LevelUp();
    void SetLevel(int level);
    int GetLevel() const { return m_Level; }
    
    int GetXPForNextLevel() const;
    int GetXPForLevel(int level) const;
    float GetLevelProgress() const;  // 0.0 - 1.0
    
    // Очки навыков
    void AddSkillPoints(int amount);
    void SpendSkillPoints(int amount);
    int GetSkillPoints() const { return m_SkillPoints; }
    int GetTotalSkillPoints() const { return m_TotalSkillPointsEarned; }
    
    // Дерево навыков
    SkillTree& GetSkillTree() { return m_SkillTree; }
    const SkillTree& GetSkillTree() const { return m_SkillTree; }
    bool UnlockSkill(const std::string& skillId);
    bool LevelUpSkill(const std::string& skillId);
    
    // Разблокировки
    void RegisterUnlockable(const Unlockable& unlockable);
    bool UnlockContent(const std::string& id);
    bool IsUnlocked(const std::string& id) const;
    
    std::vector<Unlockable*> GetAllUnlockables();
    std::vector<Unlockable*> GetUnlockedContent();
    std::vector<Unlockable*> GetAvailableUnlockables();
    
    // Настройки прогрессии
    void SetXPCurve(float baseXP, float multiplier) {
        m_BaseXPPerLevel = baseXP;
        m_XPMultiplier = multiplier;
    }
    
    void SetSkillPointsPerLevel(int points) {
        m_SkillPointsPerLevel = points;
    }
    
    void SetMaxLevel(int maxLevel) {
        m_MaxLevel = maxLevel;
    }
    
    int GetMaxLevel() const { return m_MaxLevel; }
    
    // Callbacks
    void SetOnLevelUp(std::function<void(int newLevel)> callback) {
        m_OnLevelUp = callback;
    }
    
    void SetOnSkillUnlock(std::function<void(const Skill&)> callback) {
        m_OnSkillUnlock = callback;
    }
    
    void SetOnContentUnlock(std::function<void(const Unlockable&)> callback) {
        m_OnContentUnlock = callback;
    }
    
    // Сохранение/загрузка
    bool Save(const std::string& filepath);
    bool Load(const std::string& filepath);
    
    json ToJson() const;
    void FromJson(const json& j);
    
private:
    // XP и уровни
    int m_Level = 1;
    int m_CurrentXP = 0;
    int m_MaxLevel = 100;
    
    float m_BaseXPPerLevel = 100.0f;
    float m_XPMultiplier = 1.5f;  // Экспоненциальная кривая
    
    // Очки навыков
    int m_SkillPoints = 0;
    int m_SkillPointsPerLevel = 1;
    int m_TotalSkillPointsEarned = 0;
    
    // Дерево навыков
    SkillTree m_SkillTree;
    
    // Разблокировки
    std::unordered_map<std::string, Unlockable> m_Unlockables;
    
    // Callbacks
    std::function<void(int newLevel)> m_OnLevelUp;
    std::function<void(const Skill&)> m_OnSkillUnlock;
    std::function<void(const Unlockable&)> m_OnContentUnlock;
    
    void CheckUnlockables();
};

} // namespace SAGE
