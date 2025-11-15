#include "SAGE.h"
#include "Core/ResourceManager.h"
#include "Resources/TilemapLoader.h"
#include "ECS/Components/TilemapComponent.h"

#include <iostream>
#include <iomanip>

// Minimal console example that loads assets/TiledSAGETest.tmx via the engine's
// TilemapLoader. It demonstrates how to inspect tilesets, layers, and fetch
// individual tile IDs. Run the executable from the build/bin folder once the
// example target is added to your CMake build (see top-level CMakeLists.txt).

int main() {
    using namespace SAGE;

    ResourceManager::Get().SetBaseAssetsDir("assets");

    ECS::TilemapComponent tilemap;
    if (!TilemapLoader::LoadTMX("assets/TiledSAGETest.tmx", tilemap)) {
        std::cerr << "[ERROR] Failed to load assets/TiledSAGETest.tmx" << std::endl;
        return 1;
    }

    if (!tilemap.IsValid()) {
        std::cerr << "[ERROR] Loaded TMX map is invalid" << std::endl;
        return 2;
    }

    std::cout << "[INFO] Loaded TMX map with size " << tilemap.mapWidth << "x" << tilemap.mapHeight
              << " tiles (" << tilemap.tileWidth << "x" << tilemap.tileHeight << " px per tile)" << std::endl;

    std::cout << "[INFO] Tilesets (" << tilemap.tilesets.size() << ")" << std::endl;
    for (size_t i = 0; i < tilemap.tilesets.size(); ++i) {
        const auto& ts = tilemap.tilesets[i];
        std::cout << "  - [" << std::setw(2) << i << "] name='" << ts.name << "' firstGID=" << ts.firstGID
                  << " tileCount=" << ts.tileCount << " columns=" << ts.columns
                  << " texture='" << ts.texturePath << "'" << std::endl;
    }

    std::cout << "[INFO] Layers (" << tilemap.layers.size() << ")" << std::endl;
    for (size_t i = 0; i < tilemap.layers.size(); ++i) {
        const auto& layer = tilemap.layers[i];
        std::cout << "  - [" << std::setw(2) << i << "] name='" << layer.name << "' size="
                  << layer.width << "x" << layer.height
                  << " visible=" << (layer.visible ? "true" : "false")
                  << " opacity=" << layer.opacity
                  << " collision=" << (layer.collision ? "true" : "false")
                  << " parallax=" << layer.parallaxFactor.x << "," << layer.parallaxFactor.y
                  << std::endl;
    }

    if (!tilemap.layers.empty()) {
        const auto& layer = tilemap.layers.front();
        const int sampleX = 4;
        const int sampleY = 5;
        if (sampleX >= 0 && sampleX < layer.width && sampleY >= 0 && sampleY < layer.height) {
            const size_t index = static_cast<size_t>(sampleY * layer.width + sampleX);
            const int gid = layer.tiles[index];
            std::cout << "[INFO] Sample tile at layer 0, position (" << sampleX << ", " << sampleY
                      << ") has GID=" << gid << std::endl;
        }
    }

    return 0;
}
