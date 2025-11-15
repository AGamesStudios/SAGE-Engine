#pragma once

#include <vector>
#include <memory>
#include <cassert>

namespace SAGE {

/**
 * @brief Fast object pool allocator
 * 
 * Pre-allocates objects and reuses them instead of new/delete.
 * Reduces heap fragmentation and improves cache locality.
 * 
 * Usage:
 *   ObjectPool<Particle> particlePool(1000);
 *   Particle* p = particlePool.Allocate();
 *   // ... use particle ...
 *   particlePool.Free(p);
 */
template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initialCapacity = 128) {
        Reserve(initialCapacity);
    }
    
    ~ObjectPool() {
        Clear();
    }
    
    // Non-copyable
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    
    /**
     * @brief Pre-allocate objects
     */
    void Reserve(size_t capacity) {
        size_t currentSize = m_Pool.size();
        if (capacity <= currentSize) return;
        
        m_Pool.reserve(capacity);
        for (size_t i = currentSize; i < capacity; ++i) {
            m_Pool.push_back(new T());
            m_FreeList.push_back(m_Pool.back());
        }
    }
    
    /**
     * @brief Get object from pool (O(1))
     */
    T* Allocate() {
        if (m_FreeList.empty()) {
            // Auto-grow if needed
            size_t oldSize = m_Pool.size();
            size_t newSize = oldSize == 0 ? 32 : oldSize * 2;
            Reserve(newSize);
        }
        
        T* obj = m_FreeList.back();
        m_FreeList.pop_back();
        return obj;
    }
    
    /**
     * @brief Return object to pool (O(1))
     */
    void Free(T* obj) {
        if (!obj) return;
        
        #ifdef _DEBUG
        // Check if object belongs to this pool
        bool found = false;
        for (T* poolObj : m_Pool) {
            if (poolObj == obj) {
                found = true;
                break;
            }
        }
        assert(found && "Object doesn't belong to this pool!");
        #endif
        
        m_FreeList.push_back(obj);
    }
    
    /**
     * @brief Allocate and construct with args
     */
    template<typename... Args>
    T* AllocateConstruct(Args&&... args) {
        T* obj = Allocate();
        new (obj) T(std::forward<Args>(args)...); // Placement new
        return obj;
    }
    
    /**
     * @brief Destruct and free
     */
    void FreeDestruct(T* obj) {
        if (obj) {
            obj->~T(); // Manual destructor call
            Free(obj);
        }
    }
    
    /**
     * @brief Clear pool and deallocate all objects
     */
    void Clear() {
        for (T* obj : m_Pool) {
            delete obj;
        }
        m_Pool.clear();
        m_FreeList.clear();
    }
    
    size_t GetCapacity() const { return m_Pool.size(); }
    size_t GetFreeCount() const { return m_FreeList.size(); }
    size_t GetUsedCount() const { return m_Pool.size() - m_FreeList.size(); }
    
private:
    std::vector<T*> m_Pool;      // All allocated objects
    std::vector<T*> m_FreeList;  // Available objects
};

} // namespace SAGE
