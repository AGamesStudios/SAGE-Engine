#include "../TestFramework.h"
#include "Resources/TilemapLoader.h"
#include "ECS/Components/TilemapComponent.h"
#include <fstream>

using namespace SAGE;
using namespace SAGE::ECS;

TEST_CASE("TilemapLoader - LoadCSV Basic") {
    // Create temporary CSV file
    const char* csvData = 
        "1,2,3,4\n"
        "5,6,7,8\n"
        "9,10,11,12\n";
    
    std::ofstream outFile("test_tilemap.csv");
    outFile << csvData;
    outFile.close();

    TilemapComponent tilemap;
    bool success = TilemapLoader::LoadCSV("test_tilemap.csv", tilemap);

    REQUIRE(success == true);
    REQUIRE(tilemap.mapWidth == 4);
    REQUIRE(tilemap.mapHeight == 3);
    REQUIRE(tilemap.layers.size() == 1);
    REQUIRE(tilemap.layers[0].tiles.size() == 12);
    REQUIRE(tilemap.layers[0].tiles[0] == 1);
    REQUIRE(tilemap.layers[0].tiles[11] == 12);

    // Cleanup
    std::remove("test_tilemap.csv");
}

TEST_CASE("TilemapLoader - LoadJSON Basic") {
    const char* jsonData = R"({
        "width": 3,
        "height": 2,
        "tilewidth": 32,
        "tileheight": 32,
        "tilesets": [
            {
                "name": "terrain",
                "firstgid": 1,
                "tilewidth": 32,
                "tileheight": 32,
                "columns": 8,
                "tilecount": 64,
                "image": "terrain.png"
            }
        ],
        "layers": [
            {
                "name": "ground",
                "type": "tilelayer",
                "width": 3,
                "height": 2,
                "visible": true,
                "opacity": 1.0,
                "data": [1, 2, 3, 4, 5, 6]
            }
        ]
    })";

    std::ofstream outFile("test_tilemap.json");
    outFile << jsonData;
    outFile.close();

    TilemapComponent tilemap;
    bool success = TilemapLoader::LoadJSON("test_tilemap.json", tilemap);

    REQUIRE(success == true);
    REQUIRE(tilemap.mapWidth == 3);
    REQUIRE(tilemap.mapHeight == 2);
    REQUIRE(tilemap.tileWidth == 32);
    REQUIRE(tilemap.tileHeight == 32);
    REQUIRE(tilemap.tilesets.size() == 1);
    REQUIRE(tilemap.tilesets[0].name == "terrain");
    REQUIRE(tilemap.tilesets[0].firstGID == 1);
    REQUIRE(tilemap.tilesets[0].texturePath.find("terrain.png") != std::string::npos);
    REQUIRE(tilemap.layers.size() == 1);
    REQUIRE(tilemap.layers[0].name == "ground");
    REQUIRE(tilemap.layers[0].tiles.size() == 6);

    std::remove("test_tilemap.json");
}

TEST_CASE("TilemapLoader - LoadJSON with Collision Property") {
    const char* jsonData = R"({
        "width": 2,
        "height": 2,
        "tilewidth": 16,
        "tileheight": 16,
        "tilesets": [],
        "layers": [
            {
                "name": "walls",
                "type": "tilelayer",
                "width": 2,
                "height": 2,
                "visible": true,
                "opacity": 1.0,
                "data": [1, 1, 1, 1],
                "properties": [
                    {"name": "collision", "value": true}
                ]
            }
        ]
    })";

    std::ofstream outFile("test_collision.json");
    outFile << jsonData;
    outFile.close();

    TilemapComponent tilemap;
    bool success = TilemapLoader::LoadJSON("test_collision.json", tilemap);

    REQUIRE(success == true);
    REQUIRE(tilemap.layers[0].collision == true);

    std::remove("test_collision.json");
}
