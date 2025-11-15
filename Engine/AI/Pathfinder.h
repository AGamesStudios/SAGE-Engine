#pragma once

#include "../Math/Vector2.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <memory>

namespace SAGE {

    /**
     * @brief A* Pathfinding для 2D grid navigation
     * 
     * Реализует поиск пути A* с:
     * - Grid-based navigation (плитки проходимы/не проходимы)
     * - Diagonal movement support
     * - Path smoothing (line of sight optimization)
     * - Dynamic obstacle updates
     */
    class Pathfinder {
    public:
        struct Node {
            int x, y;
            float g; // Cost from start
            float h; // Heuristic to goal
            float f; // g + h
            Node* parent;
            
            Node() : x(0), y(0), g(0), h(0), f(0), parent(nullptr) {}
            Node(int x_, int y_) : x(x_), y(y_), g(0), h(0), f(0), parent(nullptr) {}
            
            bool operator==(const Node& other) const {
                return x == other.x && y == other.y;
            }
        };

        struct Path {
            std::vector<Vector2> waypoints;
            float totalCost = 0.0f;
            bool found = false;
            
            bool IsEmpty() const { return waypoints.empty(); }
            size_t Size() const { return waypoints.size(); }
            const Vector2& operator[](size_t i) const { return waypoints[i]; }
        };

        /**
         * @brief Конструктор
         * @param width Ширина grid в тайлах
         * @param height Высота grid в тайлах
         * @param tileSize Размер одного тайла в world units
         */
        Pathfinder(int width, int height, float tileSize = 32.0f);

        /**
         * @brief Установить проходимость тайла
         * @param x Координата X в grid
         * @param y Координата Y в grid
         * @param walkable Проходим ли тайл
         */
        void SetWalkable(int x, int y, bool walkable);

        /**
         * @brief Проверить проходимость тайла
         */
        bool IsWalkable(int x, int y) const;

        /**
         * @brief Найти путь от start к goal (мировые координаты)
         * @param start Стартовая позиция (world space)
         * @param goal Целевая позиция (world space)
         * @param allowDiagonal Разрешить диагональное движение
         * @param smoothPath Применить path smoothing
         * @return Путь (список waypoints)
         */
        Path FindPath(const Vector2& start, const Vector2& goal, 
                     bool allowDiagonal = true, bool smoothPath = true);

        /**
         * @brief Конвертация world coords → grid coords
         */
        void WorldToGrid(const Vector2& worldPos, int& outX, int& outY) const;

        /**
         * @brief Конвертация grid coords → world coords (центр тайла)
         */
        Vector2 GridToWorld(int x, int y) const;

        /**
         * @brief Получить размеры grid
         */
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        float GetTileSize() const { return m_TileSize; }

        /**
         * @brief Очистить всю карту (сделать все тайлы проходимыми)
         */
        void Clear();

        /**
         * @brief Установить эвристику (по умолчанию: Manhattan distance)
         */
        using HeuristicFunc = std::function<float(int x1, int y1, int x2, int y2)>;
        void SetHeuristic(HeuristicFunc heuristic) { m_Heuristic = heuristic; }

        /**
         * @brief Стоимость движения (можно переопределить для разных типов terrain)
         */
        using CostFunc = std::function<float(int fromX, int fromY, int toX, int toY)>;
        void SetCostFunction(CostFunc costFunc) { m_CostFunc = costFunc; }

    private:
        int m_Width;
        int m_Height;
        float m_TileSize;
        std::vector<bool> m_Grid; // Walkable tiles (true = walkable)

        HeuristicFunc m_Heuristic;
        CostFunc m_CostFunc;

        // A* internal
        struct NodeHash {
            size_t operator()(const Node& n) const {
                return std::hash<int>()(n.x) ^ (std::hash<int>()(n.y) << 1);
            }
        };

        struct NodeCompare {
            bool operator()(const Node* a, const Node* b) const {
                return a->f > b->f; // Min heap
            }
        };

        std::vector<Node*> GetNeighbors(Node* current, bool allowDiagonal);
        float DefaultHeuristic(int x1, int y1, int x2, int y2) const;
        float DefaultCost(int fromX, int fromY, int toX, int toY) const;
        Path ReconstructPath(Node* goal);
        Path SmoothPath(const Path& path);
        bool HasLineOfSight(const Vector2& from, const Vector2& to) const;
    };

    /**
     * @brief Navmesh Pathfinding (для сложных карт с полигональными препятствиями)
     * 
     * TODO: Implement в будущем если нужен advanced pathfinding
     */
    class NavMesh {
    public:
        // Placeholder for future navmesh implementation
    };

} // namespace SAGE
