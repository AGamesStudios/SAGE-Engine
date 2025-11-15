#include "Pathfinder.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace SAGE {

    Pathfinder::Pathfinder(int width, int height, float tileSize)
        : m_Width(width), m_Height(height), m_TileSize(tileSize) {
        
        m_Grid.resize(width * height, true); // All walkable by default
        
        // Default heuristic: Manhattan distance
        m_Heuristic = [this](int x1, int y1, int x2, int y2) {
            return DefaultHeuristic(x1, y1, x2, y2);
        };
        
        // Default cost: Euclidean distance
        m_CostFunc = [this](int fromX, int fromY, int toX, int toY) {
            return DefaultCost(fromX, fromY, toX, toY);
        };
    }

    void Pathfinder::SetWalkable(int x, int y, bool walkable) {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height) {
            m_Grid[y * m_Width + x] = walkable;
        }
    }

    bool Pathfinder::IsWalkable(int x, int y) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
            return false;
        }
        return m_Grid[y * m_Width + x];
    }

    void Pathfinder::Clear() {
        std::fill(m_Grid.begin(), m_Grid.end(), true);
    }

    void Pathfinder::WorldToGrid(const Vector2& worldPos, int& outX, int& outY) const {
        outX = static_cast<int>(worldPos.x / m_TileSize);
        outY = static_cast<int>(worldPos.y / m_TileSize);
    }

    Vector2 Pathfinder::GridToWorld(int x, int y) const {
        return Vector2(
            x * m_TileSize + m_TileSize * 0.5f,
            y * m_TileSize + m_TileSize * 0.5f
        );
    }

    Pathfinder::Path Pathfinder::FindPath(const Vector2& start, const Vector2& goal,
                                          bool allowDiagonal, bool smoothPath) {
        int startX, startY, goalX, goalY;
        WorldToGrid(start, startX, startY);
        WorldToGrid(goal, goalX, goalY);

        // Проверка валидности
        if (!IsWalkable(startX, startY) || !IsWalkable(goalX, goalY)) {
            return Path(); // Empty path
        }

        if (startX == goalX && startY == goalY) {
            Path p;
            p.waypoints.push_back(start);
            p.found = true;
            return p;
        }

        // A* algorithm
        std::priority_queue<Node*, std::vector<Node*>, NodeCompare> openSet;
        std::unordered_set<Node*, NodeHash, std::equal_to<Node>> closedSet;
        std::unordered_map<int, Node*> allNodes; // For memory cleanup

        Node* startNode = new Node(startX, startY);
        startNode->g = 0;
        startNode->h = m_Heuristic(startX, startY, goalX, goalY);
        startNode->f = startNode->h;
        
        allNodes[startY * m_Width + startX] = startNode;
        openSet.push(startNode);

        Node* goalNode = nullptr;

        while (!openSet.empty()) {
            Node* current = openSet.top();
            openSet.pop();

            // Достигли цели
            if (current->x == goalX && current->y == goalY) {
                goalNode = current;
                break;
            }

            closedSet.insert(current);

            // Проверяем соседей
            std::vector<Node*> neighbors = GetNeighbors(current, allowDiagonal);
            
            for (Node* neighbor : neighbors) {
                if (closedSet.find(neighbor) != closedSet.end()) {
                    continue; // Уже обработан
                }

                float tentativeG = current->g + m_CostFunc(current->x, current->y, neighbor->x, neighbor->y);

                int key = neighbor->y * m_Width + neighbor->x;
                auto it = allNodes.find(key);
                
                Node* existingNode = nullptr;
                if (it != allNodes.end()) {
                    existingNode = it->second;
                }

                if (existingNode == nullptr) {
                    // Новый node
                    neighbor->g = tentativeG;
                    neighbor->h = m_Heuristic(neighbor->x, neighbor->y, goalX, goalY);
                    neighbor->f = neighbor->g + neighbor->h;
                    neighbor->parent = current;
                    
                    allNodes[key] = neighbor;
                    openSet.push(neighbor);
                } else if (tentativeG < existingNode->g) {
                    // Нашли лучший путь
                    existingNode->g = tentativeG;
                    existingNode->f = existingNode->g + existingNode->h;
                    existingNode->parent = current;
                    
                    // Re-add to priority queue (С++ std::priority_queue не поддерживает decrease-key,
                    // поэтому просто добавляем дубликат - он будет обработан с меньшим f)
                    openSet.push(existingNode);
                    
                    delete neighbor; // Удаляем временный neighbor
                } else {
                    delete neighbor; // Не нужен
                }
            }
        }

        Path result;
        if (goalNode) {
            result = ReconstructPath(goalNode);
            result.found = true;
            
            if (smoothPath && result.waypoints.size() > 2) {
                result = SmoothPath(result);
            }
        }

        // Cleanup
        for (auto& pair : allNodes) {
            delete pair.second;
        }

        return result;
    }

    std::vector<Pathfinder::Node*> Pathfinder::GetNeighbors(Node* current, bool allowDiagonal) {
        std::vector<Node*> neighbors;
        
        // 4 cardinal directions
        static const int dx4[] = {0, 1, 0, -1};
        static const int dy4[] = {-1, 0, 1, 0};
        
        // 8 directions (включая диагональные)
        static const int dx8[] = {0, 1, 1, 1, 0, -1, -1, -1};
        static const int dy8[] = {-1, -1, 0, 1, 1, 1, 0, -1};
        
        const int* dx = allowDiagonal ? dx8 : dx4;
        const int* dy = allowDiagonal ? dy8 : dy4;
        int count = allowDiagonal ? 8 : 4;
        
        for (int i = 0; i < count; ++i) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];
            
            if (IsWalkable(nx, ny)) {
                // Для диагонального движения проверяем, что соседние клетки тоже проходимы
                if (allowDiagonal && (dx[i] != 0 && dy[i] != 0)) {
                    if (!IsWalkable(current->x + dx[i], current->y) ||
                        !IsWalkable(current->x, current->y + dy[i])) {
                        continue; // Блокировано corner cutting
                    }
                }
                
                neighbors.push_back(new Node(nx, ny));
            }
        }
        
        return neighbors;
    }

    float Pathfinder::DefaultHeuristic(int x1, int y1, int x2, int y2) const {
        // Manhattan distance
        return static_cast<float>(std::abs(x1 - x2) + std::abs(y1 - y2)) * m_TileSize;
    }

    float Pathfinder::DefaultCost(int fromX, int fromY, int toX, int toY) const {
        // Euclidean distance
        int dx = toX - fromX;
        int dy = toY - fromY;
        return std::sqrt(static_cast<float>(dx * dx + dy * dy)) * m_TileSize;
    }

    Pathfinder::Path Pathfinder::ReconstructPath(Node* goal) {
        Path path;
        Node* current = goal;
        
        std::vector<Vector2> reversed;
        while (current != nullptr) {
            reversed.push_back(GridToWorld(current->x, current->y));
            current = current->parent;
        }
        
        // Reverse to get start → goal
        path.waypoints.assign(reversed.rbegin(), reversed.rend());
        
        // Calculate total cost
        path.totalCost = 0.0f;
        for (size_t i = 1; i < path.waypoints.size(); ++i) {
            Vector2 diff = path.waypoints[i] - path.waypoints[i - 1];
            path.totalCost += diff.Length();
        }
        
        return path;
    }

    Pathfinder::Path Pathfinder::SmoothPath(const Path& path) {
        if (path.waypoints.size() <= 2) {
            return path;
        }

        Path smoothed;
        smoothed.waypoints.push_back(path.waypoints[0]);

        size_t current = 0;
        while (current < path.waypoints.size() - 1) {
            size_t farthest = current + 1;
            
            // Находим самую дальнюю точку с прямой видимостью
            for (size_t i = current + 2; i < path.waypoints.size(); ++i) {
                if (HasLineOfSight(path.waypoints[current], path.waypoints[i])) {
                    farthest = i;
                } else {
                    break;
                }
            }
            
            smoothed.waypoints.push_back(path.waypoints[farthest]);
            current = farthest;
        }

        smoothed.found = path.found;
        
        // Recalculate cost
        smoothed.totalCost = 0.0f;
        for (size_t i = 1; i < smoothed.waypoints.size(); ++i) {
            Vector2 diff = smoothed.waypoints[i] - smoothed.waypoints[i - 1];
            smoothed.totalCost += diff.Length();
        }

        return smoothed;
    }

    bool Pathfinder::HasLineOfSight(const Vector2& from, const Vector2& to) const {
        // Bresenham's line algorithm для проверки LOS
        int x0, y0, x1, y1;
        WorldToGrid(from, x0, y0);
        WorldToGrid(to, x1, y1);

        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        int x = x0;
        int y = y0;

        while (true) {
            if (!IsWalkable(x, y)) {
                return false;
            }

            if (x == x1 && y == y1) {
                break;
            }

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }

        return true;
    }

} // namespace SAGE
