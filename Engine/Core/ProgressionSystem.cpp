#include "ProgressionSystem.h"
#include <fstream>
#include <cmath>
#include <algorithm>

namespace SAGE {

// Unlockable implementations
json Unlockable::ToJson() const {
    return json{
        {"id", id},
        {"name", name},
        {"description", description},
        {"type", static_cast<int>(type)},
        {"requiredLevel", requiredLevel},
        {"requiredXP", requiredXP},
        {"unlocked", unlocked}
    };
}

void Unlockable::FromJson(const json& j) {
    id = j.value("id", "");
    name = j.value("name", "");
    description = j.value("description", "");
    type = static_cast<UnlockableType>(j.value("type", 0));
    requiredLevel = j.value("requiredLevel", 1);
    requiredXP = j.value("requiredXP", 0);
    unlocked = j.value("unlocked", false);
}

// Skill implementations
bool Skill::CanUnlock(int playerLevel, const std::unordered_map<std::string, Skill>& allSkills) const {
    // Проверить уровень игрока
    if (playerLevel < requiredLevel) {
        return false;
    }
    
    // Проверить предварительные условия
    for (const auto& prereqId : prerequisites) {
        auto it = allSkills.find(prereqId);
        if (it == allSkills.end() || !it->second.unlocked) {
            return false;
        }
    }
    
    return true;
}

json Skill::ToJson() const {
    return json{
        {"id", id},
        {"name", name},
        {"description", description},
        {"iconPath", iconPath},
        {"maxLevel", maxLevel},
        {"currentLevel", currentLevel},
        {"requiredLevel", requiredLevel},
        {"pointsPerLevel", pointsPerLevel},
        {"prerequisites", prerequisites},
        {"unlocked", unlocked}
    };
}

void Skill::FromJson(const json& j) {
    id = j.value("id", "");
    name = j.value("name", "");
    description = j.value("description", "");
    iconPath = j.value("iconPath", "");
    maxLevel = j.value("maxLevel", 1);
    currentLevel = j.value("currentLevel", 0);
    requiredLevel = j.value("requiredLevel", 1);
    pointsPerLevel = j.value("pointsPerLevel", 1);
    
    if (j.contains("prerequisites")) {
        prerequisites = j["prerequisites"].get<std::vector<std::string>>();
    }
    
    unlocked = j.value("unlocked", false);
}

// SkillTree implementations
void SkillTree::AddSkill(const Skill& skill) {
    m_Skills[skill.id] = skill;
}

bool SkillTree::UnlockSkill(const std::string& skillId, int playerLevel) {
    auto it = m_Skills.find(skillId);
    if (it == m_Skills.end()) {
        return false;
    }
    
    Skill& skill = it->second;
    
    if (skill.unlocked) {
        return false;  // Уже разблокировано
    }
    
    if (!skill.CanUnlock(playerLevel, m_Skills)) {
        return false;  // Не выполнены требования
    }
    
    skill.unlocked = true;
    skill.currentLevel = 1;
    
    return true;
}

bool SkillTree::LevelUpSkill(const std::string& skillId) {
    auto it = m_Skills.find(skillId);
    if (it == m_Skills.end()) {
        return false;
    }
    
    Skill& skill = it->second;
    
    if (!skill.unlocked || !skill.CanLevelUp()) {
        return false;
    }
    
    skill.currentLevel++;
    return true;
}

Skill* SkillTree::GetSkill(const std::string& id) {
    auto it = m_Skills.find(id);
    return (it != m_Skills.end()) ? &it->second : nullptr;
}

std::vector<Skill*> SkillTree::GetAllSkills() {
    std::vector<Skill*> result;
    result.reserve(m_Skills.size());
    
    for (auto& pair : m_Skills) {
        result.push_back(&pair.second);
    }
    
    return result;
}

std::vector<Skill*> SkillTree::GetUnlockedSkills() {
    std::vector<Skill*> result;
    
    for (auto& pair : m_Skills) {
        if (pair.second.unlocked) {
            result.push_back(&pair.second);
        }
    }
    
    return result;
}

std::vector<Skill*> SkillTree::GetAvailableSkills(int playerLevel) {
    std::vector<Skill*> result;
    
    for (auto& pair : m_Skills) {
        Skill& skill = pair.second;
        if (!skill.unlocked && skill.CanUnlock(playerLevel, m_Skills)) {
            result.push_back(&skill);
        }
    }
    
    return result;
}

int SkillTree::GetTotalPointsSpent() const {
    int total = 0;
    
    for (const auto& pair : m_Skills) {
        const Skill& skill = pair.second;
        if (skill.unlocked) {
            total += skill.currentLevel * skill.pointsPerLevel;
        }
    }
    
    return total;
}

json SkillTree::ToJson() const {
    json j = json::array();
    
    for (const auto& pair : m_Skills) {
        j.push_back(pair.second.ToJson());
    }
    
    return j;
}

void SkillTree::FromJson(const json& j) {
    m_Skills.clear();
    
    for (const auto& skillJson : j) {
        Skill skill;
        skill.FromJson(skillJson);
        m_Skills[skill.id] = skill;
    }
}

// ProgressionSystem implementations
ProgressionSystem::ProgressionSystem() {
}

void ProgressionSystem::AddXP(int amount) {
    m_CurrentXP += amount;
    
    // Проверить повышение уровня
    while (m_Level < m_MaxLevel && m_CurrentXP >= GetXPForNextLevel()) {
        LevelUp();
    }
}

void ProgressionSystem::SetXP(int xp) {
    m_CurrentXP = xp;
}

void ProgressionSystem::LevelUp() {
    if (m_Level >= m_MaxLevel) {
        return;
    }
    
    m_Level++;
    
    // Дать очки навыков
    AddSkillPoints(m_SkillPointsPerLevel);
    
    // Callback
    if (m_OnLevelUp) {
        m_OnLevelUp(m_Level);
    }
    
    // Проверить разблокировки
    CheckUnlockables();
}

void ProgressionSystem::SetLevel(int level) {
    m_Level = std::clamp(level, 1, m_MaxLevel);
}

int ProgressionSystem::GetXPForNextLevel() const {
    return GetXPForLevel(m_Level + 1);
}

int ProgressionSystem::GetXPForLevel(int level) const {
    if (level <= 1) {
        return 0;
    }
    
    // Экспоненциальная кривая: XP = baseXP * (multiplier ^ (level - 1))
    return static_cast<int>(m_BaseXPPerLevel * std::pow(m_XPMultiplier, level - 1));
}

float ProgressionSystem::GetLevelProgress() const {
    if (m_Level >= m_MaxLevel) {
        return 1.0f;
    }
    
    int currentLevelXP = GetXPForLevel(m_Level);
    int nextLevelXP = GetXPForNextLevel();
    int xpInLevel = m_CurrentXP - currentLevelXP;
    int xpNeeded = nextLevelXP - currentLevelXP;
    
    return static_cast<float>(xpInLevel) / static_cast<float>(xpNeeded);
}

void ProgressionSystem::AddSkillPoints(int amount) {
    m_SkillPoints += amount;
    m_TotalSkillPointsEarned += amount;
}

void ProgressionSystem::SpendSkillPoints(int amount) {
    m_SkillPoints = std::max(0, m_SkillPoints - amount);
}

bool ProgressionSystem::UnlockSkill(const std::string& skillId) {
    Skill* skill = m_SkillTree.GetSkill(skillId);
    if (!skill) {
        return false;
    }
    
    // Проверить очки навыков
    if (m_SkillPoints < skill->pointsPerLevel) {
        return false;
    }
    
    if (m_SkillTree.UnlockSkill(skillId, m_Level)) {
        SpendSkillPoints(skill->pointsPerLevel);
        
        if (m_OnSkillUnlock) {
            m_OnSkillUnlock(*skill);
        }
        
        return true;
    }
    
    return false;
}

bool ProgressionSystem::LevelUpSkill(const std::string& skillId) {
    Skill* skill = m_SkillTree.GetSkill(skillId);
    if (!skill) {
        return false;
    }
    
    if (m_SkillPoints < skill->pointsPerLevel) {
        return false;
    }
    
    if (m_SkillTree.LevelUpSkill(skillId)) {
        SpendSkillPoints(skill->pointsPerLevel);
        return true;
    }
    
    return false;
}

void ProgressionSystem::RegisterUnlockable(const Unlockable& unlockable) {
    m_Unlockables[unlockable.id] = unlockable;
}

bool ProgressionSystem::UnlockContent(const std::string& id) {
    auto it = m_Unlockables.find(id);
    if (it == m_Unlockables.end()) {
        return false;
    }
    
    Unlockable& unlockable = it->second;
    
    if (unlockable.unlocked) {
        return false;
    }
    
    unlockable.unlocked = true;
    
    if (m_OnContentUnlock) {
        m_OnContentUnlock(unlockable);
    }
    
    return true;
}

bool ProgressionSystem::IsUnlocked(const std::string& id) const {
    auto it = m_Unlockables.find(id);
    return (it != m_Unlockables.end() && it->second.unlocked);
}

std::vector<Unlockable*> ProgressionSystem::GetAllUnlockables() {
    std::vector<Unlockable*> result;
    result.reserve(m_Unlockables.size());
    
    for (auto& pair : m_Unlockables) {
        result.push_back(&pair.second);
    }
    
    return result;
}

std::vector<Unlockable*> ProgressionSystem::GetUnlockedContent() {
    std::vector<Unlockable*> result;
    
    for (auto& pair : m_Unlockables) {
        if (pair.second.unlocked) {
            result.push_back(&pair.second);
        }
    }
    
    return result;
}

std::vector<Unlockable*> ProgressionSystem::GetAvailableUnlockables() {
    std::vector<Unlockable*> result;
    
    for (auto& pair : m_Unlockables) {
        Unlockable& unlockable = pair.second;
        
        if (!unlockable.unlocked &&
            m_Level >= unlockable.requiredLevel &&
            m_CurrentXP >= unlockable.requiredXP) {
            result.push_back(&unlockable);
        }
    }
    
    return result;
}

void ProgressionSystem::CheckUnlockables() {
    for (auto& pair : m_Unlockables) {
        Unlockable& unlockable = pair.second;
        
        if (!unlockable.unlocked &&
            m_Level >= unlockable.requiredLevel &&
            m_CurrentXP >= unlockable.requiredXP) {
            UnlockContent(unlockable.id);
        }
    }
}

bool ProgressionSystem::Save(const std::string& filepath) {
    try {
        json j = ToJson();
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4);
        file.close();
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool ProgressionSystem::Load(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        FromJson(j);
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

json ProgressionSystem::ToJson() const {
    json unlockablesJson = json::array();
    for (const auto& pair : m_Unlockables) {
        unlockablesJson.push_back(pair.second.ToJson());
    }
    
    return json{
        {"level", m_Level},
        {"currentXP", m_CurrentXP},
        {"maxLevel", m_MaxLevel},
        {"baseXPPerLevel", m_BaseXPPerLevel},
        {"xpMultiplier", m_XPMultiplier},
        {"skillPoints", m_SkillPoints},
        {"skillPointsPerLevel", m_SkillPointsPerLevel},
        {"totalSkillPointsEarned", m_TotalSkillPointsEarned},
        {"skillTree", m_SkillTree.ToJson()},
        {"unlockables", unlockablesJson}
    };
}

void ProgressionSystem::FromJson(const json& j) {
    m_Level = j.value("level", 1);
    m_CurrentXP = j.value("currentXP", 0);
    m_MaxLevel = j.value("maxLevel", 100);
    m_BaseXPPerLevel = j.value("baseXPPerLevel", 100.0f);
    m_XPMultiplier = j.value("xpMultiplier", 1.5f);
    m_SkillPoints = j.value("skillPoints", 0);
    m_SkillPointsPerLevel = j.value("skillPointsPerLevel", 1);
    m_TotalSkillPointsEarned = j.value("totalSkillPointsEarned", 0);
    
    if (j.contains("skillTree")) {
        m_SkillTree.FromJson(j["skillTree"]);
    }
    
    if (j.contains("unlockables")) {
        m_Unlockables.clear();
        for (const auto& unlockableJson : j["unlockables"]) {
            Unlockable unlockable;
            unlockable.FromJson(unlockableJson);
            m_Unlockables[unlockable.id] = unlockable;
        }
    }
}

} // namespace SAGE
