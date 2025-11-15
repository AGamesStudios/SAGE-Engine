#pragma once

#include "ECS/Components/TilemapComponent.h"

namespace SAGE {

    /**
     * @brief TilemapEditor - ImGui инструмент для редактирования tilemap слоёв
     * 
     * Функционал:
     * - Включение/выключение видимости слоёв
     * - Настройка параллакса
     * - Управление opacity
     * - Пометка collision слоёв
     */
    class TilemapEditor {
    public:
        /**
         * @brief Отображает ImGui окно редактора для tilemap компонента
         * @param tilemap Редактируемый компонент
         * @param entityName Название entity для заголовка окна
         */
        static void ShowEditor(ECS::TilemapComponent& tilemap, const char* entityName = "Tilemap");

        /**
         * @brief Отображает контролы для отдельного слоя
         * @param layer Редактируемый слой
         * @param layerIndex Индекс слоя
         */
        static void ShowLayerControls(ECS::TilemapLayer& layer, int layerIndex);

        /**
         * @brief Показывает статистику tilemap (размер, слои, тайлы)
         */
        static void ShowStats(const ECS::TilemapComponent& tilemap);
    };

} // namespace SAGE
