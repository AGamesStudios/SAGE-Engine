#pragma once

#include "Memory/Ref.h"
#include <string>
#include <vector>

namespace SAGE::ECS { struct TilemapComponent; }

namespace SAGE {

    /**
     * @brief TilemapLoader - loads tilemaps from CSV, JSON (Tiled subset), TMX, TMJ
     */
    class TilemapLoader {
    public:
        // Auto-detect format by extension (.csv, .json, .tmj, .tmx)
        static bool Load(const std::string& filepath, ECS::TilemapComponent& outMap);
        
        static bool LoadCSV(const std::string& filepath, ECS::TilemapComponent& outMap);
        static bool LoadJSON(const std::string& filepath, ECS::TilemapComponent& outMap);
        static bool LoadTMX(const std::string& filepath, ECS::TilemapComponent& outMap);
    };

} // namespace SAGE
