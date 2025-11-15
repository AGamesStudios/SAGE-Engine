#pragma once

#include "Math/Vector2.h"

namespace SAGE::ECS {

/// @brief Режим движения игрока
enum class MovementMode {
    Platformer,     ///< Платформер (сайд-скроллер с прыжками)
    TopDown         ///< Вид сверху (8-направленное движение)
};

/// @brief Компонент движения игрока с настройками для платформера и вид сверху
/// Поддерживает прыжки, бег, ускорение, дэш, скольжение по стенам
struct PlayerMovementComponent {
    // ===== РЕЖИМ ДВИЖЕНИЯ =====
    MovementMode mode = MovementMode::Platformer;
    
    // ===== БАЗОВАЯ СКОРОСТЬ =====
    float moveSpeed = 200.0f;               ///< Базовая скорость движения (пиксели/сек)
    float sprintMultiplier = 1.8f;          ///< Множитель скорости при беге
    bool canSprint = true;                  ///< Можно ли бегать
    
    // ===== УСКОРЕНИЕ И ТОРМОЖЕНИЕ =====
    float acceleration = 2000.0f;           ///< Ускорение при движении (px/s²)
    float deceleration = 2500.0f;           ///< Торможение при остановке (px/s²)
    float airAcceleration = 1000.0f;        ///< Ускорение в воздухе (для платформера)
    float airDeceleration = 500.0f;         ///< Торможение в воздухе
    
    // ===== ПРЫЖКИ (Platformer) =====
    float jumpForce = 400.0f;               ///< Сила прыжка
    float jumpHoldGravityMultiplier = 0.5f; ///< Множитель гравитации при удержании прыжка
    float fallGravityMultiplier = 1.5f;     ///< Множитель гравитации при падении
    float maxFallSpeed = 600.0f;            ///< Максимальная скорость падения
    int maxJumps = 1;                       ///< Максимальное количество прыжков (1 = обычный, 2+ = двойной/тройной)
    float coyoteTime = 0.1f;                ///< Время "койота" (прыжок после схода с платформы)
    float jumpBufferTime = 0.1f;            ///< Время буфера прыжка (нажатие до касания земли)
    bool variableJumpHeight = true;         ///< Переменная высота прыжка (отпускание кнопки = короткий прыжок)
    
    // ===== WALL JUMP / WALL SLIDE (Platformer) =====
    bool canWallSlide = false;              ///< Может ли скользить по стенам
    float wallSlideSpeed = 100.0f;          ///< Скорость скольжения по стене
    bool canWallJump = false;               ///< Может ли прыгать от стены
    float wallJumpForce = 350.0f;           ///< Сила прыжка от стены
    Vector2 wallJumpDirection = Vector2(1.0f, -1.5f); ///< Направление прыжка от стены (нормализуется)
    float wallJumpLockTime = 0.2f;          ///< Время блокировки управления после wall jump
    
    // ===== DASH =====
    bool canDash = false;                   ///< Может ли делать дэш
    float dashSpeed = 600.0f;               ///< Скорость дэша
    float dashDuration = 0.2f;              ///< Длительность дэша
    float dashCooldown = 1.0f;              ///< Время перезарядки дэша
    bool canAirDash = false;                ///< Можно ли дэшить в воздухе
    int maxAirDashes = 1;                   ///< Максимальное количество дэшей в воздухе
    
    // ===== TOP-DOWN SPECIFIC =====
    bool enable8Direction = true;           ///< 8-направленное движение (иначе 4-направленное)
    bool normalizeDiagonal = true;          ///< Нормализовать диагональное движение
    float rotationSpeed = 720.0f;           ///< Скорость поворота персонажа (градусы/сек, 0 = мгновенный)
    bool rotateToMovement = false;          ///< Поворачивать спрайт по направлению движения
    
    // ===== СОСТОЯНИЕ (runtime) =====
    Vector2 velocity = Vector2::Zero();     ///< Текущая скорость
    Vector2 inputDirection = Vector2::Zero(); ///< Направление ввода (-1 до 1)
    bool isGrounded = false;                ///< На земле ли игрок
    bool isTouchingWall = false;            ///< Касается ли стены (для wall slide)
    int wallDirection = 0;                  ///< Направление стены (-1 = слева, 1 = справа)
    int jumpsRemaining = 1;                 ///< Оставшиеся прыжки
    float coyoteTimer = 0.0f;               ///< Таймер койота
    float jumpBufferTimer = 0.0f;           ///< Таймер буфера прыжка
    bool isJumpHeld = false;                ///< Удерживается ли кнопка прыжка
    bool isDashing = false;                 ///< В процессе дэша
    float dashTimer = 0.0f;                 ///< Таймер дэша
    float dashCooldownTimer = 0.0f;         ///< Таймер перезарядки дэша
    int dashesRemaining = 1;                ///< Оставшиеся дэши в воздухе
    Vector2 dashDirection = Vector2::Zero(); ///< Направление дэша
    float wallJumpLockTimer = 0.0f;         ///< Таймер блокировки после wall jump
    bool isSprinting = false;               ///< Бежит ли игрок
    
    // ===== МЕТОДЫ =====
    
    /// @brief Установить режим платформера
    void SetPlatformerMode() {
        mode = MovementMode::Platformer;
        // Рекомендуемые настройки для платформера
        moveSpeed = 200.0f;
        acceleration = 2000.0f;
        deceleration = 2500.0f;
        jumpForce = 400.0f;
    }
    
    /// @brief Установить режим вид сверху
    void SetTopDownMode() {
        mode = MovementMode::TopDown;
        // Рекомендуемые настройки для top-down
        moveSpeed = 150.0f;
        acceleration = 1500.0f;
        deceleration = 2000.0f;
        enable8Direction = true;
        normalizeDiagonal = true;
    }
    
    /// @brief Включить wall jump функциональность
    void EnableWallJump() {
        canWallSlide = true;
        canWallJump = true;
    }
    
    /// @brief Включить дэш
    void EnableDash(bool allowAirDash = false) {
        canDash = true;
        canAirDash = allowAirDash;
    }
    
    /// @brief Сбросить состояние при касании земли
    void OnLanded() {
        isGrounded = true;
        jumpsRemaining = maxJumps;
        dashesRemaining = maxAirDashes;
        coyoteTimer = coyoteTime;
    }
    
    /// @brief Сбросить состояние при уходе с земли
    void OnLeftGround() {
        isGrounded = false;
        if (jumpsRemaining == maxJumps) {
            jumpsRemaining--; // Используем один прыжок за счёт схода с платформы
        }
    }
    
    /// @brief Инициировать прыжок
    bool TryJump() {
        // Проверка буфера прыжка или койота
        bool canJump = (isGrounded || coyoteTimer > 0.0f || jumpsRemaining > 0);
        
        if (canJump) {
            if (isGrounded || coyoteTimer > 0.0f) {
                jumpsRemaining = maxJumps - 1; // Используем один прыжок
            } else {
                jumpsRemaining--; // Двойной/тройной прыжок
            }
            
            coyoteTimer = 0.0f;
            jumpBufferTimer = 0.0f;
            isJumpHeld = true;
            return true;
        }
        
        // Буфер прыжка
        jumpBufferTimer = jumpBufferTime;
        return false;
    }
    
    /// @brief Инициировать wall jump
    bool TryWallJump() {
        if (canWallJump && isTouchingWall && !isGrounded) {
            wallJumpLockTimer = wallJumpLockTime;
            jumpsRemaining = maxJumps - 1;
            return true;
        }
        return false;
    }
    
    /// @brief Инициировать дэш
    bool TryDash(const Vector2& direction) {
        if (!canDash || dashCooldownTimer > 0.0f) {
            return false;
        }
        
        if (!isGrounded && !canAirDash) {
            return false;
        }
        
        if (!isGrounded && dashesRemaining <= 0) {
            return false;
        }
        
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        dashDirection = direction.Normalized();
        
        if (!isGrounded) {
            dashesRemaining--;
        }
        
        return true;
    }
    
    /// @brief Получить текущую максимальную скорость
    float GetCurrentMaxSpeed() const {
        if (isDashing) {
            return dashSpeed;
        }
        return isSprinting && canSprint ? moveSpeed * sprintMultiplier : moveSpeed;
    }
    
    /// @brief Получить текущее ускорение
    float GetCurrentAcceleration() const {
        return isGrounded ? acceleration : airAcceleration;
    }
    
    /// @brief Получить текущее торможение
    float GetCurrentDeceleration() const {
        return isGrounded ? deceleration : airDeceleration;
    }
};

} // namespace SAGE::ECS
