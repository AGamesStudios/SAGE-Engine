#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Rect.h"
#include "SAGE/Graphics/Texture.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace SAGE {

// Tile data for tilemaps
struct Tile {
    int tileID = -1;  // -1 = empty tile
    bool collidable = false;
    bool flipX = false;
    bool flipY = false;
    bool flipDiagonal = false;
    
    Tile() = default;
    Tile(int id, bool collide = false) : tileID(id), collidable(collide) {}
};

// Tilemap layer (multiple layers = parallax, foreground/background)
struct TilemapLayer {
    std::string name;
    std::vector<Tile> tiles;
    float parallaxFactor = 1.0f;  // 1.0 = moves with camera, 0.5 = moves half speed (background)
    bool visible = true;
    int zOrder = 0;  // For layer ordering
    float opacity = 1.0f;

    TilemapLayer() = default;
    TilemapLayer(const std::string& layerName, int width, int height)
        : name(layerName)
        , tiles(width * height)
    {}

    Tile& GetTile(int x, int y, int width) {
        return tiles[y * width + x];
    }

    const Tile& GetTile(int x, int y, int width) const {
        return tiles[y * width + x];
    }
};

struct Tileset {
    std::shared_ptr<Texture> texture;
    int firstGID = 1;
    int tileWidth = 32;
    int tileHeight = 32;
    int tileCount = 0;
    int columns = 0;
    int margin = 0;
    int spacing = 0;
    std::string name;

    bool Contains(int gid) const {
        return gid >= firstGID && gid < firstGID + tileCount;
    }
};

// Tilemap - grid-based map system
class Tilemap {
public:
    Tilemap(int width, int height, int tileWidth, int tileHeight);
    ~Tilemap() = default;

    // Layer management
    TilemapLayer& AddLayer(const std::string& name, float parallaxFactor = 1.0f);
    TilemapLayer* GetLayer(const std::string& name);
    const TilemapLayer* GetLayer(const std::string& name) const;
    size_t GetLayerCount() const { return m_Layers.size(); }

    // Tile operations
    void SetTile(const std::string& layerName, int x, int y, int tileID, bool collidable = false);
    const Tile* GetTile(const std::string& layerName, int x, int y) const;
    
    // Bulk loading
    void LoadLayerFromIntArray(const std::string& layerName, const std::vector<int>& data, int width, int height);
    void LoadLayerFromStringArray(const std::string& layerName, const std::vector<std::string>& mapData, const std::unordered_map<char, int>& charToTileId);

    // Rendering
    void Render(class RenderBackend* renderer, const class Camera2D& camera);

    // Tileset (texture with tiles)
    void AddTileset(const Tileset& tileset);
    // Legacy support (wraps into a Tileset)
    void SetTileset(std::shared_ptr<Texture> texture, int tilesPerRow);
    std::shared_ptr<Texture> GetTileset() const; // Returns first tileset's texture
    
    // Collision queries
    bool IsCollidable(const std::string& layerName, int x, int y) const;
    bool IsCollidable(const std::string& layerName, const Vector2& worldPos) const;
    
    // World <-> Tile coordinate conversion
    Vector2 TileToWorld(int tileX, int tileY) const;
    void WorldToTile(const Vector2& worldPos, int& outTileX, int& outTileY) const;
    
    // Getters
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetTileWidth() const { return m_TileWidth; }
    int GetTileHeight() const { return m_TileHeight; }
    
    // Get UV coordinates for a tile ID
    Rect GetTileUV(int tileID) const;
    const Tileset* GetTilesetForTile(int tileID) const;

    std::vector<TilemapLayer>& GetLayers() { return m_Layers; }
    const std::vector<TilemapLayer>& GetLayers() const { return m_Layers; }

private:
    int m_Width;           // Map width in tiles
    int m_Height;          // Map height in tiles
    int m_TileWidth;       // Tile width in pixels
    int m_TileHeight;      // Tile height in pixels

    std::vector<TilemapLayer> m_Layers;
    std::vector<Tileset> m_Tilesets;
};

} // namespace SAGE
