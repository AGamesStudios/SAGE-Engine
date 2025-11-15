#include "SaveSystem.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace SAGE {

namespace fs = std::filesystem;

SaveSystem::SaveSystem() {
    // Создать директорию сохранений, если не существует
    if (!fs::exists(m_SaveDirectory)) {
        fs::create_directories(m_SaveDirectory);
    }
}

bool SaveSystem::Save(const std::string& slotName, const SaveData& data) {
    try {
        // Обновить timestamp
        SaveData saveData = data;
        saveData.slotName = slotName;
        saveData.timestamp = GetCurrentTimestamp();
        
        // Вызвать callback сериализации
        if (m_OnSerialize) {
            m_OnSerialize(saveData);
        }
        
        // Конвертировать в JSON
        json j = saveData.ToJson();
        
        // Записать в файл
        std::string filepath = GetSaveFilePath(slotName);
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4);  // Pretty print с отступами
        file.close();
        
        m_ActiveSlot = slotName;
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool SaveSystem::Load(const std::string& slotName, SaveData& outData) {
    try {
        std::string filepath = GetSaveFilePath(slotName);
        
        if (!fs::exists(filepath)) {
            return false;
        }
        
        // Загрузить JSON
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        // Десериализовать
        outData.FromJson(j);
        
        // Вызвать callback десериализации
        if (m_OnDeserialize) {
            m_OnDeserialize(outData);
        }
        
        m_ActiveSlot = slotName;
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool SaveSystem::DeleteSave(const std::string& slotName) {
    try {
        std::string filepath = GetSaveFilePath(slotName);
        
        if (fs::exists(filepath)) {
            fs::remove(filepath);
            return true;
        }
        
        return false;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool SaveSystem::SaveExists(const std::string& slotName) const {
    std::string filepath = GetSaveFilePath(slotName);
    return fs::exists(filepath);
}

std::vector<std::string> SaveSystem::GetSaveSlots() const {
    std::vector<std::string> slots;
    
    try {
        if (!fs::exists(m_SaveDirectory)) {
            return slots;
        }
        
        for (const auto& entry : fs::directory_iterator(m_SaveDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sav") {
                std::string slotName = entry.path().stem().string();
                slots.push_back(slotName);
            }
        }
        
    } catch (const std::exception&) {
        // Ignore errors
    }
    
    return slots;
}

bool SaveSystem::GetSaveInfo(const std::string& slotName, SaveData& outInfo) {
    try {
        std::string filepath = GetSaveFilePath(slotName);
        
        if (!fs::exists(filepath)) {
            return false;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        // Загрузить только основную информацию (без gameData)
        outInfo.slotName = j.value("slotName", "");
        outInfo.sceneName = j.value("sceneName", "");
        outInfo.playtime = j.value("playtime", 0.0f);
        outInfo.timestamp = j.value("timestamp", "");
        outInfo.version = j.value("version", 1);
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

void SaveSystem::EnableAutoSave(bool enable, float interval) {
    m_AutoSaveEnabled = enable;
    m_AutoSaveInterval = interval;
    m_AutoSaveTimer = 0.0f;
}

void SaveSystem::UpdateAutoSave(float deltaTime) {
    if (!m_AutoSaveEnabled) {
        return;
    }
    
    m_AutoSaveTimer += deltaTime;
    
    if (m_AutoSaveTimer >= m_AutoSaveInterval) {
        AutoSaveNow();
        m_AutoSaveTimer = 0.0f;
    }
}

void SaveSystem::AutoSaveNow() {
    if (m_ActiveSlot.empty()) {
        m_ActiveSlot = "autosave";
    }
    
    SaveData data;
    data.slotName = m_ActiveSlot;
    
    Save(m_ActiveSlot, data);
}

void SaveSystem::SaveCheckpoint(const Checkpoint& checkpoint) {
    m_Checkpoints.push_back(checkpoint);
    
    // Сохранить checkpoints в файл
    try {
        json j = json::array();
        for (const auto& cp : m_Checkpoints) {
            j.push_back(cp.ToJson());
        }
        
        std::string filepath = m_SaveDirectory + "checkpoints.json";
        std::ofstream file(filepath);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
        
    } catch (const std::exception&) {
        // Ignore errors
    }
}

bool SaveSystem::LoadLastCheckpoint(Checkpoint& outCheckpoint) {
    if (m_Checkpoints.empty()) {
        // Попытаться загрузить из файла
        try {
            std::string filepath = m_SaveDirectory + "checkpoints.json";
            
            if (fs::exists(filepath)) {
                std::ifstream file(filepath);
                if (file.is_open()) {
                    json j;
                    file >> j;
                    file.close();
                    
                    m_Checkpoints.clear();
                    for (const auto& jcp : j) {
                        Checkpoint cp;
                        cp.FromJson(jcp);
                        m_Checkpoints.push_back(cp);
                    }
                }
            }
            
        } catch (const std::exception&) {
            return false;
        }
    }
    
    if (m_Checkpoints.empty()) {
        return false;
    }
    
    outCheckpoint = m_Checkpoints.back();
    return true;
}

void SaveSystem::ClearCheckpoints() {
    m_Checkpoints.clear();
    
    try {
        std::string filepath = m_SaveDirectory + "checkpoints.json";
        if (fs::exists(filepath)) {
            fs::remove(filepath);
        }
    } catch (const std::exception&) {
        // Ignore errors
    }
}

void SaveSystem::SetSaveDirectory(const std::string& directory) {
    m_SaveDirectory = directory;
    
    if (!m_SaveDirectory.empty() && m_SaveDirectory.back() != '/') {
        m_SaveDirectory += '/';
    }
    
    if (!fs::exists(m_SaveDirectory)) {
        fs::create_directories(m_SaveDirectory);
    }
}

std::string SaveSystem::GetSaveFilePath(const std::string& slotName) const {
    return m_SaveDirectory + slotName + ".sav";
}

std::string SaveSystem::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace SAGE
