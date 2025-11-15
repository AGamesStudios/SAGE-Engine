#include <iostream>
#include "Core/TweenSystem.h"
#include "Core/ProceduralGeneration.h"
#include "Core/LocalizationSystem.h"
#include <glm/glm.hpp>

using namespace SAGE;

void DemoTweenSystem() {
    std::cout << "\n=== TWEEN SYSTEM DEMO ===\n" << std::endl;
    
    auto& tweenMgr = TweenManager::Get();
    
    // Float tween with easing
    float health = 100.0f;
    auto healthTween = tweenMgr.TweenFloat(health, 0.0f, 2.0f, EasingType::QuadOut);
    healthTween->SetOnUpdate([&](float value) {
        std::cout << "Health: " << value << std::endl;
    });
    healthTween->SetOnComplete([]() {
        std::cout << "Health depleted!" << std::endl;
    });
    
    // Vector2 tween (position animation)
    glm::vec2 position(0.0f, 0.0f);
    glm::vec2 target(100.0f, 50.0f);
    auto moveTween = tweenMgr.TweenVector2(position, target, 1.5f, EasingType::ElasticOut);
    moveTween->SetLooping(true);
    
    // Color tween (fade effect)
    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 transparent(1.0f, 1.0f, 1.0f, 0.0f);
    auto fadeTween = tweenMgr.TweenColor(color, transparent, 1.0f, EasingType::SineInOut);
    
    // Simulate updates
    float deltaTime = 0.016f;  // 60 FPS
    for (int i = 0; i < 120; ++i) {  // 2 seconds
        tweenMgr.Update(deltaTime);
        
        if (i % 30 == 0) {
            std::cout << "Frame " << i << " - Position: (" 
                      << position.x << ", " << position.y << ")" << std::endl;
        }
    }
    
    // Tween sequence example
    TweenSequence sequence;
    
    float x = 0.0f;
    auto tween1 = std::make_shared<FloatTween>(x, 10.0f, 0.5f, EasingType::QuadIn);
    auto tween2 = std::make_shared<FloatTween>(x, 20.0f, 0.5f, EasingType::BounceOut);
    auto tween3 = std::make_shared<FloatTween>(x, 0.0f, 1.0f, EasingType::ElasticOut);
    
    sequence.Add(tween1);
    sequence.Add(tween2);
    sequence.Add(tween3);
    
    std::cout << "\n--- Sequence Example ---" << std::endl;
    for (int i = 0; i < 125; ++i) {  // 2 seconds
        sequence.Update(deltaTime);
        if (i % 25 == 0) {
            std::cout << "X: " << x << std::endl;
        }
    }
}

void DemoProceduralGeneration() {
    std::cout << "\n=== PROCEDURAL GENERATION DEMO ===\n" << std::endl;
    
    // Dungeon generation
    std::cout << "--- Dungeon Generator ---" << std::endl;
    DungeonGenerator dungeon(80, 40, 12345);
    dungeon.SetRoomCount(5, 10);
    dungeon.SetRoomSize(4, 10);
    dungeon.SetTreasureChance(0.3f);
    dungeon.Generate();
    
    auto& tiles = dungeon.GetTiles();
    std::cout << "Generated dungeon: " << tiles[0].size() << "x" << tiles.size() << std::endl;
    std::cout << "Rooms: " << dungeon.GetRooms().size() << std::endl;
    
    // Print small portion of dungeon
    std::cout << "\nDungeon preview (top-left 40x20):" << std::endl;
    for (int y = 0; y < 20 && y < tiles.size(); ++y) {
        for (int x = 0; x < 40 && x < tiles[y].size(); ++x) {
            char c = ' ';
            switch (tiles[y][x]) {
                case TileType::Empty: c = ' '; break;
                case TileType::Floor: c = '.'; break;
                case TileType::Wall: c = '#'; break;
                case TileType::Door: c = '+'; break;
                case TileType::Entrance: c = 'E'; break;
                case TileType::Exit: c = 'X'; break;
                case TileType::Treasure: c = 'T'; break;
            }
            std::cout << c;
        }
        std::cout << std::endl;
    }
    
    // Cave generation
    std::cout << "\n--- Cave Generator ---" << std::endl;
    CaveGenerator cave(60, 30, 54321);
    cave.Generate();
    
    auto& caveTiles = cave.GetTiles();
    std::cout << "Generated cave: " << caveTiles[0].size() << "x" << caveTiles.size() << std::endl;
    
    // Terrain generation
    std::cout << "\n--- Terrain Generator ---" << std::endl;
    TerrainGenerator terrain(100, 50, 99999);
    terrain.Generate();
    
    auto& heightMap = terrain.GetHeightMap();
    auto& biomeMap = terrain.GetBiomeMap();
    
    std::cout << "Terrain size: " << heightMap[0].size() << "x" << heightMap.size() << std::endl;
    
    // Print terrain preview with biome colors
    std::cout << "\nTerrain preview (50x25):" << std::endl;
    for (int y = 0; y < 25 && y < biomeMap.size(); ++y) {
        for (int x = 0; x < 50 && x < biomeMap[y].size(); ++x) {
            char c = '?';
            switch (biomeMap[y][x]) {
                case TerrainGenerator::BiomeType::Ocean: c = '~'; break;
                case TerrainGenerator::BiomeType::Beach: c = ':'; break;
                case TerrainGenerator::BiomeType::Plains: c = '.'; break;
                case TerrainGenerator::BiomeType::Forest: c = 'T'; break;
                case TerrainGenerator::BiomeType::Hills: c = 'n'; break;
                case TerrainGenerator::BiomeType::Mountains: c = 'A'; break;
                case TerrainGenerator::BiomeType::Snow: c = '*'; break;
            }
            std::cout << c;
        }
        std::cout << std::endl;
    }
}

void DemoLocalizationSystem() {
    std::cout << "\n=== LOCALIZATION SYSTEM DEMO ===\n" << std::endl;
    
    auto& loc = LocalizationSystem::Get();
    
    // Note: In a real application, you would have JSON files like:
    // assets/localization/en.json
    // assets/localization/ru.json
    
    // For this demo, we'll show how to use the system
    std::cout << "Localization System initialized." << std::endl;
    std::cout << "\nUsage example:" << std::endl;
    std::cout << "1. Load language files:" << std::endl;
    std::cout << "   loc.LoadLanguage(\"en\", \"assets/localization/en.json\");" << std::endl;
    std::cout << "   loc.LoadLanguage(\"ru\", \"assets/localization/ru.json\");" << std::endl;
    
    std::cout << "\n2. Set fonts for languages:" << std::endl;
    std::cout << "   loc.SetFontForLanguage(\"en\", \"assets/fonts/Roboto.ttf\");" << std::endl;
    std::cout << "   loc.SetFontForLanguage(\"ru\", \"assets/fonts/RobotoCondensed.ttf\");" << std::endl;
    
    std::cout << "\n3. Set active language:" << std::endl;
    std::cout << "   loc.SetLanguage(\"ru\");" << std::endl;
    
    std::cout << "\n4. Get localized strings:" << std::endl;
    std::cout << "   std::string start = loc.GetString(\"ui.menu.start\");" << std::endl;
    std::cout << "   std::string welcome = loc.GetString(\"ui.welcome\", {\"Player\"});" << std::endl;
    std::cout << "   std::string score = loc.GetString(\"game.score\", {\"1500\"});" << std::endl;
    
    std::cout << "\n5. Example JSON structure (en.json):" << std::endl;
    std::cout << R"({
  "language": {
    "name": "English",
    "code": "en"
  },
  "ui": {
    "menu": {
      "start": "Start Game",
      "continue": "Continue",
      "settings": "Settings",
      "quit": "Quit"
    },
    "welcome": "Welcome, {0}!",
    "player_level": "Level {0} {1}"
  },
  "game": {
    "score": "Score: {0}",
    "health": "HP: {0}/{1}",
    "game_over": "Game Over"
  }
})" << std::endl;
    
    std::cout << "\n6. Example JSON structure (ru.json):" << std::endl;
    std::cout << R"({
  "language": {
    "name": "Русский",
    "code": "ru"
  },
  "ui": {
    "menu": {
      "start": "Начать игру",
      "continue": "Продолжить",
      "settings": "Настройки",
      "quit": "Выход"
    },
    "welcome": "Добро пожаловать, {0}!",
    "player_level": "Уровень {0} {1}"
  },
  "game": {
    "score": "Счёт: {0}",
    "health": "ХП: {0}/{1}",
    "game_over": "Игра окончена"
  }
})" << std::endl;
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "   SAGE ENGINE UTILITY SYSTEMS DEMO   " << std::endl;
    std::cout << "======================================" << std::endl;
    
    DemoTweenSystem();
    DemoProceduralGeneration();
    DemoLocalizationSystem();
    
    std::cout << "\n======================================" << std::endl;
    std::cout << "           DEMO COMPLETED             " << std::endl;
    std::cout << "======================================\n" << std::endl;
    
    return 0;
}
