#pragma once

#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>
#include <functional>

namespace SAGE {

// AABB для spatial partitioning
struct AABB {
    glm::vec2 min;
    glm::vec2 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec2& min, const glm::vec2& max) : min(min), max(max) {}
    AABB(float x, float y, float width, float height) 
        : min(x, y), max(x + width, y + height) {}
    
    bool Contains(const glm::vec2& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y;
    }
    
    bool Intersects(const AABB& other) const {
        return !(max.x < other.min.x || min.x > other.max.x ||
                 max.y < other.min.y || min.y > other.max.y);
    }
    
    float GetWidth() const { return max.x - min.x; }
    float GetHeight() const { return max.y - min.y; }
    glm::vec2 GetCenter() const { return (min + max) * 0.5f; }
};

// Объект для spatial partitioning
template<typename T>
struct SpatialObject {
    T data;
    AABB bounds;
    uint32_t id;
    
    SpatialObject() : id(0) {}
    SpatialObject(const T& data, const AABB& bounds, uint32_t id) 
        : data(data), bounds(bounds), id(id) {}
};

// QuadTree Node
template<typename T>
class QuadTreeNode {
public:
    QuadTreeNode(const AABB& bounds, int depth, int maxDepth, int maxObjects)
        : m_Bounds(bounds)
        , m_Depth(depth)
        , m_MaxDepth(maxDepth)
        , m_MaxObjects(maxObjects)
        , m_Divided(false) {}
    
    ~QuadTreeNode() {
        Clear();
    }
    
    void Insert(const SpatialObject<T>& object) {
        if (!m_Bounds.Intersects(object.bounds)) {
            return;
        }
        
        if (!m_Divided) {
            m_Objects.push_back(object);
            
            if (m_Objects.size() > m_MaxObjects && m_Depth < m_MaxDepth) {
                Subdivide();
            }
        } else {
            m_TopLeft->Insert(object);
            m_TopRight->Insert(object);
            m_BottomLeft->Insert(object);
            m_BottomRight->Insert(object);
        }
    }
    
    void Query(const AABB& range, std::vector<SpatialObject<T>>& found) const {
        if (!m_Bounds.Intersects(range)) {
            return;
        }
        
        if (!m_Divided) {
            for (const auto& obj : m_Objects) {
                if (obj.bounds.Intersects(range)) {
                    found.push_back(obj);
                }
            }
        } else {
            m_TopLeft->Query(range, found);
            m_TopRight->Query(range, found);
            m_BottomLeft->Query(range, found);
            m_BottomRight->Query(range, found);
        }
    }
    
    void QueryPoint(const glm::vec2& point, std::vector<SpatialObject<T>>& found) const {
        if (!m_Bounds.Contains(point)) {
            return;
        }
        
        if (!m_Divided) {
            for (const auto& obj : m_Objects) {
                if (obj.bounds.Contains(point)) {
                    found.push_back(obj);
                }
            }
        } else {
            m_TopLeft->QueryPoint(point, found);
            m_TopRight->QueryPoint(point, found);
            m_BottomLeft->QueryPoint(point, found);
            m_BottomRight->QueryPoint(point, found);
        }
    }
    
    void Clear() {
        m_Objects.clear();
        
        if (m_Divided) {
            m_TopLeft.reset();
            m_TopRight.reset();
            m_BottomLeft.reset();
            m_BottomRight.reset();
            m_Divided = false;
        }
    }
    
    int CountObjects() const {
        if (!m_Divided) {
            return static_cast<int>(m_Objects.size());
        }
        
        return m_TopLeft->CountObjects() +
               m_TopRight->CountObjects() +
               m_BottomLeft->CountObjects() +
               m_BottomRight->CountObjects();
    }
    
private:
    void Subdivide() {
        float centerX = m_Bounds.GetCenter().x;
        float centerY = m_Bounds.GetCenter().y;
        
        m_TopLeft = std::make_unique<QuadTreeNode>(
            AABB(m_Bounds.min.x, m_Bounds.min.y, centerX - m_Bounds.min.x, centerY - m_Bounds.min.y),
            m_Depth + 1, m_MaxDepth, m_MaxObjects
        );
        
        m_TopRight = std::make_unique<QuadTreeNode>(
            AABB(centerX, m_Bounds.min.y, m_Bounds.max.x - centerX, centerY - m_Bounds.min.y),
            m_Depth + 1, m_MaxDepth, m_MaxObjects
        );
        
        m_BottomLeft = std::make_unique<QuadTreeNode>(
            AABB(m_Bounds.min.x, centerY, centerX - m_Bounds.min.x, m_Bounds.max.y - centerY),
            m_Depth + 1, m_MaxDepth, m_MaxObjects
        );
        
        m_BottomRight = std::make_unique<QuadTreeNode>(
            AABB(centerX, centerY, m_Bounds.max.x - centerX, m_Bounds.max.y - centerY),
            m_Depth + 1, m_MaxDepth, m_MaxObjects
        );
        
        for (const auto& obj : m_Objects) {
            m_TopLeft->Insert(obj);
            m_TopRight->Insert(obj);
            m_BottomLeft->Insert(obj);
            m_BottomRight->Insert(obj);
        }
        
        m_Objects.clear();
        m_Divided = true;
    }
    
    AABB m_Bounds;
    int m_Depth;
    int m_MaxDepth;
    int m_MaxObjects;
    bool m_Divided;
    
    std::vector<SpatialObject<T>> m_Objects;
    
    std::unique_ptr<QuadTreeNode> m_TopLeft;
    std::unique_ptr<QuadTreeNode> m_TopRight;
    std::unique_ptr<QuadTreeNode> m_BottomLeft;
    std::unique_ptr<QuadTreeNode> m_BottomRight;
};

// QuadTree
template<typename T>
class QuadTree {
public:
    QuadTree(const AABB& bounds, int maxDepth = 8, int maxObjects = 10)
        : m_Root(std::make_unique<QuadTreeNode<T>>(bounds, 0, maxDepth, maxObjects))
        , m_NextId(1) {}
    
    uint32_t Insert(const T& data, const AABB& bounds) {
        uint32_t id = m_NextId++;
        m_Root->Insert(SpatialObject<T>(data, bounds, id));
        return id;
    }
    
    std::vector<SpatialObject<T>> Query(const AABB& range) const {
        std::vector<SpatialObject<T>> found;
        m_Root->Query(range, found);
        return found;
    }
    
    std::vector<SpatialObject<T>> QueryPoint(const glm::vec2& point) const {
        std::vector<SpatialObject<T>> found;
        m_Root->QueryPoint(point, found);
        return found;
    }
    
    void Clear() {
        m_Root->Clear();
        m_NextId = 1;
    }
    
    int CountObjects() const {
        return m_Root->CountObjects();
    }
    
private:
    std::unique_ptr<QuadTreeNode<T>> m_Root;
    uint32_t m_NextId;
};

// Grid-based Spatial Hash
template<typename T>
class SpatialGrid {
public:
    SpatialGrid(float cellSize)
        : m_CellSize(cellSize)
        , m_NextId(1) {}
    
    uint32_t Insert(const T& data, const AABB& bounds) {
        uint32_t id = m_NextId++;
        SpatialObject<T> obj(data, bounds, id);
        
        // Определить все ячейки, которые пересекает объект
        int minX = static_cast<int>(bounds.min.x / m_CellSize);
        int maxX = static_cast<int>(bounds.max.x / m_CellSize);
        int minY = static_cast<int>(bounds.min.y / m_CellSize);
        int maxY = static_cast<int>(bounds.max.y / m_CellSize);
        
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                uint64_t cellKey = GetCellKey(x, y);
                m_Grid[cellKey].push_back(obj);
            }
        }
        
        return id;
    }
    
    std::vector<SpatialObject<T>> Query(const AABB& range) const {
        std::vector<SpatialObject<T>> found;
        std::unordered_set<uint32_t> addedIds;
        
        int minX = static_cast<int>(range.min.x / m_CellSize);
        int maxX = static_cast<int>(range.max.x / m_CellSize);
        int minY = static_cast<int>(range.min.y / m_CellSize);
        int maxY = static_cast<int>(range.max.y / m_CellSize);
        
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                uint64_t cellKey = GetCellKey(x, y);
                auto it = m_Grid.find(cellKey);
                
                if (it != m_Grid.end()) {
                    for (const auto& obj : it->second) {
                        if (addedIds.find(obj.id) == addedIds.end() && 
                            obj.bounds.Intersects(range)) {
                            found.push_back(obj);
                            addedIds.insert(obj.id);
                        }
                    }
                }
            }
        }
        
        return found;
    }
    
    std::vector<SpatialObject<T>> QueryPoint(const glm::vec2& point) const {
        std::vector<SpatialObject<T>> found;
        
        int x = static_cast<int>(point.x / m_CellSize);
        int y = static_cast<int>(point.y / m_CellSize);
        uint64_t cellKey = GetCellKey(x, y);
        
        auto it = m_Grid.find(cellKey);
        if (it != m_Grid.end()) {
            for (const auto& obj : it->second) {
                if (obj.bounds.Contains(point)) {
                    found.push_back(obj);
                }
            }
        }
        
        return found;
    }
    
    void Clear() {
        m_Grid.clear();
        m_NextId = 1;
    }
    
    int CountObjects() const {
        int count = 0;
        for (const auto& pair : m_Grid) {
            count += static_cast<int>(pair.second.size());
        }
        return count;
    }
    
    void SetCellSize(float cellSize) {
        m_CellSize = cellSize;
        Rebuild();
    }
    
private:
    uint64_t GetCellKey(int x, int y) const {
        // Хеш ключ из координат ячейки
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
    }
    
    void Rebuild() {
        // Пересобрать сетку (если изменился размер ячейки)
        std::vector<SpatialObject<T>> allObjects;
        for (const auto& pair : m_Grid) {
            for (const auto& obj : pair.second) {
                allObjects.push_back(obj);
            }
        }
        
        m_Grid.clear();
        
        for (const auto& obj : allObjects) {
            Insert(obj.data, obj.bounds);
        }
    }
    
    float m_CellSize;
    uint32_t m_NextId;
    std::unordered_map<uint64_t, std::vector<SpatialObject<T>>> m_Grid;
};

} // namespace SAGE
