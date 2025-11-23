#pragma once

#include "SAGE/Math/Rect.h"
#include "SAGE/Math/Vector2.h"
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace SAGE {

/// QuadTree for efficient spatial partitioning and collision detection
template<typename T>
class QuadTree {
public:
    struct Element {
        Rect bounds;
        T data;
        
        Element(const Rect& b, const T& d) : bounds(b), data(d) {}
    };

    /// Constructor
    /// @param bounds The boundary of this quadtree node
    /// @param maxObjects Maximum objects before subdivision
    /// @param maxLevels Maximum subdivision levels
    QuadTree(const Rect& bounds, int maxObjects = 10, int maxLevels = 5, int level = 0)
        : m_Bounds(bounds)
        , m_MaxObjects(maxObjects)
        , m_MaxLevels(maxLevels)
        , m_Level(level)
    {}

    /// Clear the quadtree
    void Clear() {
        m_Objects.clear();
        
        for (int i = 0; i < 4; ++i) {
            if (m_Nodes[i]) {
                m_Nodes[i]->Clear();
                m_Nodes[i].reset();
            }
        }
    }

    /// Insert an object into the quadtree
    void Insert(const Element& element) {
        if (m_Nodes[0]) {
            int index = GetIndex(element.bounds);
            if (index != -1) {
                m_Nodes[index]->Insert(element);
                return;
            }
        }

        m_Objects.push_back(element);

        if (m_Objects.size() > static_cast<size_t>(m_MaxObjects) && m_Level < m_MaxLevels) {
            if (!m_Nodes[0]) {
                Split();
            }

            auto it = m_Objects.begin();
            while (it != m_Objects.end()) {
                int index = GetIndex(it->bounds);
                if (index != -1) {
                    m_Nodes[index]->Insert(*it);
                    it = m_Objects.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    /// Retrieve all objects that could collide with the given bounds
    std::vector<T> Retrieve(const Rect& bounds) const {
        std::vector<T> returnObjects;
        Retrieve(bounds, returnObjects);
        return returnObjects;
    }

    /// Query all objects in the quadtree
    std::vector<T> QueryAll() const {
        std::vector<T> result;
        QueryAll(result);
        
        // Remove duplicates if any
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        
        return result;
    }

    /// Get the boundary of this node
    const Rect& GetBounds() const { return m_Bounds; }

    /// Get total object count (including subnodes)
    size_t GetTotalCount() const {
        size_t count = m_Objects.size();
        for (int i = 0; i < 4; ++i) {
            if (m_Nodes[i]) {
                count += m_Nodes[i]->GetTotalCount();
            }
        }
        return count;
    }

    /// Debug: Get depth of the tree
    int GetDepth() const {
        if (!m_Nodes[0]) {
            return m_Level;
        }
        
        int maxDepth = m_Level;
        for (int i = 0; i < 4; ++i) {
            if (m_Nodes[i]) {
                maxDepth = std::max(maxDepth, m_Nodes[i]->GetDepth());
            }
        }
        return maxDepth;
    }

private:
    /// Split the node into 4 subnodes
    void Split() {
        float subWidth = m_Bounds.width / 2.0f;
        float subHeight = m_Bounds.height / 2.0f;
        float x = m_Bounds.x;
        float y = m_Bounds.y;

        // Top-right
        m_Nodes[0] = std::make_unique<QuadTree>(
            Rect{x + subWidth, y, subWidth, subHeight},
            m_MaxObjects, m_MaxLevels, m_Level + 1);

        // Top-left
        m_Nodes[1] = std::make_unique<QuadTree>(
            Rect{x, y, subWidth, subHeight},
            m_MaxObjects, m_MaxLevels, m_Level + 1);

        // Bottom-left
        m_Nodes[2] = std::make_unique<QuadTree>(
            Rect{x, y + subHeight, subWidth, subHeight},
            m_MaxObjects, m_MaxLevels, m_Level + 1);

        // Bottom-right
        m_Nodes[3] = std::make_unique<QuadTree>(
            Rect{x + subWidth, y + subHeight, subWidth, subHeight},
            m_MaxObjects, m_MaxLevels, m_Level + 1);
    }

    /// Determine which node the object belongs to
    /// @return -1 if object cannot fit in a child node (straddles boundaries)
    int GetIndex(const Rect& bounds) const {
        int index = -1;

        float verticalMidpoint = m_Bounds.x + m_Bounds.width / 2.0f;
        float horizontalMidpoint = m_Bounds.y + m_Bounds.height / 2.0f;

        // Object can fit in top quadrants
        bool topQuadrant = (bounds.y < horizontalMidpoint && 
                           bounds.y + bounds.height < horizontalMidpoint);
        
        // Object can fit in bottom quadrants
        bool bottomQuadrant = (bounds.y > horizontalMidpoint);

        // Object can fit in left quadrants
        if (bounds.x < verticalMidpoint && 
            bounds.x + bounds.width < verticalMidpoint) {
            if (topQuadrant) {
                index = 1;
            } else if (bottomQuadrant) {
                index = 2;
            }
        }
        // Object can fit in right quadrants
        else if (bounds.x > verticalMidpoint) {
            if (topQuadrant) {
                index = 0;
            } else if (bottomQuadrant) {
                index = 3;
            }
        }

        return index;
    }

    /// Retrieve all objects that could collide with the given bounds
    void Retrieve(const Rect& bounds, std::vector<T>& returnObjects) const {
        int index = GetIndex(bounds);
        
        if (index != -1 && m_Nodes[0]) {
            m_Nodes[index]->Retrieve(bounds, returnObjects);
        }

        // Add objects from this node that intersect with query bounds
        for (const auto& obj : m_Objects) {
            if (obj.bounds.Intersects(bounds)) {
                returnObjects.push_back(obj.data);
            }
        }

        // If bounds straddle boundaries, check all child nodes
        if (index == -1 && m_Nodes[0]) {
            for (int i = 0; i < 4; ++i) {
                if (m_Nodes[i] && m_Nodes[i]->GetBounds().Intersects(bounds)) {
                    m_Nodes[i]->Retrieve(bounds, returnObjects);
                }
            }
        }
    }

    /// Query all objects recursively
    void QueryAll(std::vector<T>& result) const {
        for (const auto& obj : m_Objects) {
            result.push_back(obj.data);
        }
        
        for (int i = 0; i < 4; ++i) {
            if (m_Nodes[i]) {
                m_Nodes[i]->QueryAll(result);
            }
        }
    }

    Rect m_Bounds;
    int m_MaxObjects;
    int m_MaxLevels;
    int m_Level;
    
    std::vector<Element> m_Objects;
    std::unique_ptr<QuadTree> m_Nodes[4];
};

} // namespace SAGE
