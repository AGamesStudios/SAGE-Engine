#include "../TestFramework.h"
#include "ECS/Components/TilemapComponent.h"

using namespace SAGE;
using namespace SAGE::ECS;

TEST_CASE("TilemapComponent - Helper CreateOrthogonal") {
    auto tilemap = TilemapComponent::CreateOrthogonal(20, 15, 32, 32);
    
    REQUIRE(tilemap.mapWidth == 20);
    REQUIRE(tilemap.mapHeight == 15);
    REQUIRE(tilemap.tileWidth == 32);
    REQUIRE(tilemap.tileHeight == 32);
    REQUIRE(tilemap.orientation == TilemapOrientation::Orthogonal);
    REQUIRE(tilemap.renderOrder == TilemapRenderOrder::RightDown);
}

TEST_CASE("TilemapComponent - Helper CreateIsometric") {
    auto tilemap = TilemapComponent::CreateIsometric(16, 16, 64, 32);
    
    REQUIRE(tilemap.mapWidth == 16);
    REQUIRE(tilemap.mapHeight == 16);
    REQUIRE(tilemap.tileWidth == 64);
    REQUIRE(tilemap.tileHeight == 32);
    REQUIRE(tilemap.orientation == TilemapOrientation::Isometric);
}

TEST_CASE("TilemapComponent - Helper CreateStaggered") {
    auto tilemap = TilemapComponent::CreateStaggered(12, 10, 32, 32, 
                                                     TilemapStaggerAxis::Y, 
                                                     TilemapStaggerIndex::Odd);
    
    REQUIRE(tilemap.mapWidth == 12);
    REQUIRE(tilemap.mapHeight == 10);
    REQUIRE(tilemap.orientation == TilemapOrientation::Staggered);
    REQUIRE(tilemap.staggerAxis == TilemapStaggerAxis::Y);
    REQUIRE(tilemap.staggerIndex == TilemapStaggerIndex::Odd);
}

TEST_CASE("TilemapComponent - Helper CreateHexagonal") {
    auto tilemap = TilemapComponent::CreateHexagonal(15, 12, 28, 32, 14,
                                                     TilemapStaggerAxis::X,
                                                     TilemapStaggerIndex::Even);
    
    REQUIRE(tilemap.mapWidth == 15);
    REQUIRE(tilemap.mapHeight == 12);
    REQUIRE(tilemap.orientation == TilemapOrientation::Hexagonal);
    REQUIRE(tilemap.hexSideLength == 14);
    REQUIRE(tilemap.staggerAxis == TilemapStaggerAxis::X);
    REQUIRE(tilemap.staggerIndex == TilemapStaggerIndex::Even);
}

TEST_CASE("TilemapComponent - AddLayer Helper") {
    auto tilemap = TilemapComponent::CreateOrthogonal(10, 8);
    
    auto& layer = tilemap.AddLayer("ground", 1);
    
    REQUIRE(tilemap.layers.size() == 1);
    REQUIRE(layer.name == "ground");
    REQUIRE(layer.width == 10);
    REQUIRE(layer.height == 8);
    REQUIRE(layer.tiles.size() == 80);
    REQUIRE(layer.tiles[0] == 1);
    REQUIRE(layer.visible == true);
    REQUIRE(layer.opacity == 1.0f);
}

TEST_CASE("TilemapComponent - AddTileset Helper") {
    auto tilemap = TilemapComponent::CreateOrthogonal(10, 8, 16, 16);
    
    auto& tileset = tilemap.AddTileset("terrain", 1, 64, 8);
    
    REQUIRE(tilemap.tilesets.size() == 1);
    REQUIRE(tileset.name == "terrain");
    REQUIRE(tileset.firstGID == 1);
    REQUIRE(tileset.tileCount == 64);
    REQUIRE(tileset.columns == 8);
    REQUIRE(tileset.tileWidth == 16);
    REQUIRE(tileset.tileHeight == 16);
}

TEST_CASE("TilemapComponent - Complete Helper Workflow") {
    // Create tilemap
    auto tilemap = TilemapComponent::CreateOrthogonal(16, 12, 32, 32);
    
    // Add tileset
    auto& tileset = tilemap.AddTileset("tiles", 1, 100, 10);
    
    // Add background layer with parallax
    auto& bgLayer = tilemap.AddLayer("background", 5);
    bgLayer.parallaxFactor = {0.5f, 0.5f};
    
    // Add ground layer with collision
    auto& groundLayer = tilemap.AddLayer("ground", 1);
    groundLayer.collision = true;
    
    // Add overlay layer with transparency
    auto& overlayLayer = tilemap.AddLayer("overlay", 0);
    overlayLayer.opacity = 0.6f;
    
    // Verify
    REQUIRE(tilemap.IsValid() == true);
    REQUIRE(tilemap.tilesets.size() == 1);
    REQUIRE(tilemap.layers.size() == 3);
    REQUIRE(tilemap.GetLayer("background") != nullptr);
    REQUIRE(tilemap.GetLayer("ground") != nullptr);
    REQUIRE(tilemap.GetLayer("overlay") != nullptr);
    REQUIRE(tilemap.GetLayer("background")->parallaxFactor.x == 0.5f);
    REQUIRE(tilemap.GetLayer("ground")->collision == true);
    REQUIRE(tilemap.GetLayer("overlay")->opacity == 0.6f);
}

TEST_CASE("TilemapComponent - All Orientations with Helpers") {
    // Orthogonal
    auto ortho = TilemapComponent::CreateOrthogonal(10, 10);
    ortho.AddTileset("tiles", 1, 16, 4);
    ortho.AddLayer("ground", 1);
    REQUIRE(ortho.IsValid() == true);
    REQUIRE(ortho.orientation == TilemapOrientation::Orthogonal);
    
    // Isometric
    auto iso = TilemapComponent::CreateIsometric(10, 10);
    iso.AddTileset("tiles", 1, 16, 4);
    iso.AddLayer("ground", 1);
    REQUIRE(iso.IsValid() == true);
    REQUIRE(iso.orientation == TilemapOrientation::Isometric);
    
    // Staggered
    auto stagger = TilemapComponent::CreateStaggered(10, 10);
    stagger.AddTileset("tiles", 1, 16, 4);
    stagger.AddLayer("ground", 1);
    REQUIRE(stagger.IsValid() == true);
    REQUIRE(stagger.orientation == TilemapOrientation::Staggered);
    
    // Hexagonal
    auto hex = TilemapComponent::CreateHexagonal(10, 10);
    hex.AddTileset("tiles", 1, 16, 4);
    hex.AddLayer("ground", 1);
    REQUIRE(hex.IsValid() == true);
    REQUIRE(hex.orientation == TilemapOrientation::Hexagonal);
}
