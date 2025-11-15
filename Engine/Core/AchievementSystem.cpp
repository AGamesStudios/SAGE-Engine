#include "AchievementSystem.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace SAGE {

AchievementSystem::AchievementSystem() {
}

void AchievementSystem::RegisterAchievement(const Achievement& achievement) {
    m_Achievements[achievement.id] = achievement;
}

bool AchievementSystem::UnlockAchievement(const std::string& id) {
    auto it = m_Achievements.find(id);
    if (it == m_Achievements.end()) {
        return false;
    }
    
    Achievement& achievement = it->second;
    
    if (achievement.unlocked) {
        return false;  // Уже разблокировано
    }
    
    achievement.unlocked = true;
    achievement.unlockedTimestamp = GetCurrentTimestamp();
    
    // Вызвать callback
    if (m_OnUnlock) {
        m_OnUnlock(achievement);
    }
    
    return true;
}

bool AchievementSystem::IncrementAchievement(const std::string& id, int amount) {
    auto it = m_Achievements.find(id);
    if (it == m_Achievements.end()) {
        return false;
    }
    
    Achievement& achievement = it->second;
    
    if (achievement.unlocked) {
        return false;  // Уже разблокировано
    }
    
    if (achievement.type != AchievementType::Incremental && 
        achievement.type != AchievementType::Progress) {
        return false;  // Не поддерживает прогресс
    }
    
    achievement.currentValue += amount;
    
    // Callback прогресса
    if (m_OnProgress) {
        m_OnProgress(achievement);
    }
    
    // Проверить завершение
    CheckAndUnlock(achievement);
    
    return true;
}

bool AchievementSystem::SetAchievementProgress(const std::string& id, int value) {
    auto it = m_Achievements.find(id);
    if (it == m_Achievements.end()) {
        return false;
    }
    
    Achievement& achievement = it->second;
    
    if (achievement.unlocked) {
        return false;
    }
    
    achievement.currentValue = value;
    
    if (m_OnProgress) {
        m_OnProgress(achievement);
    }
    
    CheckAndUnlock(achievement);
    
    return true;
}

Achievement* AchievementSystem::GetAchievement(const std::string& id) {
    auto it = m_Achievements.find(id);
    return (it != m_Achievements.end()) ? &it->second : nullptr;
}

std::vector<Achievement*> AchievementSystem::GetAllAchievements() {
    std::vector<Achievement*> result;
    result.reserve(m_Achievements.size());
    
    for (auto& pair : m_Achievements) {
        result.push_back(&pair.second);
    }
    
    return result;
}

std::vector<Achievement*> AchievementSystem::GetUnlockedAchievements() {
    std::vector<Achievement*> result;
    
    for (auto& pair : m_Achievements) {
        if (pair.second.unlocked) {
            result.push_back(&pair.second);
        }
    }
    
    return result;
}

float AchievementSystem::GetCompletionPercentage() const {
    if (m_Achievements.empty()) {
        return 0.0f;
    }
    
    int unlockedCount = 0;
    for (const auto& pair : m_Achievements) {
        if (pair.second.unlocked) {
            ++unlockedCount;
        }
    }
    
    return (static_cast<float>(unlockedCount) / static_cast<float>(m_Achievements.size())) * 100.0f;
}

void AchievementSystem::TrackStat(const std::string& statName, int value) {
    m_Stats.SetInt(statName, value);
    
    // Проверить все достижения, связанные с этой статистикой
    for (auto& pair : m_Achievements) {
        Achievement& achievement = pair.second;
        
        // Пример: если достижение называется "kill_100_enemies", 
        // проверяем статистику "enemies_killed"
        // (В реальной игре нужна более сложная система связей)
    }
}

bool AchievementSystem::Save(const std::string& filepath) {
    try {
        json j;
        
        // Сохранить достижения
        json achievementsJson = json::array();
        for (const auto& pair : m_Achievements) {
            achievementsJson.push_back(pair.second.ToJson());
        }
        j["achievements"] = achievementsJson;
        
        // Сохранить статистику
        j["stats"] = m_Stats.ToJson();
        
        // Записать в файл
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

bool AchievementSystem::Load(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        // Загрузить достижения
        if (j.contains("achievements")) {
            for (const auto& achievementJson : j["achievements"]) {
                Achievement achievement;
                achievement.FromJson(achievementJson);
                m_Achievements[achievement.id] = achievement;
            }
        }
        
        // Загрузить статистику
        if (j.contains("stats")) {
            m_Stats.FromJson(j["stats"]);
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

void AchievementSystem::CheckAndUnlock(Achievement& achievement) {
    if (achievement.IsCompleted() && !achievement.unlocked) {
        achievement.unlocked = true;
        achievement.unlockedTimestamp = GetCurrentTimestamp();
        
        if (m_OnUnlock) {
            m_OnUnlock(achievement);
        }
    }
}

std::string AchievementSystem::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace SAGE
