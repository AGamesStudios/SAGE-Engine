#pragma once

#include "LODSystem.h"
#include <string>

namespace SAGE {

// Sprite LOD - разные текстуры для разных дистанций
struct SpriteLOD {
    std::string texturePathHigh;    // Полное разрешение
    std::string texturePathMedium;  // 50% разрешения
    std::string texturePathLow;     // 25% разрешения
    
    uint32_t textureIdHigh = 0;
    uint32_t textureIdMedium = 0;
    uint32_t textureIdLow = 0;
    
    uint32_t GetTextureForLOD(LODLevel level) const {
        switch (level) {
            case LODLevel::High:   return textureIdHigh;
            case LODLevel::Medium: return textureIdMedium;
            case LODLevel::Low:    return textureIdLow;
            default:               return 0;
        }
    }
};

// Animation LOD - разные FPS для разных дистанций
struct AnimationLOD {
    int fpsHigh = 30;     // Полная анимация
    int fpsMedium = 15;   // Половина кадров
    int fpsLow = 5;       // Минимум кадров
    bool disableOnOff = true;  // Отключить анимацию на LODLevel::Off
    
    int GetFPSForLOD(LODLevel level) const {
        switch (level) {
            case LODLevel::High:   return fpsHigh;
            case LODLevel::Medium: return fpsMedium;
            case LODLevel::Low:    return fpsLow;
            case LODLevel::Off:    return disableOnOff ? 0 : fpsLow;
            default:               return fpsHigh;
        }
    }
    
    bool ShouldAnimate(LODLevel level) const {
        if (level == LODLevel::Off && disableOnOff) {
            return false;
        }
        return true;
    }
};

// 2D LOD Component
struct LOD2DComponent {
    SpriteLOD spriteLOD;
    AnimationLOD animationLOD;
    
    LODLevel currentLevel = LODLevel::High;
    
    // Callback при смене LOD
    std::function<void(LODLevel oldLevel, LODLevel newLevel)> onLODChange;
    
    void UpdateLOD(LODLevel newLevel) {
        if (newLevel != currentLevel) {
            LODLevel oldLevel = currentLevel;
            currentLevel = newLevel;
            
            if (onLODChange) {
                onLODChange(oldLevel, newLevel);
            }
        }
    }
};

// LOD Manager для 2D объектов
class LOD2DManager {
public:
    void RegisterSprite(uint32_t entityId, LOD2DComponent& component, const glm::vec2& position) {
        m_Objects[entityId] = {&component, position};
    }
    
    void UnregisterSprite(uint32_t entityId) {
        m_Objects.erase(entityId);
    }
    
    void UpdatePosition(uint32_t entityId, const glm::vec2& position) {
        auto it = m_Objects.find(entityId);
        if (it != m_Objects.end()) {
            it->second.position = position;
        }
    }
    
    void UpdateAll(const glm::vec2& cameraPosition, const LODConfig& config) {
        for (auto& [entityId, entry] : m_Objects) {
            float distance = glm::distance(cameraPosition, entry.position);
            
            LODLevel newLevel = LODLevel::Off;
            if (distance < config.distanceHigh) {
                newLevel = LODLevel::High;
            } else if (distance < config.distanceMedium) {
                newLevel = LODLevel::Medium;
            } else if (distance < config.distanceLow) {
                newLevel = LODLevel::Low;
            }
            
            entry.component->UpdateLOD(newLevel);
        }
    }
    
    struct Stats {
        int high = 0;
        int medium = 0;
        int low = 0;
        int off = 0;
    };
    
    Stats GetStats() const {
        Stats stats;
        for (const auto& [id, entry] : m_Objects) {
            switch (entry.component->currentLevel) {
                case LODLevel::High:   stats.high++; break;
                case LODLevel::Medium: stats.medium++; break;
                case LODLevel::Low:    stats.low++; break;
                case LODLevel::Off:    stats.off++; break;
            }
        }
        return stats;
    }
    
private:
    struct ObjectEntry {
        LOD2DComponent* component;
        glm::vec2 position;
    };
    
    std::unordered_map<uint32_t, ObjectEntry> m_Objects;
};

} // namespace SAGE
