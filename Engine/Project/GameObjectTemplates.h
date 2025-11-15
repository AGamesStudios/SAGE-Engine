#pragma once

#include "Core/Core.h"
#include "ECS/Registry.h"
#include "ECS/Entity.h"
#include "Math/Vector2.h"
#include "Core/Color.h"
#include <string>

namespace SAGE {

/// @brief GameObject Templates - готовые шаблоны для быстрого создания объектов
/// Упрощает создание игр для новичков
class GameObjectTemplates {
public:
    
    /// @brief Создать камеру (ОБЯЗАТЕЛЬНО для отображения!)
    /// @param registry Реестр ECS
    /// @param position Позиция камеры
    /// @param width Ширина видимой области
    /// @param height Высота видимой области
    /// @return Entity ID камеры
    static ECS::Entity CreateCamera(
        ECS::Registry& registry,
        const Vector2& position = Vector2::Zero(),
        float width = 1280.0f,
        float height = 720.0f,
        bool isMain = true
    );
    
    /// @brief Создать игрока (для платформера)
    /// @param registry Реестр ECS
    /// @param position Начальная позиция
    /// @param size Размер игрока
    /// @return Entity ID игрока
    static ECS::Entity CreatePlayer(
        ECS::Registry& registry,
        const Vector2& position = Vector2::Zero(),
        const Vector2& size = Vector2(32, 32)
    );
    
    /// @brief Создать платформу / землю
    /// @param registry Реестр ECS
    /// @param position Позиция
    /// @param size Размер
    /// @return Entity ID платформы
    static ECS::Entity CreatePlatform(
        ECS::Registry& registry,
        const Vector2& position,
        const Vector2& size
    );
    
    /// @brief Создать врага
    /// @param registry Реестр ECS
    /// @param position Начальная позиция
    /// @param size Размер
    /// @return Entity ID врага
    static ECS::Entity CreateEnemy(
        ECS::Registry& registry,
        const Vector2& position,
        const Vector2& size = Vector2(32, 32)
    );
    
    /// @brief Создать предмет для сбора (монета, звезда и т.д.)
    /// @param registry Реестр ECS
    /// @param position Позиция
    /// @return Entity ID предмета
    static ECS::Entity CreateCollectible(
        ECS::Registry& registry,
        const Vector2& position
    );
    
    /// @brief Создать спрайт (простой визуальный объект)
    /// @param registry Реестр ECS
    /// @param position Позиция
    /// @param size Размер
    /// @param color Цвет
    /// @param texturePath Путь к текстуре (опционально)
    /// @return Entity ID спрайта
    static ECS::Entity CreateSprite(
        ECS::Registry& registry,
        const Vector2& position,
        const Vector2& size,
        const Color& color = Color::White(),
        const std::string& texturePath = ""
    );
    
    /// @brief Создать UI текст
    /// @param registry Реестр ECS
    /// @param text Текст
    /// @param position Позиция на экране
    /// @param fontSize Размер шрифта
    /// @return Entity ID текста
    static ECS::Entity CreateText(
        ECS::Registry& registry,
        const std::string& text,
        const Vector2& position,
        float fontSize = 24.0f
    );
    
    /// @brief Создать фон (background)
    /// @param registry Реестр ECS
    /// @param texturePath Путь к текстуре фона
    /// @param layer Слой отрисовки (отрицательный = на заднем плане)
    /// @return Entity ID фона
    static ECS::Entity CreateBackground(
        ECS::Registry& registry,
        const std::string& texturePath,
        int layer = -100
    );
    
    /// @brief Создать кнопку UI
    /// @param registry Реестр ECS
    /// @param text Текст кнопки
    /// @param position Позиция
    /// @param size Размер
    /// @return Entity ID кнопки
    static ECS::Entity CreateButton(
        ECS::Registry& registry,
        const std::string& text,
        const Vector2& position,
        const Vector2& size = Vector2(200, 50)
    );
    
    /// @brief Создать частицы (эффект)
    /// @param registry Реестр ECS
    /// @param position Позиция эмиттера
    /// @param particleCount Количество частиц
    /// @return Entity ID системы частиц
    static ECS::Entity CreateParticleEffect(
        ECS::Registry& registry,
        const Vector2& position,
        int particleCount = 100
    );
};

} // namespace SAGE
