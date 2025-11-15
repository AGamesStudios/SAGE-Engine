#pragma once

#include <vector>
#include <memory>
#include <random>
#include <glm/glm.hpp>

namespace SAGE {

// Tile type для dungeon
enum class TileType {
    Empty = 0,
    Floor = 1,
    Wall = 2,
    Door = 3,
    Entrance = 4,
    Exit = 5,
    Treasure = 6
};

// Room в dungeon
struct Room {
    int x, y;
    int width, height;
    
    Room(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    
    glm::vec2 GetCenter() const {
        return glm::vec2(x + width / 2, y + height / 2);
    }
    
    bool Intersects(const Room& other) const {
        return !(x + width < other.x || x > other.x + other.width ||
                 y + height < other.y || y > other.y + other.height);
    }
};

// Corridor между комнатами
struct Corridor {
    glm::vec2 start;
    glm::vec2 end;
    bool horizontal;
};

// Dungeon Generator
class DungeonGenerator {
public:
    DungeonGenerator(int width, int height, int seed = 0);
    
    // Генерация
    void Generate();
    
    // Настройки
    void SetRoomCount(int min, int max) { m_MinRooms = min; m_MaxRooms = max; }
    void SetRoomSize(int min, int max) { m_MinRoomSize = min; m_MaxRoomSize = max; }
    void SetTreasureChance(float chance) { m_TreasureChance = chance; }
    
    // Получение данных
    const std::vector<std::vector<TileType>>& GetTiles() const { return m_Tiles; }
    const std::vector<Room>& GetRooms() const { return m_Rooms; }
    
    TileType GetTile(int x, int y) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return TileType::Empty;
        return m_Tiles[y][x];
    }
    
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    
private:
    void CreateRooms();
    void CreateCorridors();
    void PlaceSpecialTiles();
    void CreateWalls();
    
    bool CanPlaceRoom(const Room& room) const;
    void PlaceRoom(const Room& room);
    void CreateCorridor(const glm::vec2& start, const glm::vec2& end);
    void PlaceDoors();
    
    int m_Width, m_Height;
    std::vector<std::vector<TileType>> m_Tiles;
    std::vector<Room> m_Rooms;
    std::vector<Corridor> m_Corridors;
    
    std::mt19937 m_Random;
    
    int m_MinRooms = 5;
    int m_MaxRooms = 10;
    int m_MinRoomSize = 4;
    int m_MaxRoomSize = 10;
    float m_TreasureChance = 0.3f;
};

// BSP Dungeon Generator (Binary Space Partitioning)
class BSPDungeonGenerator {
public:
    struct BSPNode {
        int x, y, width, height;
        std::unique_ptr<BSPNode> left;
        std::unique_ptr<BSPNode> right;
        std::unique_ptr<Room> room;
        
        BSPNode(int x, int y, int w, int h) 
            : x(x), y(y), width(w), height(h) {}
    };
    
    BSPDungeonGenerator(int width, int height, int seed = 0);
    
    void Generate();
    void SetMinRoomSize(int size) { m_MinRoomSize = size; }
    
    const std::vector<std::vector<TileType>>& GetTiles() const { return m_Tiles; }
    
private:
    void Split(BSPNode* node, int depth);
    void CreateRooms(BSPNode* node);
    void ConnectRooms(BSPNode* left, BSPNode* right);
    
    int m_Width, m_Height;
    std::vector<std::vector<TileType>> m_Tiles;
    std::unique_ptr<BSPNode> m_Root;
    std::mt19937 m_Random;
    int m_MinRoomSize = 4;
    int m_MaxDepth = 4;
};

// Cellular Automata для пещер
class CaveGenerator {
public:
    CaveGenerator(int width, int height, int seed = 0);
    
    void Generate();
    
    void SetFillProbability(float prob) { m_FillProbability = prob; }
    void SetSimulationSteps(int steps) { m_SimulationSteps = steps; }
    
    const std::vector<std::vector<bool>>& GetCells() const { return m_Cells; }
    
    bool IsWall(int x, int y) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return true;
        return m_Cells[y][x];
    }
    
private:
    void InitializeCells();
    void SimulateStep();
    int CountAliveNeighbors(int x, int y) const;
    
    int m_Width, m_Height;
    std::vector<std::vector<bool>> m_Cells;
    std::mt19937 m_Random;
    
    float m_FillProbability = 0.45f;
    int m_SimulationSteps = 5;
};

// Perlin Noise для terrain
class PerlinNoise {
public:
    PerlinNoise(int seed = 0);
    
    float Noise(float x, float y) const;
    float FractalNoise(float x, float y, int octaves = 4, float persistence = 0.5f) const;
    
private:
    float Fade(float t) const { return t * t * t * (t * (t * 6 - 15) + 10); }
    float Lerp(float t, float a, float b) const { return a + t * (b - a); }
    float Grad(int hash, float x, float y) const;
    
    std::vector<int> m_Permutation;
};

// Terrain Generator
class TerrainGenerator {
public:
    enum class BiomeType {
        Ocean,
        Beach,
        Plains,
        Forest,
        Hills,
        Mountains,
        Snow
    };
    
    TerrainGenerator(int width, int height, int seed = 0);
    
    void Generate();
    
    void SetNoiseScale(float scale) { m_NoiseScale = scale; }
    void SetOctaves(int octaves) { m_Octaves = octaves; }
    
    float GetHeight(int x, int y) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return 0.0f;
        return m_HeightMap[y][x];
    }
    
    BiomeType GetBiome(int x, int y) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return BiomeType::Ocean;
        return m_BiomeMap[y][x];
    }
    
    const std::vector<std::vector<float>>& GetHeightMap() const { return m_HeightMap; }
    const std::vector<std::vector<BiomeType>>& GetBiomeMap() const { return m_BiomeMap; }
    
private:
    void GenerateHeightMap();
    void GenerateBiomes();
    BiomeType DetermineBiome(float height, float moisture) const;
    
    int m_Width, m_Height;
    std::vector<std::vector<float>> m_HeightMap;
    std::vector<std::vector<BiomeType>> m_BiomeMap;
    
    PerlinNoise m_Noise;
    float m_NoiseScale = 50.0f;
    int m_Octaves = 4;
};

} // namespace SAGE
