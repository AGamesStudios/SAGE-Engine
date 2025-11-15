#pragma once

#include "Entity.h"
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>

namespace SAGE::ECS {

/**
 * @file MemoryPool.h
 * @brief Memory pool for component allocation with minimal overhead
 */

/// @brief Pool configuration
struct PoolConfig {
    size_t chunkSize = 4096;        // Bytes per chunk
    size_t initialChunks = 4;       // Initial allocation
    bool allowGrowth = true;        // Grow when full
};

/**
 * @brief Fixed-size memory pool for components
 * 
 * Benefits:
 * - Zero fragmentation
 * - Cache-friendly allocation
 * - Batch deallocation
 * - No per-allocation overhead
 */
template<typename T>
class MemoryPool {
public:
    explicit MemoryPool(const PoolConfig& config = {})
        : m_Config(config)
        , m_ElementsPerChunk(config.chunkSize / sizeof(T))
    {
        if (m_ElementsPerChunk == 0) m_ElementsPerChunk = 1;
        
        // Allocate initial chunks
        for (size_t i = 0; i < config.initialChunks; ++i) {
            AllocateChunk();
        }
    }
    
    ~MemoryPool() {
        Clear();
    }
    
    /// @brief Allocate element from pool
    T* Allocate() {
        // Try to use free slot
        if (!m_FreeSlots.empty()) {
            T* ptr = m_FreeSlots.back();
            m_FreeSlots.pop_back();
            return ptr;
        }
        
        // Try current chunk
        if (m_CurrentChunk && m_CurrentOffset < m_ElementsPerChunk) {
            T* ptr = &m_CurrentChunk[m_CurrentOffset++];
            return ptr;
        }
        
        // Need new chunk
        if (!m_Config.allowGrowth && m_Chunks.size() >= m_Config.initialChunks) {
            return nullptr; // Pool exhausted
        }
        
        AllocateChunk();
        if (m_CurrentChunk) {
            T* ptr = &m_CurrentChunk[m_CurrentOffset++];
            return ptr;
        }
        
        return nullptr;
    }
    
    /// @brief Construct element in pool
    template<typename... Args>
    T* Construct(Args&&... args) {
        T* ptr = Allocate();
        if (ptr) {
            new (ptr) T(std::forward<Args>(args)...);
        }
        return ptr;
    }
    
    /// @brief Return element to pool (does not call destructor!)
    void Deallocate(T* ptr) {
        if (!ptr) return;
        
        // Add to free list
        m_FreeSlots.push_back(ptr);
    }
    
    /// @brief Destroy and return to pool
    void Destroy(T* ptr) {
        if (!ptr) return;
        
        ptr->~T();
        Deallocate(ptr);
    }
    
    /// @brief Clear all allocations
    void Clear() {
        // Call destructors for all allocated elements
        for (auto& chunk : m_Chunks) {
            delete[] chunk;
        }
        
        m_Chunks.clear();
        m_FreeSlots.clear();
        m_CurrentChunk = nullptr;
        m_CurrentOffset = 0;
    }
    
    /// @brief Get total capacity
    size_t GetCapacity() const {
        return m_Chunks.size() * m_ElementsPerChunk;
    }
    
    /// @brief Get used slots
    size_t GetUsed() const {
        size_t total = 0;
        for (size_t i = 0; i < m_Chunks.size() - 1; ++i) {
            total += m_ElementsPerChunk;
        }
        total += m_CurrentOffset;
        total -= m_FreeSlots.size();
        return total;
    }
    
    /// @brief Compact free slots (reduce fragmentation)
    void Compact() {
        // Sort free slots by address
        std::sort(m_FreeSlots.begin(), m_FreeSlots.end());
        
        // Remove duplicates
        m_FreeSlots.erase(
            std::unique(m_FreeSlots.begin(), m_FreeSlots.end()),
            m_FreeSlots.end()
        );
    }
    
private:
    void AllocateChunk() {
        T* chunk = new T[m_ElementsPerChunk];
        m_Chunks.push_back(chunk);
        m_CurrentChunk = chunk;
        m_CurrentOffset = 0;
    }
    
    PoolConfig m_Config;
    size_t m_ElementsPerChunk;
    
    std::vector<T*> m_Chunks;
    std::vector<T*> m_FreeSlots;
    
    T* m_CurrentChunk = nullptr;
    size_t m_CurrentOffset = 0;
};

/**
 * @brief Object pool for reusable game objects
 */
template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initialSize = 64) {
        m_Pool.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            m_Pool.push_back(std::make_unique<T>());
        }
    }
    
    /// @brief Get object from pool
    T* Acquire() {
        if (!m_Available.empty()) {
            T* obj = m_Available.back();
            m_Available.pop_back();
            m_Active.push_back(obj);
            return obj;
        }
        
        // Create new
        m_Pool.push_back(std::make_unique<T>());
        T* obj = m_Pool.back().get();
        m_Active.push_back(obj);
        return obj;
    }
    
    /// @brief Return object to pool
    void Release(T* obj) {
        auto it = std::find(m_Active.begin(), m_Active.end(), obj);
        if (it != m_Active.end()) {
            m_Active.erase(it);
            m_Available.push_back(obj);
        }
    }
    
    /// @brief Release all active objects
    void ReleaseAll() {
        m_Available.insert(m_Available.end(), m_Active.begin(), m_Active.end());
        m_Active.clear();
    }
    
    /// @brief Get active count
    size_t GetActiveCount() const { return m_Active.size(); }
    
    /// @brief Get available count
    size_t GetAvailableCount() const { return m_Available.size(); }
    
private:
    std::vector<std::unique_ptr<T>> m_Pool;
    std::vector<T*> m_Active;
    std::vector<T*> m_Available;
};

} // namespace SAGE::ECS
