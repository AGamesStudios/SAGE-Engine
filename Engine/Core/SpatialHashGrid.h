#pragma once

#include "GameObject.h"
#include <vector>
#include <unordered_map>
#include <cmath>

namespace SAGE {

/**
 * @brief Spatial hash grid for fast collision detection
 * 
 * Divides space into cells and only checks collisions within same/nearby cells.
 * Reduces O(n²) to O(n) for well-distributed objects.
 */
class SpatialHashGrid {
public:
    SpatialHashGrid(float cellSize = 128.0f) : m_CellSize(cellSize) {}
    
    /**
     * @brief Clear and rebuild grid with current objects
     */
    void Rebuild(const std::vector<GameObject*>& objects) {
        m_Grid.clear();
        m_Grid.reserve(objects.size() * 2);
        
        for (GameObject* obj : objects) {
            if (!obj || !obj->active) continue;
            Insert(obj);
        }
    }
    
    /**
     * @brief Get potentially colliding objects near target
     */
    std::vector<GameObject*> QueryNearby(GameObject* target) const {
        if (!target) return {};
        
        std::vector<GameObject*> result;
        result.reserve(32); // Reserve for typical case
        
        // Get cells that object spans
        int minCellX, minCellY, maxCellX, maxCellY;
        GetCellRange(target, minCellX, minCellY, maxCellX, maxCellY);
        
        // Check all cells object spans
        for (int cy = minCellY; cy <= maxCellY; ++cy) {
            for (int cx = minCellX; cx <= maxCellX; ++cx) {
                int64_t key = GetCellKey(cx, cy);
                auto it = m_Grid.find(key);
                if (it != m_Grid.end()) {
                    for (GameObject* obj : it->second) {
                        if (obj != target) {
                            result.push_back(obj);
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
    /**
     * @brief Get all objects in radius (optimized circle query)
     */
    std::vector<GameObject*> QueryRadius(float centerX, float centerY, float radius) const {
        std::vector<GameObject*> result;
        result.reserve(64);
        
        // Get cell range for bounding box
        int minCellX = static_cast<int>(std::floor((centerX - radius) / m_CellSize));
        int minCellY = static_cast<int>(std::floor((centerY - radius) / m_CellSize));
        int maxCellX = static_cast<int>(std::floor((centerX + radius) / m_CellSize));
        int maxCellY = static_cast<int>(std::floor((centerY + radius) / m_CellSize));
        
        float radiusSq = radius * radius;
        
        for (int cy = minCellY; cy <= maxCellY; ++cy) {
            for (int cx = minCellX; cx <= maxCellX; ++cx) {
                int64_t key = GetCellKey(cx, cy);
                auto it = m_Grid.find(key);
                if (it != m_Grid.end()) {
                    for (GameObject* obj : it->second) {
                        float dx = obj->x - centerX;
                        float dy = obj->y - centerY;
                        if (dx * dx + dy * dy <= radiusSq) {
                            result.push_back(obj);
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
    float GetCellSize() const { return m_CellSize; }
    void SetCellSize(float size) { m_CellSize = size; }
    size_t GetObjectCount() const {
        size_t count = 0;
        for (const auto& pair : m_Grid) {
            count += pair.second.size();
        }
        return count;
    }
    
private:
    float m_CellSize;
    std::unordered_map<int64_t, std::vector<GameObject*>> m_Grid;
    
    void Insert(GameObject* obj) {
        int minCellX, minCellY, maxCellX, maxCellY;
        GetCellRange(obj, minCellX, minCellY, maxCellX, maxCellY);
        
        // Insert into all cells object spans
        for (int cy = minCellY; cy <= maxCellY; ++cy) {
            for (int cx = minCellX; cx <= maxCellX; ++cx) {
                int64_t key = GetCellKey(cx, cy);
                m_Grid[key].push_back(obj);
            }
        }
    }
    
    void GetCellRange(GameObject* obj, int& minX, int& minY, int& maxX, int& maxY) const {
        float halfW = obj->width * 0.5f;
        float halfH = obj->height * 0.5f;
        
        minX = static_cast<int>(std::floor((obj->x - halfW) / m_CellSize));
        minY = static_cast<int>(std::floor((obj->y - halfH) / m_CellSize));
        maxX = static_cast<int>(std::floor((obj->x + halfW) / m_CellSize));
        maxY = static_cast<int>(std::floor((obj->y + halfH) / m_CellSize));
    }
    
    int64_t GetCellKey(int x, int y) const {
        // Cantor pairing function for unique key
        return static_cast<int64_t>(x) * 1000000LL + static_cast<int64_t>(y);
    }
};

} // namespace SAGE
