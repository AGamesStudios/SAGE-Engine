#include "ProceduralGeneration.h"
#include <algorithm>
#include <cmath>

namespace SAGE {

// DungeonGenerator implementation
DungeonGenerator::DungeonGenerator(int width, int height, int seed)
    : m_Width(width)
    , m_Height(height)
    , m_Random(seed == 0 ? std::random_device{}() : seed) {
    
    m_Tiles.resize(height, std::vector<TileType>(width, TileType::Empty));
}

void DungeonGenerator::Generate() {
    // Очистить
    for (auto& row : m_Tiles) {
        std::fill(row.begin(), row.end(), TileType::Empty);
    }
    m_Rooms.clear();
    m_Corridors.clear();
    
    CreateRooms();
    CreateCorridors();
    CreateWalls();
    PlaceDoors();
    PlaceSpecialTiles();
}

void DungeonGenerator::CreateRooms() {
    std::uniform_int_distribution<> roomCountDist(m_MinRooms, m_MaxRooms);
    int roomCount = roomCountDist(m_Random);
    
    int maxAttempts = roomCount * 10;
    int attempts = 0;
    
    while (m_Rooms.size() < static_cast<size_t>(roomCount) && attempts < maxAttempts) {
        std::uniform_int_distribution<> sizeDist(m_MinRoomSize, m_MaxRoomSize);
        std::uniform_int_distribution<> xDist(1, m_Width - m_MaxRoomSize - 2);
        std::uniform_int_distribution<> yDist(1, m_Height - m_MaxRoomSize - 2);
        
        int w = sizeDist(m_Random);
        int h = sizeDist(m_Random);
        int x = xDist(m_Random);
        int y = yDist(m_Random);
        
        Room newRoom(x, y, w, h);
        
        if (CanPlaceRoom(newRoom)) {
            PlaceRoom(newRoom);
            m_Rooms.push_back(newRoom);
        }
        
        attempts++;
    }
}

bool DungeonGenerator::CanPlaceRoom(const Room& room) const {
    for (const auto& existingRoom : m_Rooms) {
        if (room.Intersects(existingRoom)) {
            return false;
        }
    }
    return true;
}

void DungeonGenerator::PlaceRoom(const Room& room) {
    for (int y = room.y; y < room.y + room.height; ++y) {
        for (int x = room.x; x < room.x + room.width; ++x) {
            if (x >= 0 && x < m_Width && y >= 0 && y < m_Height) {
                m_Tiles[y][x] = TileType::Floor;
            }
        }
    }
}

void DungeonGenerator::CreateCorridors() {
    for (size_t i = 1; i < m_Rooms.size(); ++i) {
        glm::vec2 prev = m_Rooms[i - 1].GetCenter();
        glm::vec2 curr = m_Rooms[i].GetCenter();
        
        CreateCorridor(prev, curr);
    }
}

void DungeonGenerator::CreateCorridor(const glm::vec2& start, const glm::vec2& end) {
    int x = static_cast<int>(start.x);
    int y = static_cast<int>(start.y);
    
    // Horizontal corridor
    while (x != static_cast<int>(end.x)) {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height) {
            if (m_Tiles[y][x] == TileType::Empty) {
                m_Tiles[y][x] = TileType::Floor;
            }
        }
        x += (x < end.x) ? 1 : -1;
    }
    
    // Vertical corridor
    while (y != static_cast<int>(end.y)) {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height) {
            if (m_Tiles[y][x] == TileType::Empty) {
                m_Tiles[y][x] = TileType::Floor;
            }
        }
        y += (y < end.y) ? 1 : -1;
    }
}

void DungeonGenerator::CreateWalls() {
    for (int y = 0; y < m_Height; ++y) {
        for (int x = 0; x < m_Width; ++x) {
            if (m_Tiles[y][x] == TileType::Floor) {
                // Проверить соседей
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;
                        
                        if (nx >= 0 && nx < m_Width && ny >= 0 && ny < m_Height) {
                            if (m_Tiles[ny][nx] == TileType::Empty) {
                                m_Tiles[ny][nx] = TileType::Wall;
                            }
                        }
                    }
                }
            }
        }
    }
}

void DungeonGenerator::PlaceDoors() {
    // Поместить двери на границах комнат и коридоров
    for (int y = 1; y < m_Height - 1; ++y) {
        for (int x = 1; x < m_Width - 1; ++x) {
            if (m_Tiles[y][x] == TileType::Floor) {
                int wallCount = 0;
                if (m_Tiles[y-1][x] == TileType::Wall) wallCount++;
                if (m_Tiles[y+1][x] == TileType::Wall) wallCount++;
                if (m_Tiles[y][x-1] == TileType::Wall) wallCount++;
                if (m_Tiles[y][x+1] == TileType::Wall) wallCount++;
                
                if (wallCount == 2) {
                    std::uniform_real_distribution<> dist(0.0, 1.0);
                    if (dist(m_Random) < 0.3) {  // 30% шанс двери
                        m_Tiles[y][x] = TileType::Door;
                    }
                }
            }
        }
    }
}

void DungeonGenerator::PlaceSpecialTiles() {
    if (m_Rooms.empty()) return;
    
    // Entrance в первой комнате
    auto& firstRoom = m_Rooms[0];
    glm::vec2 entrance = firstRoom.GetCenter();
    m_Tiles[static_cast<int>(entrance.y)][static_cast<int>(entrance.x)] = TileType::Entrance;
    
    // Exit в последней комнате
    auto& lastRoom = m_Rooms.back();
    glm::vec2 exit = lastRoom.GetCenter();
    m_Tiles[static_cast<int>(exit.y)][static_cast<int>(exit.x)] = TileType::Exit;
    
    // Treasure в случайных комнатах
    std::uniform_real_distribution<> treasureDist(0.0, 1.0);
    for (size_t i = 1; i < m_Rooms.size() - 1; ++i) {
        if (treasureDist(m_Random) < m_TreasureChance) {
            auto& room = m_Rooms[i];
            glm::vec2 treasure = room.GetCenter();
            m_Tiles[static_cast<int>(treasure.y)][static_cast<int>(treasure.x)] = TileType::Treasure;
        }
    }
}

// PerlinNoise implementation
PerlinNoise::PerlinNoise(int seed) {
    m_Permutation.resize(512);
    
    // Инициализация permutation table
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    std::shuffle(p.begin(), p.end(), rng);
    
    for (int i = 0; i < 256; ++i) {
        m_Permutation[i] = p[i];
        m_Permutation[256 + i] = p[i];
    }
}

float PerlinNoise::Noise(float x, float y) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    
    x -= std::floor(x);
    y -= std::floor(y);
    
    float u = Fade(x);
    float v = Fade(y);
    
    int A = m_Permutation[X] + Y;
    int B = m_Permutation[X + 1] + Y;
    
    return Lerp(v, 
        Lerp(u, Grad(m_Permutation[A], x, y), Grad(m_Permutation[B], x - 1, y)),
        Lerp(u, Grad(m_Permutation[A + 1], x, y - 1), Grad(m_Permutation[B + 1], x - 1, y - 1))
    );
}

float PerlinNoise::FractalNoise(float x, float y, int octaves, float persistence) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        total += Noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

float PerlinNoise::Grad(int hash, float x, float y) const {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

// TerrainGenerator implementation
TerrainGenerator::TerrainGenerator(int width, int height, int seed)
    : m_Width(width)
    , m_Height(height)
    , m_Noise(seed) {
    
    m_HeightMap.resize(height, std::vector<float>(width, 0.0f));
    m_BiomeMap.resize(height, std::vector<BiomeType>(width, BiomeType::Plains));
}

void TerrainGenerator::Generate() {
    GenerateHeightMap();
    GenerateBiomes();
}

void TerrainGenerator::GenerateHeightMap() {
    for (int y = 0; y < m_Height; ++y) {
        for (int x = 0; x < m_Width; ++x) {
            float nx = static_cast<float>(x) / m_NoiseScale;
            float ny = static_cast<float>(y) / m_NoiseScale;
            
            m_HeightMap[y][x] = (m_Noise.FractalNoise(nx, ny, m_Octaves) + 1.0f) * 0.5f;
        }
    }
}

void TerrainGenerator::GenerateBiomes() {
    PerlinNoise moistureNoise(12345);
    
    for (int y = 0; y < m_Height; ++y) {
        for (int x = 0; x < m_Width; ++x) {
            float height = m_HeightMap[y][x];
            
            float nx = static_cast<float>(x) / m_NoiseScale;
            float ny = static_cast<float>(y) / m_NoiseScale;
            float moisture = (moistureNoise.FractalNoise(nx * 2.0f, ny * 2.0f, 3) + 1.0f) * 0.5f;
            
            m_BiomeMap[y][x] = DetermineBiome(height, moisture);
        }
    }
}

TerrainGenerator::BiomeType TerrainGenerator::DetermineBiome(float height, float moisture) const {
    if (height < 0.3f) return BiomeType::Ocean;
    if (height < 0.35f) return BiomeType::Beach;
    if (height < 0.5f) {
        return moisture > 0.5f ? BiomeType::Forest : BiomeType::Plains;
    }
    if (height < 0.7f) return BiomeType::Hills;
    if (height < 0.85f) return BiomeType::Mountains;
    return BiomeType::Snow;
}

} // namespace SAGE
