#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Core/Color.h"

namespace SAGE {
    class Camera2D;
    class Texture;
}

namespace SAGE::ECS {
    struct TilemapComponent;
}

namespace SAGE {

    /**
     * @brief TilemapRenderer - рендерит тайловые карты с view frustum culling
     * 
     * Оптимизации:
     * - Отрисовка только видимых тайлов в пределах камеры
     * - Batch submission через BatchRenderer
     * - Опционально: per-layer VBO caching (TODO)
     * - Поддержка нескольких tileset'ов через GID ranges
     */
    class TilemapRenderer {
    public:
        /**
         * @brief Отрисовывает tilemap компонент с учётом камеры
         * @param tilemap Компонент тайловой карты
         * @param transform Трансформ entity (позиция/масштаб/поворот)
         * @param camera Камера для frustum culling
         */
        static void Render(const ECS::TilemapComponent& tilemap, const Float2& position, const Camera2D& camera);

        /**
         * @brief Отрисовывает отдельный тайловый слой (legacy helper)
         * @param tilemap Компонент тайловой карты
         * @param layerIndex Индекс слоя
         * @param position Позиция тайловой карты в мире
         * @param camera Камера для culling
         */
        static void RenderLayer(const ECS::TilemapComponent& tilemap, int layerIndex, const Float2& position, const Camera2D& camera);

        /**
         * @brief Вычисляет видимый диапазон тайлов для слоя
         * @param tilemap Компонент тайловой карты
         * @param layerIndex Индекс слоя
         * @param position Позиция тайловой карты в мире
         * @param camera Камера
         * @param outMinX Выходной минимальный X индекс тайла
         * @param outMinY Выходной минимальный Y индекс тайла
         * @param outMaxX Выходной максимальный X индекс тайла
         * @param outMaxY Выходной максимальный Y индекс тайла
         */
        static void ComputeVisibleRange(const ECS::TilemapComponent& tilemap, int layerIndex, 
                                       const Float2& position, const Camera2D& camera,
                                       int& outMinX, int& outMinY, int& outMaxX, int& outMaxY);

        /**
         * @brief Вычисляет UV координаты для тайла по GID
         * @param gid Глобальный tile ID
         * @param tilemap Компонент тайловой карты
         * @param outUVMin Выходные минимальные UV
         * @param outUVMax Выходные максимальные UV
         * @param outTexture Выходная текстура для данного GID
         * @return true если GID валиден и tileset найден
         */
        static bool GetTileUV(int gid, const ECS::TilemapComponent& tilemap, 
                             Float2& outUVMin, Float2& outUVMax, Ref<Texture>& outTexture);

    private:
        // Internal: найти tileset для GID
        static int FindTilesetIndex(int gid, const ECS::TilemapComponent& tilemap);
        static void RenderTileLayer(const ECS::TilemapComponent& tilemap, int layerIndex, const Float2& position, const Camera2D& camera);
        static void RenderObjectLayer(const ECS::TilemapComponent& tilemap, int objectLayerIndex, const Float2& position, const Camera2D& camera);
        static void RenderImageLayer(const ECS::TilemapComponent& tilemap, int imageLayerIndex, const Float2& position, const Camera2D& camera);
    };

} // namespace SAGE
