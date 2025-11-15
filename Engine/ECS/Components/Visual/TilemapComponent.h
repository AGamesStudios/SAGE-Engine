#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Core/Color.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <variant>

namespace SAGE {
class Texture;
}

namespace SAGE::ECS {

    // Chunk size for infinite tilemaps
    constexpr int TILEMAP_CHUNK_SIZE = 16;
    constexpr int TILEMAP_CHUNK_TILES = TILEMAP_CHUNK_SIZE * TILEMAP_CHUNK_SIZE;

    /**
     * @brief CustomProperty - typed property value from Tiled
     */
    enum class PropertyType : uint8_t {
        String,
        Int,
        Float,
        Bool,
        Color,
        File,
        Object  // object reference (not fully supported)
    };

    struct CustomProperty {
        PropertyType type = PropertyType::String;
        std::variant<std::string, int, float, bool, SAGE::Color> value;

        // Helper constructors
        CustomProperty() : type(PropertyType::String), value(std::string()) {}
        CustomProperty(const std::string& s) : type(PropertyType::String), value(s) {}
        CustomProperty(int i) : type(PropertyType::Int), value(i) {}
        CustomProperty(float f) : type(PropertyType::Float), value(f) {}
        CustomProperty(bool b) : type(PropertyType::Bool), value(b) {}
        CustomProperty(const SAGE::Color& c) : type(PropertyType::Color), value(c) {}

        // Helper getters
        template<typename T>
        T GetValue() const {
            return std::get<T>(value);
        }

        std::string AsString() const {
            if (type == PropertyType::String || type == PropertyType::File) {
                return std::get<std::string>(value);
            }
            return "";
        }

        int AsInt() const {
            if (type == PropertyType::Int) return std::get<int>(value);
            if (type == PropertyType::Bool) return std::get<bool>(value) ? 1 : 0;
            return 0;
        }

        float AsFloat() const {
            if (type == PropertyType::Float) return std::get<float>(value);
            if (type == PropertyType::Int) return static_cast<float>(std::get<int>(value));
            return 0.0f;
        }

        bool AsBool() const {
            if (type == PropertyType::Bool) return std::get<bool>(value);
            if (type == PropertyType::Int) return std::get<int>(value) != 0;
            return false;
        }

        SAGE::Color AsColor() const {
            if (type == PropertyType::Color) return std::get<SAGE::Color>(value);
            return SAGE::Color::White();
        }
    };

    /**
     * @brief TilemapChunk - chunk of tiles for infinite maps
     */
    struct TilemapChunk {
        int x = 0;                  // chunk X coordinate (in tiles)
        int y = 0;                  // chunk Y coordinate (in tiles)
        int width = TILEMAP_CHUNK_SIZE;   // chunk width in tiles
        int height = TILEMAP_CHUNK_SIZE;  // chunk height in tiles
        int tiles[TILEMAP_CHUNK_TILES];   // tile IDs (fixed size = 256)
        
        TilemapChunk() {
            std::fill_n(tiles, TILEMAP_CHUNK_TILES, -1);
        }
    };

    /**
     * @brief TilemapLayer - single layer in a tilemap
     * Supports: visual layers, collision layers, metadata layers.
     */
    struct TilemapLayer {
        std::string name;
        int width = 0;              // number of tiles horizontally
        int height = 0;             // number of tiles vertically
        std::vector<int> tiles;     // tile IDs (size = width*height), -1 = empty
        
        // Infinite map chunks (only used if tilemap.infinite == true)
        std::unordered_map<int64_t, TilemapChunk> chunks; // key = (chunkY << 32) | chunkX
        
        bool visible = true;
        float opacity = 1.0f;
        bool collision = false;     // whether this layer should generate collision
        // Per-layer pixel offset (Tiled: offsetx / offsety)
        Float2 offset {0.0f, 0.0f};
        // Layer tint color (Tiled attribute 'tintcolor' #AARRGGBB or #RRGGBB). Applied multiplicatively.
        Color tint = Color::White();

        // Parallax factor (1.0 = normal, <1.0 = slower background, >1.0 = faster foreground)
        Float2 parallaxFactor = {1.0f, 1.0f};
        
        // Custom properties from Tiled
        std::unordered_map<std::string, CustomProperty> properties;
        
        // VBO cache (for per-layer buffer optimization)
        bool vboCached = false;
        uint32_t vboID = 0;         // OpenGL VBO handle (if cached)
        uint32_t iboID = 0;         // OpenGL IBO handle (if cached)
        int cachedVertexCount = 0;
        int cachedIndexCount = 0;
        
        // Helper: get tile at position (handles both regular and chunk-based storage)
        int GetTile(int x, int y) const {
            if (chunks.empty()) {
                // Regular tilemap
                if (x < 0 || y < 0 || x >= width || y >= height) return -1;
                return tiles[y * width + x];
            } else {
                // Infinite map with chunks
                int chunkX = x / TILEMAP_CHUNK_SIZE;
                int chunkY = y / TILEMAP_CHUNK_SIZE;
                int64_t key = (static_cast<int64_t>(chunkY) << 32) | static_cast<uint32_t>(chunkX);
                auto it = chunks.find(key);
                if (it == chunks.end()) return -1;
                
                const auto& chunk = it->second;
                int localX = x - chunk.x;
                int localY = y - chunk.y;
                if (localX < 0 || localY < 0 || localX >= chunk.width || localY >= chunk.height) return -1;
                return chunk.tiles[localY * chunk.width + localX];
            }
        }
    };

    enum class TilemapObjectShape : uint8_t {
        Rectangle,
        Ellipse,
        Polygon,
        Polyline,
        Point,
        Text,
        Tile // classic tile object with GID
    };

    struct TilemapSprite { // Generic object representation
        std::string name;
        Float2 position{0.0f, 0.0f}; // top-left in map pixel coordinates
        Float2 size{0.0f, 0.0f};
        float rotation = 0.0f;       // degrees
        uint32_t gid = 0;            // raw GID including flip flags
        bool visible = true;
        TilemapObjectShape shape = TilemapObjectShape::Tile;
        // For polygon / polyline shapes
        std::vector<Float2> points;  // local points relative to position
        
        Color tint = Color::White();
        // Custom properties from Tiled
        std::unordered_map<std::string, CustomProperty> properties;
    };

    struct TilemapObjectLayer {
        std::string name;
        bool visible = true;
        float opacity = 1.0f;
        Float2 offset{0.0f, 0.0f};
        Float2 parallaxFactor = {1.0f, 1.0f};
        bool collision = false;
        Color tint = Color::White();
        std::vector<TilemapSprite> sprites;
        // Custom properties from Tiled
        std::unordered_map<std::string, CustomProperty> properties;
    };

    /**
     * @brief TilemapImageLayer - background/foreground image layer
     */
    struct TilemapImageLayer {
        std::string name;
        std::string imagePath;      // relative path to image
        Ref<Texture> texture = nullptr;
        Float2 offset{0.0f, 0.0f};  // pixel offset (offsetx, offsety)
        Float2 parallaxFactor = {1.0f, 1.0f}; // parallax scrolling
        bool visible = true;
        float opacity = 1.0f;       // 0.0 - 1.0
        Color tint = Color::White(); // multiplicative tint
        bool repeatX = false;       // repeat image horizontally
        bool repeatY = false;       // repeat image vertically
    };

    enum class TilemapOrientation : uint8_t {
        Orthogonal,
        Isometric,
        Staggered,
        Hexagonal
    };

    enum class TilemapStaggerAxis : uint8_t {
        None,
        X,
        Y
    };

    enum class TilemapStaggerIndex : uint8_t {
        None,
        Even,
        Odd
    };

    enum class TilemapRenderOrder : uint8_t {
        RightDown,
        RightUp,
        LeftDown,
        LeftUp
    };

    enum class TilemapLayerType : uint8_t {
        Tile = 0,
        Object = 1,
        Image = 2
    };

    struct TilemapLayerEntry {
        TilemapLayerType type = TilemapLayerType::Tile;
        int index = -1;
    };

    /**
     * @brief AnimationFrame - frame in tile animation
     */
    struct AnimationFrame {
        int localTileID = -1;  // local tile id (within tileset)
        int durationMs = 0;    // frame duration in milliseconds
    };

    /**
     * @brief TileCollisionShape - collision shape for a tile
     */
    enum class CollisionShapeType : uint8_t {
        Rectangle,
        Ellipse,
        Polygon
    };

    struct TileCollisionShape {
        CollisionShapeType type = CollisionShapeType::Rectangle;
        Float2 offset{0.0f, 0.0f};      // offset from tile origin
        Float2 size{0.0f, 0.0f};        // for rectangle/ellipse
        std::vector<Float2> points;      // for polygon (relative to tile origin)
    };

    struct TileDefinition {
        int localID = -1; // 0-based local tile index within tileset
        std::unordered_map<std::string, CustomProperty> properties; // typed custom properties
        std::vector<AnimationFrame> animation; // optional animation sequence
        std::vector<TileCollisionShape> collisionShapes; // collision shapes for this tile
        bool IsAnimated() const { return !animation.empty(); }
        const AnimationFrame* FirstFrame() const { return animation.empty() ? nullptr : &animation.front(); }
        bool HasCollision() const { return !collisionShapes.empty(); }
    };

    struct TilesetInfo {
        std::string name;
        int firstGID = 1;           // First global tile ID
        int tileWidth = 0;
        int tileHeight = 0;
        int columns = 0;
        int tileCount = 0;
        // Optional pixel margins/spacing around tiles in the atlas (Tiled supports these)
        int margin = 0;             // pixels before first tile and after last row/col
        int spacing = 0;            // pixels between tiles
        // Tile offset - offset to apply when drawing tiles (in pixels)
        Float2 tileOffset{0.0f, 0.0f}; // offsetx, offsety from Tiled
        std::string texturePath;
        Ref<Texture> texture = nullptr;
        // Extended metadata per tile
        std::vector<TileDefinition> tiles; // index by localID

        const TileDefinition* GetTileDefinition(int localID) const {
            if (localID < 0 || localID >= (int)tiles.size()) return nullptr;
            return &tiles[localID];
        }
    };

    /**
     * @brief TilemapComponent - ECS component storing tilemap data
     */
    struct TilemapComponent {
        int mapWidth = 0;   // in tiles (0 for infinite maps)
        int mapHeight = 0;  // in tiles (0 for infinite maps)
        int tileWidth = 0;  // pixel size
        int tileHeight = 0; // pixel size
        bool infinite = false; // true if this is an infinite map with chunks
    TilemapOrientation orientation = TilemapOrientation::Orthogonal;
    TilemapRenderOrder renderOrder = TilemapRenderOrder::RightDown;
    TilemapStaggerAxis staggerAxis = TilemapStaggerAxis::None;
    TilemapStaggerIndex staggerIndex = TilemapStaggerIndex::None;
    int hexSideLength = 0; // relevant for hexagonal orientation

        std::vector<TilemapLayer> layers;
        std::vector<TilemapObjectLayer> objectLayers;
        std::vector<TilemapImageLayer> imageLayers;
        std::vector<TilesetInfo> tilesets;

        struct AnimationState {
            size_t currentFrame = 0;
            float elapsedMs = 0.0f;
        };

        std::unordered_map<int, AnimationState> animationStates;
        bool collisionBuilt = false;
        
        // Custom properties from Tiled (map-level)
        std::unordered_map<std::string, CustomProperty> properties;
        
        // Helper: access layer by name
        TilemapLayer* GetLayer(const std::string& name) {
            for (auto& layer : layers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }
        const TilemapLayer* GetLayer(const std::string& name) const {
            for (auto& layer : layers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }

        TilemapObjectLayer* GetObjectLayer(const std::string& name) {
            for (auto& layer : objectLayers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }
        const TilemapObjectLayer* GetObjectLayer(const std::string& name) const {
            for (auto& layer : objectLayers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }

        TilemapImageLayer* GetImageLayer(const std::string& name) {
            for (auto& layer : imageLayers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }
        const TilemapImageLayer* GetImageLayer(const std::string& name) const {
            for (auto& layer : imageLayers) {
                if (layer.name == name) return &layer;
            }
            return nullptr;
        }
        
        bool IsValid() const {
            return mapWidth > 0 && mapHeight > 0 && tileWidth > 0 && tileHeight > 0 && (!layers.empty() || !objectLayers.empty());
        }
        
        // ============================================================
        // Helper functions for programmatic tilemap creation
        // ============================================================
        
        /**
         * @brief Create an empty orthogonal tilemap
         * @param width Map width in tiles
         * @param height Map height in tiles
         * @param tileW Tile width in pixels
         * @param tileH Tile height in pixels
         * @return Configured TilemapComponent
         */
        static TilemapComponent CreateOrthogonal(int width, int height, int tileW = 32, int tileH = 32) {
            TilemapComponent tilemap;
            tilemap.mapWidth = width;
            tilemap.mapHeight = height;
            tilemap.tileWidth = tileW;
            tilemap.tileHeight = tileH;
            tilemap.orientation = TilemapOrientation::Orthogonal;
            tilemap.renderOrder = TilemapRenderOrder::RightDown;
            return tilemap;
        }
        
        /**
         * @brief Create an empty isometric tilemap
         * @param width Map width in tiles
         * @param height Map height in tiles
         * @param tileW Tile width in pixels
         * @param tileH Tile height in pixels
         * @return Configured TilemapComponent
         */
        static TilemapComponent CreateIsometric(int width, int height, int tileW = 64, int tileH = 32) {
            TilemapComponent tilemap;
            tilemap.mapWidth = width;
            tilemap.mapHeight = height;
            tilemap.tileWidth = tileW;
            tilemap.tileHeight = tileH;
            tilemap.orientation = TilemapOrientation::Isometric;
            tilemap.renderOrder = TilemapRenderOrder::RightDown;
            return tilemap;
        }
        
        /**
         * @brief Create an empty staggered tilemap
         * @param width Map width in tiles
         * @param height Map height in tiles
         * @param tileW Tile width in pixels
         * @param tileH Tile height in pixels
         * @param axis Stagger axis (X or Y)
         * @param index Stagger index (Even or Odd)
         * @return Configured TilemapComponent
         */
        static TilemapComponent CreateStaggered(int width, int height, int tileW = 32, int tileH = 32,
                                               TilemapStaggerAxis axis = TilemapStaggerAxis::Y,
                                               TilemapStaggerIndex index = TilemapStaggerIndex::Odd) {
            TilemapComponent tilemap;
            tilemap.mapWidth = width;
            tilemap.mapHeight = height;
            tilemap.tileWidth = tileW;
            tilemap.tileHeight = tileH;
            tilemap.orientation = TilemapOrientation::Staggered;
            tilemap.staggerAxis = axis;
            tilemap.staggerIndex = index;
            tilemap.renderOrder = TilemapRenderOrder::RightDown;
            return tilemap;
        }
        
        /**
         * @brief Create an empty hexagonal tilemap
         * @param width Map width in tiles
         * @param height Map height in tiles
         * @param tileW Tile width in pixels
         * @param tileH Tile height in pixels
         * @param sideLength Hexagon side length
         * @param axis Stagger axis (X or Y)
         * @param index Stagger index (Even or Odd)
         * @return Configured TilemapComponent
         */
        static TilemapComponent CreateHexagonal(int width, int height, int tileW = 28, int tileH = 32,
                                               int sideLength = 14,
                                               TilemapStaggerAxis axis = TilemapStaggerAxis::X,
                                               TilemapStaggerIndex index = TilemapStaggerIndex::Even) {
            TilemapComponent tilemap;
            tilemap.mapWidth = width;
            tilemap.mapHeight = height;
            tilemap.tileWidth = tileW;
            tilemap.tileHeight = tileH;
            tilemap.orientation = TilemapOrientation::Hexagonal;
            tilemap.staggerAxis = axis;
            tilemap.staggerIndex = index;
            tilemap.hexSideLength = sideLength;
            tilemap.renderOrder = TilemapRenderOrder::RightDown;
            return tilemap;
        }
        
        /**
         * @brief Add a basic tile layer to the tilemap
         * @param name Layer name
         * @param fillTileID Tile ID to fill layer with (0 = empty)
         * @return Reference to the created layer
         */
        TilemapLayer& AddLayer(const std::string& name, int fillTileID = 0) {
            TilemapLayer layer;
            layer.name = name;
            layer.width = mapWidth;
            layer.height = mapHeight;
            layer.visible = true;
            layer.opacity = 1.0f;
            layer.collision = false;
            layer.parallaxFactor = {1.0f, 1.0f};
            layer.tiles.resize(mapWidth * mapHeight, fillTileID);
            layers.push_back(layer);
            return layers.back();
        }
        
        /**
         * @brief Add a basic tileset to the tilemap
         * @param name Tileset name
         * @param firstGID First global tile ID
         * @param tileCount Total number of tiles in tileset
         * @param columns Number of columns in tileset texture
         * @return Reference to the created tileset
         */
        TilesetInfo& AddTileset(const std::string& name, int firstGID, int tileCount, int columns) {
            TilesetInfo tileset;
            tileset.name = name;
            tileset.firstGID = firstGID;
            tileset.tileCount = tileCount;
            tileset.columns = columns;
            tileset.tileWidth = tileWidth;
            tileset.tileHeight = tileHeight;
            tilesets.push_back(tileset);
            return tilesets.back();
        }
    };

} // namespace SAGE::ECS
