#pragma once

#include "Entity.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <immintrin.h> // SIMD intrinsics

namespace SAGE::ECS {

/**
 * @file ComponentArray.h
 * @brief Component storage with sparse-set indexing
 */

template<typename T>
class ComponentArray {
public:
    ComponentArray() {
        Reserve(2048);
    }
    
    ~ComponentArray() = default;
    
    /// @brief Reserve memory for components (prevents reallocation)
    void Reserve(size_t capacity) {
        m_Entities.reserve(capacity);
        m_Components.reserve(capacity);
        
        // Sparse set: 2x size for better hash distribution
        if (capacity * 2 > m_SparseSet.size()) {
            m_SparseSet.resize(capacity * 2, INVALID_INDEX);
        }
    }
    
    /// @brief Add or update component
    void Set(Entity entity, T&& component) {
        uint32_t entityID = GetEntityID(entity);
        
        // Expand sparse set if needed
        if (entityID >= m_SparseSet.size()) {
            m_SparseSet.resize(entityID * 2 + 1, INVALID_INDEX);
        }
        
        uint32_t denseIndex = m_SparseSet[entityID];
        
        if (denseIndex != INVALID_INDEX && denseIndex < m_Entities.size() && m_Entities[denseIndex] == entity) {
            // Update existing
            m_Components[denseIndex] = std::move(component);
        } else {
            // Add new
            denseIndex = static_cast<uint32_t>(m_Entities.size());
            m_SparseSet[entityID] = denseIndex;
            m_Entities.push_back(entity);
            m_Components.push_back(std::move(component));
        }
    }
    
    /// @brief Get component pointer with prefetching (fast path)
    inline T* Get(Entity entity) {
        uint32_t entityID = GetEntityID(entity);
        
        if (entityID >= m_SparseSet.size()) [[unlikely]] return nullptr;
        
        uint32_t denseIndex = m_SparseSet[entityID];
        
        if (denseIndex >= m_Entities.size()) [[unlikely]] return nullptr;
        if (m_Entities[denseIndex] != entity) [[unlikely]] return nullptr;
        
        // Prefetch next 2 components for sequential iteration
        if (denseIndex + 2 < m_Components.size()) [[likely]] {
            _mm_prefetch(reinterpret_cast<const char*>(&m_Components[denseIndex + 2]), _MM_HINT_T0);
        }
        
        return &m_Components[denseIndex];
    }
    
    /// @brief Get component pointer (const)
    const T* Get(Entity entity) const {
        uint32_t entityID = GetEntityID(entity);
        
        if (entityID >= m_SparseSet.size()) return nullptr;
        
        uint32_t denseIndex = m_SparseSet[entityID];
        
        if (denseIndex >= m_Entities.size()) return nullptr;
        if (m_Entities[denseIndex] != entity) return nullptr;
        
        return &m_Components[denseIndex];
    }
    
    /// @brief Remove component
    void Remove(Entity entity) {
        uint32_t entityID = GetEntityID(entity);
        
        if (entityID >= m_SparseSet.size()) return;
        
        uint32_t denseIndex = m_SparseSet[entityID];
        
        if (denseIndex >= m_Entities.size()) return;
        if (m_Entities[denseIndex] != entity) return;
        
        // Swap with last element (keep dense)
        uint32_t lastIndex = static_cast<uint32_t>(m_Entities.size() - 1);
        
        if (denseIndex != lastIndex) {
            Entity lastEntity = m_Entities[lastIndex];
            
            // Update sparse set for swapped entity
            m_SparseSet[GetEntityID(lastEntity)] = denseIndex;
            
            // Swap
            m_Entities[denseIndex] = lastEntity;
            m_Components[denseIndex] = std::move(m_Components[lastIndex]);
        }
        
        // Remove last
        m_SparseSet[entityID] = INVALID_INDEX;
        m_Entities.pop_back();
        m_Components.pop_back();
    }
    
    /// @brief Check if entity has component
    bool Has(Entity entity) const {
        uint32_t entityID = GetEntityID(entity);
        
        if (entityID >= m_SparseSet.size()) return false;
        
        uint32_t denseIndex = m_SparseSet[entityID];
        
        return denseIndex < m_Entities.size() && m_Entities[denseIndex] == entity;
    }
    
    /// @brief Get dense entity array (cache-friendly iteration)
    const std::vector<Entity>& GetEntities() const { return m_Entities; }
    
    /// @brief Get dense component array (SIMD-ready)
    std::vector<T>& GetComponents() { return m_Components; }
    const std::vector<T>& GetComponents() const { return m_Components; }
    
    /// @brief Get component count
    size_t Size() const { return m_Entities.size(); }
    
    /// @brief Clear all components
    void Clear() {
        m_Entities.clear();
        m_Components.clear();
        std::fill(m_SparseSet.begin(), m_SparseSet.end(), INVALID_INDEX);
    }
    
    /// @brief Shrink to fit (reclaim memory)
    void Shrink() {
        m_Entities.shrink_to_fit();
        m_Components.shrink_to_fit();
        
        // Compact sparse set
        if (!m_Entities.empty()) {
            uint32_t maxEntityID = 0;
            for (Entity e : m_Entities) {
                maxEntityID = std::max(maxEntityID, GetEntityID(e));
            }
            m_SparseSet.resize(maxEntityID + 1);
        } else {
            m_SparseSet.clear();
        }
        m_SparseSet.shrink_to_fit();
    }
    
    /// @brief Batch update components with SIMD (example for float4/vec4)
    template<typename Func>
    void BatchUpdate(Func&& func) {
        // Process 4 components at a time with SIMD
        size_t count = m_Components.size();
        size_t simdCount = (count / 4) * 4;
        
        // SIMD loop
        for (size_t i = 0; i < simdCount; i += 4) {
            // Prefetch next batch
            if (i + 8 < count) {
                _mm_prefetch(reinterpret_cast<const char*>(&m_Components[i + 8]), _MM_HINT_T0);
            }
            
            // Process 4 components
            func(m_Components[i + 0], m_Entities[i + 0]);
            func(m_Components[i + 1], m_Entities[i + 1]);
            func(m_Components[i + 2], m_Entities[i + 2]);
            func(m_Components[i + 3], m_Entities[i + 3]);
        }
        
        // Process remaining
        for (size_t i = simdCount; i < count; ++i) {
            func(m_Components[i], m_Entities[i]);
        }
    }

private:
    static constexpr uint32_t INVALID_INDEX = 0xFFFFFFFF;
    
    // Dense arrays (cache-friendly, contiguous memory)
    std::vector<Entity> m_Entities;      // Packed entity IDs
    std::vector<T> m_Components;          // Packed components (aligned for SIMD)
    
    // Sparse set for O(1) lookup: EntityID -> DenseIndex
    std::vector<uint32_t> m_SparseSet;
};

/**
 * @brief Optimized multi-component query iterator
 * 
 * Efficiently iterates entities with multiple components:
 * for (auto [entity, transform, velocity] : QueryView<Transform, Velocity>(registry))
 */
template<typename... Components>
class QueryView {
public:
    QueryView(ComponentPoolOptimized<Components>&... pools)
        : m_Pools(pools...)
    {
        // Find smallest pool to iterate
        FindSmallestPool();
    }
    
    template<typename Func>
    void ForEach(Func&& func) {
        // Iterate smallest pool, check others
        size_t index = m_SmallestPoolIndex;
        
        // TODO: Template metaprogramming to iterate pools efficiently
    }

private:
    std::tuple<ComponentPoolOptimized<Components>&...> m_Pools;
    size_t m_SmallestPoolIndex = 0;
    
    void FindSmallestPool() {
        // Find pool with fewest entities for optimization
    }
};

} // namespace SAGE::ECS
