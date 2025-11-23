#include "SAGE/Graphics/Tilemap.h"
#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Log.h"
#include <algorithm>
#include <cmath>

namespace SAGE {

Tilemap::Tilemap(int width, int height, int tileWidth, int tileHeight)
    : m_Width(std::max(1, width))
    , m_Height(std::max(1, height))
    , m_TileWidth(std::max(1, tileWidth))
    , m_TileHeight(std::max(1, tileHeight))
{
    if (width <= 0 || height <= 0 || tileWidth <= 0 || tileHeight <= 0) {
        SAGE_ERROR("Tilemap: Invalid dimensions (w:{}, h:{}, tw:{}, th:{}), using minimum of 1", 
                   width, height, tileWidth, tileHeight);
    }
}

TilemapLayer& Tilemap::AddLayer(const std::string& name, float parallaxFactor) {
    m_Layers.emplace_back(name, m_Width, m_Height);
    m_Layers.back().parallaxFactor = parallaxFactor;
    m_Layers.back().zOrder = static_cast<int>(m_Layers.size()) - 1;
    return m_Layers.back();
}

TilemapLayer* Tilemap::GetLayer(const std::string& name) {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const TilemapLayer& layer) { return layer.name == name; });
    return it != m_Layers.end() ? &(*it) : nullptr;
}

const TilemapLayer* Tilemap::GetLayer(const std::string& name) const {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const TilemapLayer& layer) { return layer.name == name; });
    return it != m_Layers.end() ? &(*it) : nullptr;
}

void Tilemap::SetTile(const std::string& layerName, int x, int y, int tileID, bool collidable) {
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
        return;
    }

    auto* layer = GetLayer(layerName);
    if (!layer) {
        SAGE_WARN("Tilemap: Layer '{}' not found", layerName);
        return;
    }

    layer->GetTile(x, y, m_Width) = Tile(tileID, collidable);
}

const Tile* Tilemap::GetTile(const std::string& layerName, int x, int y) const {
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
        return nullptr;
    }

    const auto* layer = GetLayer(layerName);
    if (!layer) {
        return nullptr;
    }

    return &layer->GetTile(x, y, m_Width);
}

void Tilemap::LoadLayerFromIntArray(const std::string& layerName, const std::vector<int>& data, int width, int height) {
    if (width != m_Width || height != m_Height) {
        SAGE_ERROR("Tilemap: Data dimensions ({}x{}) do not match map dimensions ({}x{})", 
                   width, height, m_Width, m_Height);
        return;
    }

    if (data.size() != static_cast<size_t>(width * height)) {
        SAGE_ERROR("Tilemap: Data size {} does not match dimensions {}x{}", 
                   data.size(), width, height);
        return;
    }

    auto* layer = GetLayer(layerName);
    if (!layer) {
        // Create layer if it doesn't exist
        AddLayer(layerName);
        layer = GetLayer(layerName);
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int tileID = data[y * width + x];
            layer->GetTile(x, y, width) = Tile(tileID, false); // Default non-collidable
        }
    }
}

void Tilemap::LoadLayerFromStringArray(const std::string& layerName, const std::vector<std::string>& mapData, const std::unordered_map<char, int>& charToTileId) {
    if (mapData.size() != static_cast<size_t>(m_Height)) {
        SAGE_ERROR("Tilemap: Map data height {} does not match map height {}", mapData.size(), m_Height);
        return;
    }

    auto* layer = GetLayer(layerName);
    if (!layer) {
        AddLayer(layerName);
        layer = GetLayer(layerName);
    }

    for (int y = 0; y < m_Height; ++y) {
        const std::string& row = mapData[y];
        if (row.length() != static_cast<size_t>(m_Width)) {
            SAGE_ERROR("Tilemap: Row {} length {} does not match map width {}", y, row.length(), m_Width);
            continue; 
        }

        for (int x = 0; x < m_Width; ++x) {
            char c = row[x];
            int tileID = -1;
            if (charToTileId.find(c) != charToTileId.end()) {
                tileID = charToTileId.at(c);
            }
            layer->GetTile(x, y, m_Width) = Tile(tileID, false);
        }
    }
}

void Tilemap::Render(RenderBackend* renderer, const Camera2D& camera) {
    if (m_Tilesets.empty() || !renderer) return;

    Vector2 camPos = camera.GetPosition();
    
    // Simple culling
    // Calculate visible world bounds based on camera
    // Camera position is center of view
    float halfViewW = (camera.GetViewportWidth() / camera.GetZoom()) * 0.5f;
    float halfViewH = (camera.GetViewportHeight() / camera.GetZoom()) * 0.5f;
    
    Vector2 minView = camPos - Vector2(halfViewW, halfViewH);
    Vector2 maxView = camPos + Vector2(halfViewW, halfViewH);

    // Convert world bounds to tile coordinates
    int startX, startY, endX, endY;
    WorldToTile(Vector2(minView.x, maxView.y), startX, startY); // Top-Left (max Y)
    WorldToTile(Vector2(maxView.x, minView.y), endX, endY);     // Bottom-Right (min Y)

    // Clamp to map bounds
    // Note: WorldToTile might return indices outside map
    // Also note: Y is inverted, so startY (from maxView.y) should be smaller index (top of map)
    // But WorldToTile handles inversion:
    // maxView.y (top of screen) -> small tileY (top of map)
    // minView.y (bottom of screen) -> large tileY (bottom of map)
    
    // Ensure correct min/max for loops
    int minTileX = std::max(0, std::min(startX, endX));
    int maxTileX = std::min(m_Width, std::max(startX, endX) + 1);
    
    int minTileY = std::max(0, std::min(startY, endY));
    int maxTileY = std::min(m_Height, std::max(startY, endY) + 1);

    Sprite sprite;

    for (const auto& layer : m_Layers) {
        if (!layer.visible) continue;

        for (int y = minTileY; y < maxTileY; ++y) {
            for (int x = minTileX; x < maxTileX; ++x) {
                const Tile& tile = layer.GetTile(x, y, m_Width);
                if (tile.tileID < 0) continue;

                const Tileset* ts = GetTilesetForTile(tile.tileID);
                if (!ts || !ts->texture) continue;

                sprite.SetTexture(ts->texture);
                Rect uv = GetTileUV(tile.tileID);
                sprite.textureRect = uv;
                
                // Calculate scale to match tile size exactly, plus overlap
                // This compensates for the UV inset which shrinks the sprite
                float texW = (float)ts->texture->GetWidth();
                float texH = (float)ts->texture->GetHeight();
                float spriteW = uv.width * texW;
                float spriteH = uv.height * texH;
                
                float baseScaleX = 1.0f;
                float baseScaleY = 1.0f;
                float rotation = 0.0f;

                if (tile.flipDiagonal) {
                    rotation = 90.0f;
                    baseScaleX = 1.0f;
                    baseScaleY = -1.0f;
                }
                if (tile.flipX) baseScaleX *= -1.0f;
                if (tile.flipY) baseScaleY *= -1.0f;

                if (spriteW > 0.001f && spriteH > 0.001f) {
                    float scaleX = ((float)m_TileWidth / spriteW) * 1.005f * baseScaleX;
                    float scaleY = ((float)m_TileHeight / spriteH) * 1.005f * baseScaleY;
                    sprite.transform.scale = {scaleX, scaleY};
                }
                sprite.transform.rotation = rotation;

                Vector2 worldPos = TileToWorld(x, y);
                
                if (layer.parallaxFactor != 1.0f) {
                    Vector2 offset = camPos * (1.0f - layer.parallaxFactor);
                    sprite.transform.position = worldPos + offset;
                } else {
                    sprite.transform.position = worldPos;
                }
                
                // Apply layer opacity
                sprite.tint.a = layer.opacity;

                renderer->SubmitSprite(sprite);
            }
        }
    }
}

void Tilemap::AddTileset(const Tileset& tileset) {
    m_Tilesets.push_back(tileset);
}

void Tilemap::SetTileset(std::shared_ptr<Texture> texture, int tilesPerRow) {
    m_Tilesets.clear();
    Tileset ts;
    ts.texture = texture;
    ts.firstGID = 0; 
    ts.tileWidth = m_TileWidth;
    ts.tileHeight = m_TileHeight;
    ts.columns = tilesPerRow;
    if (texture) {
        // Ensure pixel-perfect rendering for tilemaps
        texture->SetFilter(TextureFilter::Nearest, TextureFilter::Nearest);
        int rows = texture->GetHeight() / m_TileHeight;
        ts.tileCount = rows * tilesPerRow;
    }
    m_Tilesets.push_back(ts);
}

std::shared_ptr<Texture> Tilemap::GetTileset() const {
    if (m_Tilesets.empty()) return nullptr;
    return m_Tilesets[0].texture;
}

const Tileset* Tilemap::GetTilesetForTile(int tileID) const {
    // Iterate in reverse to find the tileset with the highest firstGID <= tileID
    for (auto it = m_Tilesets.rbegin(); it != m_Tilesets.rend(); ++it) {
        if (tileID >= it->firstGID) {
            return &(*it);
        }
    }
    return nullptr;
}

bool Tilemap::IsCollidable(const std::string& layerName, int x, int y) const {
    const Tile* tile = GetTile(layerName, x, y);
    return tile && tile->collidable;
}

bool Tilemap::IsCollidable(const std::string& layerName, const Vector2& worldPos) const {
    int tileX, tileY;
    WorldToTile(worldPos, tileX, tileY);
    return IsCollidable(layerName, tileX, tileY);
}

Vector2 Tilemap::TileToWorld(int tileX, int tileY) const {
    // Invert Y so that (0,0) is at the top-left visually (highest Y)
    // Assuming Y-Up coordinate system (OpenGL/Box2D standard)
    return {
        static_cast<float>(tileX * m_TileWidth),
        static_cast<float>((m_Height - 1 - tileY) * m_TileHeight)
    };
}

void Tilemap::WorldToTile(const Vector2& worldPos, int& outTileX, int& outTileY) const {
    if (m_TileWidth == 0 || m_TileHeight == 0) {
        outTileX = 0;
        outTileY = 0;
        return;
    }
    // Use floor to correctly handle negative coordinates
    outTileX = static_cast<int>(std::floor(worldPos.x / m_TileWidth));
    // Invert Y back: tileY = m_Height - 1 - (worldY / tileHeight)
    outTileY = m_Height - 1 - static_cast<int>(std::floor(worldPos.y / m_TileHeight));
}

Rect Tilemap::GetTileUV(int tileID) const {
    const Tileset* ts = GetTilesetForTile(tileID);
    if (!ts || !ts->texture || !ts->texture->IsLoaded()) {
        return {0.0f, 0.0f, 1.0f, 1.0f};
    }

    int localID = tileID - ts->firstGID;
    int col = localID % ts->columns;
    int row = localID / ts->columns;

    float u = (float)(ts->margin + (col * (ts->tileWidth + ts->spacing))) / ts->texture->GetWidth();
    
    // Texture is NOT flipped (Top-Left origin).
    // Row 0 is at V=0.
    float v_top = (float)(ts->margin + (row * (ts->tileHeight + ts->spacing))) / ts->texture->GetHeight();
    float v_bottom = v_top + (float)ts->tileHeight / ts->texture->GetHeight();
    
    float u2 = u + (float)ts->tileWidth / ts->texture->GetWidth();

    // Apply small inset to prevent bleeding from neighboring tiles in the atlas
    if (ts->texture->GetWidth() > 0 && ts->texture->GetHeight() > 0) {
        float insetX = 0.05f / ts->texture->GetWidth();
        float insetY = 0.05f / ts->texture->GetHeight();
        u += insetX;
        v_top += insetY;
        u2 -= insetX;
        v_bottom -= insetY;
    }

    // Debug logging for the first tile of the first layer (once)
    static bool logged = false;
    if (!logged && tileID == ts->firstGID) {
        SAGE_INFO("Tilemap UV Debug: TileID={}, LocalID={}, Col={}, Row={}, UV=({}, {}) - ({}, {})", 
            tileID, localID, col, row, u, v_top, u2, v_bottom);
        logged = true;
    }

    // Return Rect{x, y, w, h}.
    // We use negative height trick for SpriteRenderer.
    // y = v_bottom, h = v_top - v_bottom = -height.
    return {u, v_bottom, u2 - u, v_top - v_bottom};
}

} // namespace SAGE
