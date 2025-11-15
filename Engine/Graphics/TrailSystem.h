#pragma once

#include "Math/Vector2.h"
#include "Core/Color.h"
#include <vector>
#include <memory>

namespace SAGE {

class Texture;

/**
 * @brief Точка следа (trail point)
 */
struct TrailPoint {
    Vector2 position;
    float lifetime = 0.0f;      ///< Время жизни этой точки
    float width = 1.0f;         ///< Ширина в этой точке
    float alpha = 1.0f;         ///< Прозрачность
    
    TrailPoint() = default;
    TrailPoint(const Vector2& pos, float life, float w, float a)
        : position(pos), lifetime(life), width(w), alpha(a) {}
};

/**
 * @brief Motion Trail (след за объектом)
 */
class MotionTrail {
public:
    // Настройки
    float pointLifetime = 0.5f;         ///< Время жизни каждой точки
    float emissionRate = 0.05f;         ///< Интервал создания точек (секунды)
    float startWidth = 10.0f;           ///< Начальная ширина
    float endWidth = 2.0f;              ///< Конечная ширина
    Color startColor = Color::White();  ///< Начальный цвет
    Color endColor = Color::White();    ///< Конечный цвет
    float startAlpha = 1.0f;            ///< Начальная прозрачность
    float endAlpha = 0.0f;              ///< Конечная прозрачность
    int maxPoints = 50;                 ///< Максимум точек
    
    // Текстура для trail
    std::shared_ptr<Texture> texture;
    
    // Runtime
    std::vector<TrailPoint> points;
    float emissionTimer = 0.0f;
    bool active = true;
    
    MotionTrail() = default;
    
    /**
     * @brief Обновить trail
     */
    void Update(float deltaTime, const Vector2& currentPosition) {
        if (!active) return;
        
        // Обновить существующие точки
        for (auto it = points.begin(); it != points.end();) {
            it->lifetime -= deltaTime;
            
            if (it->lifetime <= 0.0f) {
                it = points.erase(it);
            } else {
                // Интерполировать ширину и прозрачность
                float lifePercent = it->lifetime / pointLifetime;
                it->width = endWidth + (startWidth - endWidth) * lifePercent;
                it->alpha = endAlpha + (startAlpha - endAlpha) * lifePercent;
                ++it;
            }
        }
        
        // Создать новую точку
        emissionTimer += deltaTime;
        if (emissionTimer >= emissionRate) {
            emissionTimer = 0.0f;
            
            if (static_cast<int>(points.size()) < maxPoints) {
                points.emplace_back(currentPosition, pointLifetime, startWidth, startAlpha);
            }
        }
    }
    
    /**
     * @brief Очистить все точки
     */
    void Clear() {
        points.clear();
        emissionTimer = 0.0f;
    }
    
    /**
     * @brief Получить точки для рендеринга
     */
    const std::vector<TrailPoint>& GetPoints() const { return points; }
    
    /**
     * @brief Проверить, активен ли trail
     */
    bool IsActive() const { return active; }
    
    /**
     * @brief Включить/выключить trail
     */
    void SetActive(bool isActive) { 
        active = isActive;
        if (!active) {
            Clear();
        }
    }
};

/**
 * @brief Dash Effect (эффект рывка с последовательными "призраками")
 */
struct DashGhost {
    Vector2 position;
    float rotation = 0.0f;
    float lifetime = 0.0f;
    float alpha = 1.0f;
    Vector2 scale = Vector2(1.0f, 1.0f);
};

class DashEffect {
public:
    // Настройки
    float ghostLifetime = 0.3f;     ///< Время жизни каждого "призрака"
    float ghostInterval = 0.05f;    ///< Интервал создания призраков
    int maxGhosts = 10;             ///< Максимум призраков
    Color ghostColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
    
    // Runtime
    std::vector<DashGhost> ghosts;
    float ghostTimer = 0.0f;
    bool active = false;
    
    DashEffect() = default;
    
    /**
     * @brief Начать dash эффект
     */
    void Start() {
        active = true;
        ghosts.clear();
        ghostTimer = 0.0f;
    }
    
    /**
     * @brief Обновить эффект
     */
    void Update(float deltaTime, const Vector2& position, float rotation, const Vector2& scale) {
        if (!active) return;
        
        // Обновить существующие призраки
        for (auto it = ghosts.begin(); it != ghosts.end();) {
            it->lifetime -= deltaTime;
            
            if (it->lifetime <= 0.0f) {
                it = ghosts.erase(it);
            } else {
                // Fade out
                it->alpha = it->lifetime / ghostLifetime;
                ++it;
            }
        }
        
        // Создать новый призрак
        ghostTimer += deltaTime;
        if (ghostTimer >= ghostInterval && static_cast<int>(ghosts.size()) < maxGhosts) {
            ghostTimer = 0.0f;
            
            DashGhost ghost;
            ghost.position = position;
            ghost.rotation = rotation;
            ghost.lifetime = ghostLifetime;
            ghost.alpha = 1.0f;
            ghost.scale = scale;
            
            ghosts.push_back(ghost);
        }
    }
    
    /**
     * @brief Остановить эффект
     */
    void Stop() {
        active = false;
    }
    
    /**
     * @brief Получить призраков
     */
    const std::vector<DashGhost>& GetGhosts() const { return ghosts; }
    
    bool IsActive() const { return active; }
};

} // namespace SAGE
