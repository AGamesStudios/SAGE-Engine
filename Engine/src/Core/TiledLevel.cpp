#include "SAGE/Core/TiledLevel.h"

#include <fstream>
#include <sstream>

namespace SAGE::ECS {

void TiledLevelBuilder::Build(const TiledLevel& level, Registry& reg, const BuildOptions& opts) {
    if (level.grid.empty()) return;
    const size_t rows = level.grid.size();
    const size_t cols = level.grid[0].size();

    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            char id = level.grid[y][x];
            auto it = level.definitions.find(id);
            if (it == level.definitions.end()) {
                continue; // неизвестный id трактуем как "воздух"
            }
            const TileDefinition& def = it->second;
            // позиция — центр тайла
            Vector2 pos{
                level.origin.x + static_cast<float>(x) * level.tileSize.x + level.tileSize.x * 0.5f,
                level.origin.y + static_cast<float>(y) * level.tileSize.y + level.tileSize.y * 0.5f
            };

            Entity e = reg.CreateEntity();
            auto& t = reg.Add<TransformComponent>(e);
            t.position = pos;
            t.scale = level.tileSize;

            if (def.texture) {
                auto& s = reg.Add<SpriteComponent>(e);
                s.layer = opts.renderLayer;
                s.transparent = def.transparent;
                s.sprite.SetTexture(def.texture);
            }

            if (def.solid) {
                auto& c = reg.Add<ColliderComponent>(e);
                c.size = level.tileSize;
            }

            if (def.onSpawn) {
                def.onSpawn(e, reg);
            }
        }
    }

    // Merging solid colliders по строкам для снижения числа коллайдеров
    if (opts.mergeSolidColliders) {
        for (size_t y = 0; y < rows; ++y) {
            size_t x = 0;
            while (x < cols) {
                char id = level.grid[y][x];
                auto it = level.definitions.find(id);
                if (it == level.definitions.end() || !it->second.solid) {
                    ++x;
                    continue;
                }
                // старт сегмента
                size_t start = x;
                while (x < cols) {
                    auto it2 = level.definitions.find(level.grid[y][x]);
                    if (it2 == level.definitions.end() || !it2->second.solid) {
                        break;
                    }
                    ++x;
                }
                size_t end = x; // невключительно
                float width = static_cast<float>(end - start) * level.tileSize.x;
                // центр сегмента
                Vector2 pos{
                    level.origin.x + (static_cast<float>(start) + (static_cast<float>(end - start) * 0.5f)) * level.tileSize.x,
                    level.origin.y + static_cast<float>(y) * level.tileSize.y + level.tileSize.y * 0.5f
                };
                Entity seg = reg.CreateEntity();
                auto& tSeg = reg.Add<TransformComponent>(seg);
                tSeg.position = pos;
                tSeg.scale = {width, level.tileSize.y};
                auto& cSeg = reg.Add<ColliderComponent>(seg);
                cSeg.size = {width, level.tileSize.y};
            }
        }
    }
}

TiledLevel TiledLevelBuilder::LoadFromText(const std::string& path, const Vector2& tileSize, const Vector2& origin) {
    TiledLevel level{};
    level.tileSize = tileSize;
    level.origin = origin;

    std::ifstream file(path);
    if (!file.is_open()) {
        return level;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        level.grid.push_back(line);
    }
    return level;
}

} // namespace SAGE::ECS
