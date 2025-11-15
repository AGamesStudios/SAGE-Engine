#pragma once

#include "Graphics/TrailSystem.h"
#include "Math/Vector2.h"

namespace SAGE::ECS {

/**
 * @brief Компонент для motion trail и dash эффектов
 */
struct TrailComponent {
    MotionTrail trail;
    DashEffect dashEffect;
    
    // Настройки
    bool enableTrail = false;       ///< Постоянный trail
    bool enableDashEffect = false;  ///< Dash effect (по требованию)
    
    TrailComponent() = default;
    
    /**
     * @brief Включить постоянный trail
     */
    void EnableTrail(bool enable = true) {
        enableTrail = enable;
        trail.SetActive(enable);
    }
    
    /**
     * @brief Начать dash эффект
     */
    void StartDash() {
        enableDashEffect = true;
        dashEffect.Start();
    }
    
    /**
     * @brief Остановить dash эффект
     */
    void StopDash() {
        enableDashEffect = false;
        dashEffect.Stop();
    }
    
    /**
     * @brief Настроить trail
     */
    void SetupTrail(float pointLife, float emissionRate, float startWidth, float endWidth) {
        trail.pointLifetime = pointLife;
        trail.emissionRate = emissionRate;
        trail.startWidth = startWidth;
        trail.endWidth = endWidth;
    }
    
    /**
     * @brief Настроить dash эффект
     */
    void SetupDash(float ghostLife, float interval, int maxGhosts) {
        dashEffect.ghostLifetime = ghostLife;
        dashEffect.ghostInterval = interval;
        dashEffect.maxGhosts = maxGhosts;
    }
};

} // namespace SAGE::ECS
