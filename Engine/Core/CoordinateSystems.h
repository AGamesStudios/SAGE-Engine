/**
 * @file CoordinateSystems.h
 * @brief Документация систем координат в SAGE Engine
 * 
 * SAGE Engine использует две основные системы координат:
 * 
 * 1. SCREEN (Экранные координаты)
 * 2. WORLD (Мировые координаты)
 * 
 * Понимание различий между системами критически важно для корректной работы.
 */

#pragma once

namespace SAGE {

/**
 * @brief Типы координатных систем, используемых в движке
 */
enum class CoordinateSpace {
    /**
     * @brief SCREEN - Экранные/Viewport координаты
     * 
     * Характеристики:
     * - Origin: TOP-LEFT (0,0) в левом верхнем углу окна
     * - X-ось: направлена ВПРАВО → (увеличивается слева направо)
     * - Y-ось: направлена ВНИЗ ↓ (увеличивается сверху вниз)
     * - Единицы: ПИКСЕЛИ
     * - Диапазон: [0,0] до [viewportWidth, viewportHeight]
     * 
     * Используется в:
     * - Renderer (OpenGL projection с (0,0) = top-left)
     * - UI система (ImGui, button positions, etc)
     * - Camera2D.position
     * - Sprite positions
     * - Mouse/Input coordinates
     * - Window events (WindowResizeEvent)
     * 
     * Пример:
     * @code
     * // Точка в центре экрана 1280x720:
     * Vector2 screenCenter(640.0f, 360.0f); // X=640, Y=360
     * 
     * // Точка в левом верхнем углу:
     * Vector2 topLeft(0.0f, 0.0f);
     * 
     * // Точка в правом нижнем углу:
     * Vector2 bottomRight(1280.0f, 720.0f);
     * @endcode
     * 
     * @note Эта система соответствует DirectX, Unity UI, ImGui координатам
     */
    Screen,

    /**
     * @brief WORLD - Мировые игровые координаты
     * 
     * Характеристики:
     * - Origin: Определяется CAMERA position
     * - X-ось: направлена ВПРАВО →
     * - Y-ось: направлена ВНИЗ ↓ (как Screen)
     * - Единицы: ПИКСЕЛИ (как Screen)
     * - Диапазон: Неограничен (зависит от размера уровня)
     * 
     * Используется в:
     * - GameObject positions в мире
     * - Transform2D.position
     * - Tile maps, background layers
     * - Game entities позиционирование
     * 
     * Связь с Screen координатами:
     * @code
     * // Конверсия World → Screen учитывает камеру:
     * Vector2 screenPos = worldPos - camera.position;
     * screenPos.x *= camera.zoom;
     * screenPos.y *= camera.zoom;
     * // + rotation transform если camera.rotation != 0
     * 
     * // Конверсия Screen → World:
     * Vector2 worldPos = screenPos;
     * worldPos.x /= camera.zoom;
     * worldPos.y /= camera.zoom;
     * worldPos += camera.position;
     * @endcode
     * 
     * Пример:
     * @code
     * // Игрок на позиции в мире:
     * Transform2D playerTransform;
     * playerTransform.position = Vector2(5000.0f, 3000.0f); // WORLD coords
     * 
     * // Камера следует за игроком:
     * camera.position = playerTransform.position; // Camera в WORLD coords
     * @endcode
     * 
     * @note World использует ту же ориентацию осей что и Screen (Y-down)
     */
    World
};

/**
 * @brief Утилиты для работы с координатными системами
 * 
 * @note Для конверсии между системами используйте:
 * - CoordinateTransformer::WorldToScreen() / ScreenToWorld() - World ↔ Screen
 * - Camera2D методы для camera transformations
 */
namespace CoordinateSystemInfo {
    
    /**
     * @brief Возвращает человеко-читаемое название системы координат
     */
    inline const char* GetName(CoordinateSpace space) {
        switch(space) {
            case CoordinateSpace::Screen: return "Screen (Y-down, pixels)";
            case CoordinateSpace::World: return "World (Y-down, pixels)";
            default: return "Unknown";
        }
    }
    
    /**
     * @brief Проверяет, направлена ли Y-ось вниз в данной системе
     */
    inline bool IsYAxisDown(CoordinateSpace space) {
        return space == CoordinateSpace::Screen || space == CoordinateSpace::World;
    }
    
    /**
     * @brief Проверяет, использует ли система координат пиксели как единицы измерения
     */
    inline bool UsesPixels(CoordinateSpace space) {
        return space == CoordinateSpace::Screen || space == CoordinateSpace::World;
    }
    
    /**
     * @brief Проверяет, использует ли система координат метры как единицы измерения
     */
    inline bool UsesMeters(CoordinateSpace space) {
        return false;
    }

} // namespace CoordinateSystemInfo

} // namespace SAGE

/**
 * @page coordinate_systems Системы координат в SAGE Engine
 * 
 * @section coord_overview Обзор
 * 
 * SAGE Engine использует две системы координат:
 * 
 * | Система  | Origin       | Y-ось | Единицы | Где используется           |
 * |----------|--------------|-------|---------|----------------------------|
 * | SCREEN   | Top-Left     | ↓ Down| Pixels  | Renderer, UI, Camera       |
 * | WORLD    | Camera-based | ↓ Down| Pixels  | GameObjects, Transforms    |
 * 
 * @section coord_screen Screen Coordinates
 * 
 * Экранные координаты начинаются в левом верхнем углу viewport (0,0) и увеличиваются вправо и вниз.
 * Это стандарт для большинства UI frameworks (ImGui, DirectX, Unity UI).
 * 
 * @section coord_world World Coordinates
 * 
 * Мировые координаты используют те же оси что и Screen, но смещены на camera.position.
 * Zoom и rotation камеры также влияют на трансформацию World ↔ Screen.
 * 
 * @section coord_conversion Конверсия координат
 * 
 * Используйте специализированные функции для конверсии:
 * 
 * @code
 * // World ↔ Screen:
 * Vector2 screenPos = CoordinateTransformer::WorldToScreen(worldPos, camera);
 * Vector2 worldPos = CoordinateTransformer::ScreenToWorld(screenPos, camera);
 * @endcode
 * 
 * @warning НЕ делайте ручную конверсию! Всегда используйте утилиты конверсии.
 * 
 * @see CoordinateTransformer
 * @see Camera2D
 */
