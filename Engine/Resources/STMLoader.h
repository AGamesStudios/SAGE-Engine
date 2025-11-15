#pragma once

#include "ECS/Components/TilemapComponent.h"
#include <string>
#include <fstream>

namespace SAGE {

/// @brief Loader for SAGE Tilemap (.stm) binary format
class STMLoader {
public:
    /// @brief Load .stm file into TilemapComponent
    static bool Load(const std::string& filepath, ECS::TilemapComponent& outMap);

private:
    static bool ReadHeader(std::ifstream& file, ECS::TilemapComponent& outMap);
    static bool ReadTilesets(std::ifstream& file, ECS::TilemapComponent& outMap, uint32_t count, uint32_t flags);
    static bool ReadLayers(std::ifstream& file, ECS::TilemapComponent& outMap, uint32_t count, uint32_t flags);
    static bool ReadTileLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags);
    static bool ReadObjectLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags);
    static bool ReadImageLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags);
    static bool DecompressTileDataRLE(std::ifstream& file, std::vector<int>& outTiles, uint32_t width, uint32_t height);
    
    // Utility functions
    static std::string ReadString(std::ifstream& file, uint16_t length);
    static bool Validate(const std::string& filepath);
};

} // namespace SAGE
