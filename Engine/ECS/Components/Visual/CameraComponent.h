#pragma once

/**
 * @file CameraComponent.h
 * @brief ECS компонент камеры на основе Camera2D
 */

#include "Graphics/Core/Camera2D.h"

namespace SAGE::ECS {

/// @brief Компонент камеры для ECS системы
/// Обертка над Camera2D для использования в Entity-Component системе
struct CameraComponent {
    Camera2D camera;                    ///< Экземпляр 2D камеры
    bool isPrimary = false;             ///< Является ли основной камерой сцены
    int renderOrder = 0;                ///< Порядок рендеринга (меньше = раньше)
    
    CameraComponent() = default;
    
    explicit CameraComponent(float viewportWidth, float viewportHeight, bool primary = false)
        : camera(viewportWidth, viewportHeight)
        , isPrimary(primary)
    {}
    
    /// @brief Получить матрицу view-projection
    Matrix4 GetViewProjectionMatrix() const {
        return camera.GetViewProjectionMatrix();
    }
    
    /// @brief Обновить размер viewport
    void SetViewport(float width, float height) {
        camera.SetViewportSize(width, height);
    }
    
    /// @brief Получить позицию камеры
    Vector2 GetPosition() const {
        return camera.GetPosition();
    }
    
    /// @brief Установить позицию камеры
    void SetPosition(const Vector2& pos) {
        camera.SetPosition(pos);
    }
    
    /// @brief Получить зум камеры
    float GetZoom() const {
        return camera.GetZoom();
    }
    
    /// @brief Установить зум камеры
    void SetZoom(float zoom) {
        camera.SetZoom(zoom);
    }
};

} // namespace SAGE::ECS
