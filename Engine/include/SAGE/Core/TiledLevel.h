#pragma once

#include "SAGE/Core/ECS.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Graphics/Texture.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace SAGE::ECS {

struct TileDefinition {
    char id = '0';
    std::shared_ptr<Texture> texture;
    bool solid = false;
    bool transparent = false;
    // пользовательский спавн: возвращает Entity (может создать новый)
    std::function<void(Entity, Registry&)> onSpawn;
};

struct TiledLevel {
    std::vector<std::string> grid; // строки одинаковой длины
    std::unordered_map<char, TileDefinition> definitions;
    Vector2 origin{0.0f, 0.0f};
    Vector2 tileSize{32.0f, 32.0f};
};

// Утилита генерации уровня из текстовой сетки
class TiledLevelBuilder {
public:
    struct BuildOptions {
        int renderLayer = 0;
        bool mergeSolidColliders = true; // объединять соседние solid-тайлы в один коллайдер по строкам
    };

    static void Build(const TiledLevel& level, Registry& reg, const BuildOptions& opts);
    static void Build(const TiledLevel& level, Registry& reg) { Build(level, reg, BuildOptions{}); }
    static TiledLevel LoadFromText(const std::string& path, const Vector2& tileSize, const Vector2& origin = {0.0f, 0.0f});
};

} // namespace SAGE::ECS
