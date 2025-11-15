#include "../TestFramework.h"
#include "ECS/Components/TilemapComponent.h"
#include "Graphics/Core/Rendering/TilemapRenderer.h"

using namespace SAGE;
using namespace SAGE::ECS;

TEST_CASE("TilemapComponent - Manual Creation Orthogonal") {
    TilemapComponent tilemap;
    
    // Setup basic tilemap parameters
    tilemap.mapWidth = 10;
    tilemap.mapHeight = 8;
    tilemap.tileWidth = 32;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Orthogonal;
    tilemap.renderOrder = TilemapRenderOrder::RightDown;
    
    // Create a tileset
    TilesetInfo tileset;
    tileset.name = "test_tileset";
    tileset.firstGID = 1;
    tileset.tileCount = 64;
    tileset.columns = 8;
    tileset.tileWidth = 32;
    tileset.tileHeight = 32;
    tilemap.tilesets.push_back(tileset);
    
    // Create a layer
    TilemapLayer layer;
    layer.name = "ground";
    layer.width = tilemap.mapWidth;
    layer.height = tilemap.mapHeight;
    layer.visible = true;
    layer.opacity = 1.0f;
    layer.collision = false;
    layer.parallaxFactor = {1.0f, 1.0f};
    
    // Fill with checkerboard pattern
    layer.tiles.resize(layer.width * layer.height);
    for (int y = 0; y < layer.height; ++y) {
        for (int x = 0; x < layer.width; ++x) {
            int idx = y * layer.width + x;
            layer.tiles[idx] = ((x + y) % 2 == 0) ? 1 : 2;
        }
    }
    
    tilemap.layers.push_back(layer);
    
    // Validate
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.mapWidth == 10);
    REQUIRE(tilemap.mapHeight == 8);
    REQUIRE(tilemap.orientation == TilemapOrientation::Orthogonal);
    REQUIRE(tilemap.layers.size() == 1);
    REQUIRE(tilemap.layers[0].tiles.size() == 80);
    REQUIRE(tilemap.tilesets.size() == 1);
    REQUIRE(tilemap.tilesets[0].firstGID == 1);
}

TEST_CASE("TilemapComponent - Manual Creation Isometric") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 10;
    tilemap.mapHeight = 10;
    tilemap.tileWidth = 64;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Isometric;
    tilemap.renderOrder = TilemapRenderOrder::RightDown;
    
    TilesetInfo tileset;
    tileset.name = "iso_tileset";
    tileset.firstGID = 1;
    tileset.tileCount = 16;
    tileset.columns = 4;
    tileset.tileWidth = 64;
    tileset.tileHeight = 32;
    tilemap.tilesets.push_back(tileset);
    
    TilemapLayer layer;
    layer.name = "iso_ground";
    layer.width = tilemap.mapWidth;
    layer.height = tilemap.mapHeight;
    layer.visible = true;
    layer.opacity = 1.0f;
    layer.tiles.resize(layer.width * layer.height, 1);
    
    tilemap.layers.push_back(layer);
    
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.orientation == TilemapOrientation::Isometric);
    REQUIRE(tilemap.tileWidth == 64);
    REQUIRE(tilemap.tileHeight == 32);
}

TEST_CASE("TilemapComponent - Manual Creation Staggered") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 12;
    tilemap.mapHeight = 10;
    tilemap.tileWidth = 32;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Staggered;
    tilemap.staggerAxis = TilemapStaggerAxis::Y;
    tilemap.staggerIndex = TilemapStaggerIndex::Odd;
    tilemap.renderOrder = TilemapRenderOrder::RightDown;
    
    TilesetInfo tileset;
    tileset.name = "stagger_tileset";
    tileset.firstGID = 1;
    tileset.tileCount = 32;
    tileset.columns = 8;
    tileset.tileWidth = 32;
    tileset.tileHeight = 32;
    tilemap.tilesets.push_back(tileset);
    
    TilemapLayer layer;
    layer.name = "stagger_ground";
    layer.width = tilemap.mapWidth;
    layer.height = tilemap.mapHeight;
    layer.visible = true;
    layer.opacity = 1.0f;
    layer.tiles.resize(layer.width * layer.height, 1);
    
    tilemap.layers.push_back(layer);
    
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.orientation == TilemapOrientation::Staggered);
    REQUIRE(tilemap.staggerAxis == TilemapStaggerAxis::Y);
    REQUIRE(tilemap.staggerIndex == TilemapStaggerIndex::Odd);
}

TEST_CASE("TilemapComponent - Manual Creation Hexagonal") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 15;
    tilemap.mapHeight = 12;
    tilemap.tileWidth = 28;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Hexagonal;
    tilemap.staggerAxis = TilemapStaggerAxis::X;
    tilemap.staggerIndex = TilemapStaggerIndex::Even;
    tilemap.hexSideLength = 14;
    tilemap.renderOrder = TilemapRenderOrder::RightDown;
    
    TilesetInfo tileset;
    tileset.name = "hex_tileset";
    tileset.firstGID = 1;
    tileset.tileCount = 20;
    tileset.columns = 5;
    tileset.tileWidth = 28;
    tileset.tileHeight = 32;
    tilemap.tilesets.push_back(tileset);
    
    TilemapLayer layer;
    layer.name = "hex_ground";
    layer.width = tilemap.mapWidth;
    layer.height = tilemap.mapHeight;
    layer.visible = true;
    layer.opacity = 1.0f;
    layer.tiles.resize(layer.width * layer.height, 1);
    
    tilemap.layers.push_back(layer);
    
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.orientation == TilemapOrientation::Hexagonal);
    REQUIRE(tilemap.staggerAxis == TilemapStaggerAxis::X);
    REQUIRE(tilemap.staggerIndex == TilemapStaggerIndex::Even);
    REQUIRE(tilemap.hexSideLength == 14);
}

TEST_CASE("TilemapComponent - Multiple Layers") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 16;
    tilemap.mapHeight = 16;
    tilemap.tileWidth = 16;
    tilemap.tileHeight = 16;
    tilemap.orientation = TilemapOrientation::Orthogonal;
    
    TilesetInfo tileset;
    tileset.name = "terrain";
    tileset.firstGID = 1;
    tileset.tileCount = 100;
    tileset.columns = 10;
    tileset.tileWidth = 16;
    tileset.tileHeight = 16;
    tilemap.tilesets.push_back(tileset);
    
    // Layer 1: Background
    TilemapLayer bgLayer;
    bgLayer.name = "background";
    bgLayer.width = tilemap.mapWidth;
    bgLayer.height = tilemap.mapHeight;
    bgLayer.visible = true;
    bgLayer.opacity = 1.0f;
    bgLayer.parallaxFactor = {0.5f, 0.5f};
    bgLayer.tiles.resize(bgLayer.width * bgLayer.height, 1);
    tilemap.layers.push_back(bgLayer);
    
    // Layer 2: Ground
    TilemapLayer groundLayer;
    groundLayer.name = "ground";
    groundLayer.width = tilemap.mapWidth;
    groundLayer.height = tilemap.mapHeight;
    groundLayer.visible = true;
    groundLayer.opacity = 1.0f;
    groundLayer.collision = true;
    groundLayer.tiles.resize(groundLayer.width * groundLayer.height, 2);
    tilemap.layers.push_back(groundLayer);
    
    // Layer 3: Overlay
    TilemapLayer overlayLayer;
    overlayLayer.name = "overlay";
    overlayLayer.width = tilemap.mapWidth;
    overlayLayer.height = tilemap.mapHeight;
    overlayLayer.visible = true;
    overlayLayer.opacity = 0.7f;
    overlayLayer.tiles.resize(overlayLayer.width * overlayLayer.height, 0);
    tilemap.layers.push_back(overlayLayer);
    
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.layers.size() == 3);
    REQUIRE(tilemap.GetLayer("background") != nullptr);
    REQUIRE(tilemap.GetLayer("ground") != nullptr);
    REQUIRE(tilemap.GetLayer("overlay") != nullptr);
    REQUIRE(tilemap.GetLayer("ground")->collision == true);
    REQUIRE(tilemap.GetLayer("background")->parallaxFactor.x == 0.5f);
    REQUIRE(tilemap.GetLayer("overlay")->opacity == 0.7f);
}

TEST_CASE("TilemapComponent - Animated Tiles") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 8;
    tilemap.mapHeight = 8;
    tilemap.tileWidth = 32;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Orthogonal;
    
    TilesetInfo tileset;
    tileset.name = "animated";
    tileset.firstGID = 1;
    tileset.tileCount = 16;
    tileset.columns = 4;
    tileset.tileWidth = 32;
    tileset.tileHeight = 32;
    
    // Add animated tile definition
    TileDefinition animTile;
    animTile.id = 5;
    animTile.hasAnimation = true;
    
    AnimationFrame frame1;
    frame1.tileID = 5;
    frame1.duration = 100;
    animTile.animationFrames.push_back(frame1);
    
    AnimationFrame frame2;
    frame2.tileID = 6;
    frame2.duration = 100;
    animTile.animationFrames.push_back(frame2);
    
    AnimationFrame frame3;
    frame3.tileID = 7;
    frame3.duration = 100;
    animTile.animationFrames.push_back(frame3);
    
    tileset.tiles.push_back(animTile);
    tilemap.tilesets.push_back(tileset);
    
    TilemapLayer layer;
    layer.name = "animated_layer";
    layer.width = tilemap.mapWidth;
    layer.height = tilemap.mapHeight;
    layer.visible = true;
    layer.opacity = 1.0f;
    layer.tiles.resize(layer.width * layer.height, 0);
    layer.tiles[0] = 5; // Place animated tile at (0,0)
    
    tilemap.layers.push_back(layer);
    
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.tilesets[0].tiles.size() == 1);
    REQUIRE(tilemap.tilesets[0].tiles[0].hasAnimation == true);
    REQUIRE(tilemap.tilesets[0].tiles[0].animationFrames.size() == 3);
}

TEST_CASE("TilemapComponent - Object Layer") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 20;
    tilemap.mapHeight = 15;
    tilemap.tileWidth = 16;
    tilemap.tileHeight = 16;
    tilemap.orientation = TilemapOrientation::Orthogonal;
    
    // Create object layer
    TilemapObjectLayer objLayer;
    objLayer.name = "entities";
    objLayer.visible = true;
    objLayer.opacity = 1.0f;
    
    // Add a spawn point object
    TilemapObject spawnPoint;
    spawnPoint.name = "player_spawn";
    spawnPoint.type = "spawn";
    spawnPoint.x = 100.0f;
    spawnPoint.y = 150.0f;
    spawnPoint.width = 16.0f;
    spawnPoint.height = 16.0f;
    spawnPoint.visible = true;
    objLayer.objects.push_back(spawnPoint);
    
    // Add an enemy object
    TilemapObject enemy;
    enemy.name = "enemy_1";
    enemy.type = "enemy";
    enemy.x = 200.0f;
    enemy.y = 250.0f;
    enemy.width = 32.0f;
    enemy.height = 32.0f;
    enemy.visible = true;
    objLayer.objects.push_back(enemy);
    
    tilemap.objectLayers.push_back(objLayer);
    
    REQUIRE(tilemap.objectLayers.size() == 1);
    REQUIRE(tilemap.objectLayers[0].objects.size() == 2);
    REQUIRE(tilemap.GetObjectLayer("entities") != nullptr);
    REQUIRE(tilemap.GetObjectLayer("entities")->objects[0].name == "player_spawn");
    REQUIRE(tilemap.GetObjectLayer("entities")->objects[1].type == "enemy");
}

TEST_CASE("TilemapComponent - Infinite Map with Chunks") {
    TilemapComponent tilemap;
    
    tilemap.mapWidth = 0;  // 0 for infinite
    tilemap.mapHeight = 0;
    tilemap.tileWidth = 32;
    tilemap.tileHeight = 32;
    tilemap.orientation = TilemapOrientation::Orthogonal;
    tilemap.infinite = true;
    
    TilesetInfo tileset;
    tileset.name = "terrain";
    tileset.firstGID = 1;
    tileset.tileCount = 64;
    tileset.columns = 8;
    tileset.tileWidth = 32;
    tileset.tileHeight = 32;
    tilemap.tilesets.push_back(tileset);
    
    TilemapLayer layer;
    layer.name = "infinite_ground";
    layer.width = 0;
    layer.height = 0;
    layer.visible = true;
    layer.opacity = 1.0f;
    
    // Add chunk at (0, 0)
    TilemapChunk chunk1;
    chunk1.x = 0;
    chunk1.y = 0;
    for (int i = 0; i < TILEMAP_CHUNK_SIZE; ++i) {
        chunk1.tiles[i] = (i % 2 == 0) ? 1 : 2;
    }
    layer.chunks.push_back(chunk1);
    
    // Add chunk at (16, 0)
    TilemapChunk chunk2;
    chunk2.x = 16;
    chunk2.y = 0;
    for (int i = 0; i < TILEMAP_CHUNK_SIZE; ++i) {
        chunk2.tiles[i] = 3;
    }
    layer.chunks.push_back(chunk2);
    
    tilemap.layers.push_back(layer);
    
    REQUIRE(tilemap.infinite == true);
    REQUIRE(tilemap.mapWidth == 0);
    REQUIRE(tilemap.mapHeight == 0);
    REQUIRE(tilemap.layers[0].chunks.size() == 2);
    REQUIRE(tilemap.layers[0].chunks[0].x == 0);
    REQUIRE(tilemap.layers[0].chunks[1].x == 16);
}
