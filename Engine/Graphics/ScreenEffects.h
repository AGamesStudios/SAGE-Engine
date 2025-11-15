#pragma once

#include "Math/Vector2.h"
#include <functional>
#include <cmath>

namespace SAGE {

/**
 * @brief Типы easing функций для эффектов
 */
enum class EaseType {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseOutElastic,
    EaseOutBounce
};

/**
 * @brief Easing функции
 */
class Easing {
public:
    static float Evaluate(EaseType type, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        
        switch (type) {
            case EaseType::Linear:
                return t;
                
            case EaseType::EaseInQuad:
                return t * t;
                
            case EaseType::EaseOutQuad:
                return t * (2.0f - t);
                
            case EaseType::EaseInOutQuad:
                return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
                
            case EaseType::EaseInCubic:
                return t * t * t;
                
            case EaseType::EaseOutCubic: {
                float f = t - 1.0f;
                return f * f * f + 1.0f;
            }
                
            case EaseType::EaseInOutCubic:
                return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
                
            case EaseType::EaseOutElastic: {
                if (t == 0.0f || t == 1.0f) return t;
                float p = 0.3f;
                return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * 3.14159f) / p) + 1.0f;
            }
                
            case EaseType::EaseOutBounce: {
                if (t < 1.0f / 2.75f) {
                    return 7.5625f * t * t;
                } else if (t < 2.0f / 2.75f) {
                    t -= 1.5f / 2.75f;
                    return 7.5625f * t * t + 0.75f;
                } else if (t < 2.5f / 2.75f) {
                    t -= 2.25f / 2.75f;
                    return 7.5625f * t * t + 0.9375f;
                } else {
                    t -= 2.625f / 2.75f;
                    return 7.5625f * t * t + 0.984375f;
                }
            }
                
            default:
                return t;
        }
    }
};

/**
 * @brief Camera Shake эффект
 */
struct CameraShake {
    float duration = 0.0f;          ///< Длительность тряски
    float intensity = 1.0f;         ///< Интенсивность (амплитуда)
    float frequency = 20.0f;        ///< Частота колебаний
    EaseType damping = EaseType::EaseOutQuad;  ///< Затухание
    
    // Runtime состояние
    float timer = 0.0f;
    Vector2 offset = Vector2::Zero();
    bool active = false;
    
    CameraShake() = default;
    
    CameraShake(float dur, float intens, float freq = 20.0f)
        : duration(dur), intensity(intens), frequency(freq) {}
    
    /**
     * @brief Запустить тряску
     */
    void Start() {
        timer = 0.0f;
        active = true;
    }
    
    /**
     * @brief Обновить тряску
     */
    void Update(float deltaTime) {
        if (!active) return;
        
        timer += deltaTime;
        
        if (timer >= duration) {
            active = false;
            offset = Vector2::Zero();
            return;
        }
        
        // Прогресс (0 -> 1)
        float progress = timer / duration;
        
        // Затухание
        float dampingFactor = 1.0f - Easing::Evaluate(damping, progress);
        
        // Случайное смещение с частотой
        float angle = timer * frequency * 6.28318f;  // 2*PI
        float randomX = std::sin(angle) * intensity * dampingFactor;
        float randomY = std::cos(angle * 1.3f) * intensity * dampingFactor;
        
        offset = Vector2(randomX, randomY);
    }
    
    /**
     * @brief Остановить тряску
     */
    void Stop() {
        active = false;
        offset = Vector2::Zero();
    }
    
    bool IsActive() const { return active; }
    Vector2 GetOffset() const { return offset; }
};

/**
 * @brief Screen Flash эффект (вспышка экрана)
 */
struct ScreenFlash {
    float duration = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float maxAlpha = 1.0f;
    EaseType fadeType = EaseType::EaseOutQuad;
    
    // Runtime
    float timer = 0.0f;
    float currentAlpha = 0.0f;
    bool active = false;
    
    ScreenFlash() = default;
    
    ScreenFlash(float dur, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
        : duration(dur), r(red), g(green), b(blue), maxAlpha(alpha) {}
    
    /**
     * @brief Запустить вспышку
     */
    void Start() {
        timer = 0.0f;
        currentAlpha = maxAlpha;
        active = true;
    }
    
    /**
     * @brief Обновить вспышку
     */
    void Update(float deltaTime) {
        if (!active) return;
        
        timer += deltaTime;
        
        if (timer >= duration) {
            active = false;
            currentAlpha = 0.0f;
            return;
        }
        
        float progress = timer / duration;
        currentAlpha = maxAlpha * (1.0f - Easing::Evaluate(fadeType, progress));
    }
    
    void Stop() {
        active = false;
        currentAlpha = 0.0f;
    }
    
    bool IsActive() const { return active; }
    float GetAlpha() const { return currentAlpha; }
};

/**
 * @brief Screen Transition эффект
 */
enum class TransitionType {
    Fade,           ///< Плавное затемнение
    Wipe,           ///< Смахивание
    Circle,         ///< Круговой переход
    Pixelate,       ///< Пикселизация
    Custom          ///< Кастомный shader
};

struct ScreenTransition {
    TransitionType type = TransitionType::Fade;
    float duration = 1.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    EaseType easing = EaseType::EaseInOutQuad;
    
    // Runtime
    float timer = 0.0f;
    float progress = 0.0f;
    bool active = false;
    bool fadingOut = true;  ///< true = fade out, false = fade in
    std::function<void()> onComplete;
    
    ScreenTransition() = default;
    
    ScreenTransition(TransitionType t, float dur, bool fadeOut = true)
        : type(t), duration(dur), fadingOut(fadeOut) {}
    
    /**
     * @brief Запустить переход
     */
    void Start(bool fadeOut = true) {
        timer = 0.0f;
        progress = 0.0f;
        active = true;
        fadingOut = fadeOut;
    }
    
    /**
     * @brief Обновить переход
     */
    void Update(float deltaTime) {
        if (!active) return;
        
        timer += deltaTime;
        
        if (timer >= duration) {
            active = false;
            progress = 1.0f;
            
            if (onComplete) {
                onComplete();
            }
            return;
        }
        
        float t = timer / duration;
        progress = Easing::Evaluate(easing, t);
        
        if (!fadingOut) {
            progress = 1.0f - progress;
        }
    }
    
    void Stop() {
        active = false;
    }
    
    bool IsActive() const { return active; }
    float GetProgress() const { return progress; }
};

} // namespace SAGE
