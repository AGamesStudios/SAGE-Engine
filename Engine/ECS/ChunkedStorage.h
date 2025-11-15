#pragma once

#include "Entity.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
#include <algorithm>

namespace SAGE::ECS {

/**
 * @file ChunkedStorage.h
 * @brief Chunk-based entity storage for efficient memory access
 */

/// @brief Chunk size in bytes
constexpr size_t CHUNK_SIZE = 16384;

/// @brief Cache line size for alignment
constexpr size_t CACHE_LINE_SIZE = 64;

/// @brief Component array metadata
struct ComponentArrayInfo {
    std::type_index type;
    size_t size;           // sizeof(T)
    size_t alignment;      // alignof(T)
    size_t offset;         // Offset in chunk
    
    // Destructor function pointer (for non-trivial types)
    using DestructorFn = void(*)(void*);
    DestructorFn destructor = nullptr;
    
    ComponentArrayInfo(std::type_index t, size_t s, size_t a, DestructorFn d = nullptr)
        : type(t), size(s), alignment(a), offset(0), destructor(d) {}
};

/// @brief Archetype ID (hash of component types)
using ArchetypeID = uint64_t;

/**
 * @brief Memory chunk storing entities with same component signature
 */
class Chunk {
public:
    Chunk(size_t entityCapacity, const std::vector<ComponentArrayInfo>& componentInfo)
        : m_EntityCapacity(entityCapacity)
        , m_EntityCount(0)
        , m_ComponentInfo(componentInfo)
    {
        // Allocate cache-line aligned memory for optimal access
        m_Memory = static_cast<uint8_t*>(std::aligned_alloc(CACHE_LINE_SIZE, CHUNK_SIZE));
        std::memset(m_Memory, 0, CHUNK_SIZE);
        
        // Calculate cache-aligned offsets for each component array
        size_t offset = entityCapacity * sizeof(Entity);
        offset = AlignOffset(offset, CACHE_LINE_SIZE); // Align to cache line
        
        for (auto& info : m_ComponentInfo) {
            // Ensure each component array starts on cache line boundary
            offset = AlignOffset(offset, std::max(info.alignment, CACHE_LINE_SIZE));
            const_cast<ComponentArrayInfo&>(info).offset = offset;
            offset += entityCapacity * info.size;
        }
    }
    
    ~Chunk() {
        // Call destructors for non-trivial components
        for (size_t i = 0; i < m_EntityCount; ++i) {
            for (const auto& info : m_ComponentInfo) {
                if (info.destructor) {
                    void* component = GetComponentPtr(i, info);
                    info.destructor(component);
                }
            }
        }
        std::free(m_Memory);
    }
    
    // Non-copyable, movable
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&& other) noexcept
        : m_Memory(other.m_Memory)
        , m_EntityCapacity(other.m_EntityCapacity)
        , m_EntityCount(other.m_EntityCount)
        , m_ComponentInfo(std::move(other.m_ComponentInfo))
    {
        other.m_Memory = nullptr;
    }
    
    /// @brief Check if chunk has space for more entities
    bool HasSpace() const { return m_EntityCount < m_EntityCapacity; }
    
    /// @brief Get number of entities in chunk
    size_t GetEntityCount() const { return m_EntityCount; }
    
    /// @brief Get entity at index
    Entity GetEntity(size_t index) const {
        return reinterpret_cast<Entity*>(m_Memory)[index];
    }
    
    /// @brief Get component pointer for entity at index
    template<typename T>
    T* GetComponent(size_t index, std::type_index type) {
        for (const auto& info : m_ComponentInfo) {
            if (info.type == type) {
                return reinterpret_cast<T*>(m_Memory + info.offset + index * sizeof(T));
            }
        }
        return nullptr;
    }
    
    /// @brief Get raw component array for SIMD operations
    template<typename T>
    T* GetComponentArray(std::type_index type) {
        for (const auto& info : m_ComponentInfo) {
            if (info.type == type) {
                return reinterpret_cast<T*>(m_Memory + info.offset);
            }
        }
        return nullptr;
    }
    
    /// @brief Add entity with components to chunk
    /// @return Index of added entity in chunk
    size_t AddEntity(Entity entity, const std::vector<std::pair<std::type_index, void*>>& components) {
        if (!HasSpace()) {
            throw std::runtime_error("Chunk is full");
        }
        
        size_t index = m_EntityCount++;
        
        // Set entity ID
        reinterpret_cast<Entity*>(m_Memory)[index] = entity;
        
        // Copy components
        for (const auto& [type, data] : components) {
            for (const auto& info : m_ComponentInfo) {
                if (info.type == type) {
                    void* dest = m_Memory + info.offset + index * info.size;
                    std::memcpy(dest, data, info.size);
                    break;
                }
            }
        }
        
        return index;
    }
    
    /// @brief Remove entity at index (swap with last)
    void RemoveEntity(size_t index) {
        if (index >= m_EntityCount) return;
        
        size_t lastIndex = m_EntityCount - 1;
        
        if (index != lastIndex) {
            // Swap entity ID
            Entity* entities = reinterpret_cast<Entity*>(m_Memory);
            entities[index] = entities[lastIndex];
            
            // Swap all components
            for (const auto& info : m_ComponentInfo) {
                void* src = m_Memory + info.offset + lastIndex * info.size;
                void* dest = m_Memory + info.offset + index * info.size;
                
                // Call destructor on destination
                if (info.destructor) {
                    info.destructor(dest);
                }
                
                std::memcpy(dest, src, info.size);
            }
        }
        
        // Call destructors on last element
        for (const auto& info : m_ComponentInfo) {
            if (info.destructor) {
                void* component = m_Memory + info.offset + lastIndex * info.size;
                info.destructor(component);
            }
        }
        
        --m_EntityCount;
    }

private:
    uint8_t* m_Memory = nullptr;
    size_t m_EntityCapacity;
    size_t m_EntityCount;
    std::vector<ComponentArrayInfo> m_ComponentInfo;
    
    static size_t AlignOffset(size_t offset, size_t alignment) {
        return (offset + alignment - 1) & ~(alignment - 1);
    }
    
    void* GetComponentPtr(size_t index, const ComponentArrayInfo& info) const {
        return m_Memory + info.offset + index * info.size;
    }
};

/**
 * @brief Stores all entities with same component signature
 */
class Archetype {
public:
    Archetype(ArchetypeID id, const std::vector<ComponentArrayInfo>& componentInfo)
        : m_ID(id)
        , m_ComponentInfo(componentInfo)
    {
        // Calculate entities per chunk based on component sizes + alignment
        size_t componentSizeTotal = 0;
        for (const auto& info : componentInfo) {
            // Account for alignment padding
            componentSizeTotal += info.size + CACHE_LINE_SIZE;
        }
        
        // Reserve space for entity IDs + components + metadata
        size_t usableSize = CHUNK_SIZE - 512; // Reserve 512 bytes for metadata
        m_EntitiesPerChunk = usableSize / (sizeof(Entity) + componentSizeTotal);
        
        // Clamp to reasonable range (16-1024 entities per chunk)
        m_EntitiesPerChunk = std::clamp(m_EntitiesPerChunk, size_t(16), size_t(1024));
        
        // Create first chunk
        m_Chunks.push_back(std::make_unique<Chunk>(m_EntitiesPerChunk, m_ComponentInfo));
    }
    
    ArchetypeID GetID() const { return m_ID; }
    
    /// @brief Add entity to archetype
    void AddEntity(Entity entity, const std::vector<std::pair<std::type_index, void*>>& components) {
        // Find chunk with space or create new one
        Chunk* targetChunk = nullptr;
        for (auto& chunk : m_Chunks) {
            if (chunk->HasSpace()) {
                targetChunk = chunk.get();
                break;
            }
        }
        
        if (!targetChunk) {
            m_Chunks.push_back(std::make_unique<Chunk>(m_EntitiesPerChunk, m_ComponentInfo));
            targetChunk = m_Chunks.back().get();
        }
        
        size_t index = targetChunk->AddEntity(entity, components);
        m_EntityToChunk[entity] = {m_Chunks.size() - 1, index};
    }
    
    /// @brief Remove entity from archetype
    void RemoveEntity(Entity entity) {
        auto it = m_EntityToChunk.find(entity);
        if (it == m_EntityToChunk.end()) return;
        
        auto [chunkIndex, entityIndex] = it->second;
        m_Chunks[chunkIndex]->RemoveEntity(entityIndex);
        m_EntityToChunk.erase(it);
        
        // TODO: Compact chunks if too many are empty
    }
    
    /// @brief Iterate over all entities with callback
    template<typename Func>
    void ForEach(Func&& func) {
        for (auto& chunk : m_Chunks) {
            size_t count = chunk->GetEntityCount();
            for (size_t i = 0; i < count; ++i) {
                Entity entity = chunk->GetEntity(i);
                func(entity, chunk.get(), i);
            }
        }
    }
    
    /// @brief Get all chunks (for batch processing)
    const std::vector<std::unique_ptr<Chunk>>& GetChunks() const { return m_Chunks; }

private:
    ArchetypeID m_ID;
    std::vector<ComponentArrayInfo> m_ComponentInfo;
    std::vector<std::unique_ptr<Chunk>> m_Chunks;
    size_t m_EntitiesPerChunk;
    
    // Quick lookup: Entity -> (ChunkIndex, EntityIndexInChunk)
    std::unordered_map<Entity, std::pair<size_t, size_t>> m_EntityToChunk;
};

} // namespace SAGE::ECS
