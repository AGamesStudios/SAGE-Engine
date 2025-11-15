#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace SAGE {

/**
 * @brief Level-of-Detail (LOD) levels for rendering optimization
 */
enum class LODLevel {
    High = 0,    // Full detail (close to camera)
    Medium = 1,  // Medium detail
    Low = 2,     // Low detail (far from camera)
    Off = 3      // Culled (too far)
};

/**
 * @brief LOD configuration for an entity
 */
struct LODConfig {
    float distanceHigh = 50.0f;    // Distance threshold for high detail
    float distanceMedium = 100.0f; // Distance threshold for medium detail
    float distanceLow = 200.0f;    // Distance threshold for low detail
    float distanceCull = 300.0f;   // Beyond this, object is culled

    // Optional custom LOD selection logic
    std::function<LODLevel(float distance)> customLODSelector;
};

/**
 * @brief LOD System for optimizing large worlds
 * 
 * Features:
 * - Distance-based LOD selection
 * - Automatic culling of distant objects
 * - Per-entity LOD configuration
 * - Camera-relative distance calculations
 * - LOD transition callbacks
 * - Performance statistics
 * 
 * Usage:
 *   LODSystem lodSystem;
 *   lodSystem.RegisterEntity(entityID, position, lodConfig);
 *   lodSystem.UpdateLOD(cameraPosition);
 *   
 *   LODLevel level = lodSystem.GetEntityLOD(entityID);
 *   if (level != LODLevel::Off) {
 *       RenderEntity(entityID, level);
 *   }
 */
class LODSystem {
public:
    struct EntityLODData {
        Vector2 position;
        LODConfig config;
        LODLevel currentLOD = LODLevel::High;
        float lastDistance = 0.0f;
    };

    struct LODStats {
        uint32_t totalEntities = 0;
        uint32_t highDetail = 0;
        uint32_t mediumDetail = 0;
        uint32_t lowDetail = 0;
        uint32_t culled = 0;
        float averageDistance = 0.0f;
    };

    LODSystem() = default;
    ~LODSystem() = default;

    /**
     * @brief Register entity with LOD system
     */
    void RegisterEntity(uint32_t entityID, const Vector2& position, 
                       const LODConfig& config = LODConfig{}) {
        EntityLODData data;
        data.position = position;
        data.config = config;
        data.currentLOD = LODLevel::High;
        data.lastDistance = 0.0f;

        m_Entities[entityID] = data;
    }

    /**
     * @brief Unregister entity from LOD system
     */
    void UnregisterEntity(uint32_t entityID) {
        m_Entities.erase(entityID);
    }

    /**
     * @brief Update entity position (for moving objects)
     */
    void UpdateEntityPosition(uint32_t entityID, const Vector2& newPosition) {
        auto it = m_Entities.find(entityID);
        if (it != m_Entities.end()) {
            it->second.position = newPosition;
        }
    }

    /**
     * @brief Update LOD levels for all entities based on camera position
     * Call this every frame before rendering
     */
    void UpdateLOD(const Vector2& cameraPosition) {
        m_Stats = {}; // Reset stats

        for (auto& [entityID, data] : m_Entities) {
            // Calculate distance from camera
            float distance = CalculateDistance(cameraPosition, data.position);
            data.lastDistance = distance;

            // Determine LOD level
            LODLevel newLOD;
            if (data.config.customLODSelector) {
                newLOD = data.config.customLODSelector(distance);
            } else {
                newLOD = SelectLODLevel(distance, data.config);
            }

            // Check for LOD transition
            if (newLOD != data.currentLOD) {
                OnLODTransition(entityID, data.currentLOD, newLOD);
                data.currentLOD = newLOD;
            }

            // Update statistics
            UpdateStats(newLOD, distance);
        }

        // Finalize stats
        if (m_Stats.totalEntities > 0) {
            m_Stats.averageDistance /= m_Stats.totalEntities;
        }
    }

    /**
     * @brief Get current LOD level for entity
     */
    [[nodiscard]] LODLevel GetEntityLOD(uint32_t entityID) const {
        auto it = m_Entities.find(entityID);
        if (it != m_Entities.end()) {
            return it->second.currentLOD;
        }
        return LODLevel::Off;
    }

    /**
     * @brief Check if entity should be rendered (not culled)
     */
    [[nodiscard]] bool ShouldRender(uint32_t entityID) const {
        return GetEntityLOD(entityID) != LODLevel::Off;
    }

    /**
     * @brief Get last calculated distance for entity
     */
    [[nodiscard]] float GetEntityDistance(uint32_t entityID) const {
        auto it = m_Entities.find(entityID);
        if (it != m_Entities.end()) {
            return it->second.lastDistance;
        }
        return 0.0f;
    }

    /**
     * @brief Get all entities at specific LOD level
     */
    [[nodiscard]] std::vector<uint32_t> GetEntitiesAtLOD(LODLevel level) const {
        std::vector<uint32_t> result;
        for (const auto& [entityID, data] : m_Entities) {
            if (data.currentLOD == level) {
                result.push_back(entityID);
            }
        }
        return result;
    }

    /**
     * @brief Get performance statistics
     */
    [[nodiscard]] const LODStats& GetStats() const {
        return m_Stats;
    }

    /**
     * @brief Register callback for LOD transitions
     */
    void OnLODTransitionCallback(std::function<void(uint32_t, LODLevel, LODLevel)> callback) {
        m_OnLODTransition = callback;
    }

    /**
     * @brief Clear all entities
     */
    void Clear() {
        m_Entities.clear();
        m_Stats = {};
    }

    /**
     * @brief Get total entity count
     */
    [[nodiscard]] size_t GetEntityCount() const {
        return m_Entities.size();
    }

private:
    /**
     * @brief Calculate distance between two points
     */
    float CalculateDistance(const Vector2& a, const Vector2& b) const {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Select LOD level based on distance and config
     */
    LODLevel SelectLODLevel(float distance, const LODConfig& config) const {
        if (distance > config.distanceCull) {
            return LODLevel::Off;
        } else if (distance > config.distanceLow) {
            return LODLevel::Low;
        } else if (distance > config.distanceMedium) {
            return LODLevel::Medium;
        } else if (distance > config.distanceHigh) {
            return LODLevel::High;
        } else {
            return LODLevel::High;
        }
    }

    /**
     * @brief Handle LOD transition event
     */
    void OnLODTransition(uint32_t entityID, LODLevel oldLOD, LODLevel newLOD) {
        if (m_OnLODTransition) {
            m_OnLODTransition(entityID, oldLOD, newLOD);
        }
    }

    /**
     * @brief Update performance statistics
     */
    void UpdateStats(LODLevel level, float distance) {
        m_Stats.totalEntities++;
        m_Stats.averageDistance += distance;

        switch (level) {
            case LODLevel::High:
                m_Stats.highDetail++;
                break;
            case LODLevel::Medium:
                m_Stats.mediumDetail++;
                break;
            case LODLevel::Low:
                m_Stats.lowDetail++;
                break;
            case LODLevel::Off:
                m_Stats.culled++;
                break;
        }
    }

    std::unordered_map<uint32_t, EntityLODData> m_Entities;
    LODStats m_Stats;
    std::function<void(uint32_t, LODLevel, LODLevel)> m_OnLODTransition;
};

/**
 * @brief Helper: Render scale multiplier based on LOD level
 */
inline float GetLODScaleMultiplier(LODLevel level) {
    switch (level) {
        case LODLevel::High: return 1.0f;
        case LODLevel::Medium: return 0.75f;
        case LODLevel::Low: return 0.5f;
        case LODLevel::Off: return 0.0f;
        default: return 1.0f;
    }
}

/**
 * @brief Helper: Suggested texture resolution based on LOD
 */
inline uint32_t GetLODTextureResolution(LODLevel level, uint32_t baseResolution) {
    switch (level) {
        case LODLevel::High: return baseResolution;
        case LODLevel::Medium: return baseResolution / 2;
        case LODLevel::Low: return baseResolution / 4;
        case LODLevel::Off: return 0;
        default: return baseResolution;
    }
}

} // namespace SAGE
