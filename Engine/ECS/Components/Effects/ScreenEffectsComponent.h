#pragma once

#include "Graphics/ScreenEffects.h"
#include "Math/Vector2.h"

namespace SAGE::ECS {

/**
 * @brief Компонент для screen effects (shake, flash, transitions)
 */
struct ScreenEffectsComponent {
    CameraShake shake;
    ScreenFlash flash;
    ScreenTransition transition;
    
    // Настройки
    bool enableShake = true;
    bool enableFlash = true;
    bool enableTransition = true;
    
    ScreenEffectsComponent() = default;
    
    /**
     * @brief Тряска камеры
     */
    void Shake(float duration, float intensity, float frequency = 20.0f) {
        if (!enableShake) return;
        shake = CameraShake(duration, intensity, frequency);
        shake.Start();
    }
    
    /**
     * @brief Вспышка экрана
     */
    void Flash(float duration, float r = 1.0f, float g = 1.0f, float b = 1.0f, float alpha = 1.0f) {
        if (!enableFlash) return;
        flash = ScreenFlash(duration, r, g, b, alpha);
        flash.Start();
    }
    
    /**
     * @brief Переход (fade out)
     */
    void FadeOut(float duration, std::function<void()> callback = nullptr) {
        if (!enableTransition) return;
        transition = ScreenTransition(TransitionType::Fade, duration, true);
        transition.onComplete = callback;
        transition.Start(true);
    }
    
    /**
     * @brief Переход (fade in)
     */
    void FadeIn(float duration, std::function<void()> callback = nullptr) {
        if (!enableTransition) return;
        transition = ScreenTransition(TransitionType::Fade, duration, false);
        transition.onComplete = callback;
        transition.Start(false);
    }
    
    /**
     * @brief Получить смещение камеры от shake
     */
    Vector2 GetCameraOffset() const {
        return shake.IsActive() ? shake.GetOffset() : Vector2::Zero();
    }
};

} // namespace SAGE::ECS
