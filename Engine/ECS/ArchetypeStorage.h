#pragma once

#include "EntityV2.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace SAGE::ECS {

/**
 * @file ArchetypeStorage.h
 * @brief Archetype-based entity storage for cache-efficient iteration
 * 
 * Entities grouped by component signature (archetype)
 * - SoA layout: components in separate arrays
 * - Batch operations: process whole archetypes
 * - Fast queries: iterate matching archetypes only
 * - Minimal moving: entities rarely change archetype
 */

/// @brief Component type mask (64 component types max)
using ComponentMask = uint64_t;

/// @brief Get component bit for type
template<typename T>
constexpr ComponentMask GetComponentBit() {
    // Compile-time unique ID per type
    // Can be improved with type registry
    return ComponentMask(1) << (std::hash<const char*>{}(typeid(T).name()) % 64);
}

/// @brief Component array with SoA layout
struct ComponentArray {
    void* data = nullptr;          // Raw component data
    size_t elementSize = 0;        // Size of one component
    size_t elementAlign = 0;       // Alignment requirement
    size_t capacity = 0;           // Allocated capacity
    size_t count = 0;              // Current count
    
    void (*destructor)(void*) = nullptr;  // Component destructor
    
    ~ComponentArray() {
        if (data && destructor) {
            // Call destructors for all components
            for (size_t i = 0; i < count; ++i) {
                destructor(static_cast<uint8_t*>(data) + i * elementSize);
            }
        }
        if (data) {
            ::operator delete(data, std::align_val_t(elementAlign));
        }
    }
    
    void Reserve(size_t newCapacity) {
        if (newCapacity <= capacity) return;
        
        void* newData = ::operator new(newCapacity * elementSize, std::align_val_t(elementAlign));
        
        if (data) {
            std::memcpy(newData, data, count * elementSize);
            ::operator delete(data, std::align_val_t(elementAlign));
        }
        
        data = newData;
        capacity = newCapacity;
    }
    
    void* Get(size_t index) {
        return static_cast<uint8_t*>(data) + index * elementSize;
    }
    
    const void* Get(size_t index) const {
        return static_cast<const uint8_t*>(data) + index * elementSize;
    }
    
    void Add(const void* component) {
        if (count >= capacity) {
            Reserve(capacity == 0 ? 16 : capacity * 2);
        }
        
        void* dest = Get(count++);
        std::memcpy(dest, component, elementSize);
    }
    
    void RemoveSwap(size_t index) {
        if (index >= count) return;
        
        if (destructor) {
            destructor(Get(index));
        }
        
        // Swap with last
        if (index < count - 1) {
            std::memcpy(Get(index), Get(count - 1), elementSize);
        }
        
        --count;
    }
};

/// @brief Archetype: entities with same component signature
struct Archetype {
    ComponentMask signature = 0;                  // Component mask
    std::vector<EntityV2> entities;               // Packed entities
    std::vector<ComponentArray> componentArrays;  // SoA: one array per component type
    
    /// @brief Reserve memory for N entities
    void Reserve(size_t capacity) {
        entities.reserve(capacity);
        for (auto& array : componentArrays) {
            array.Reserve(capacity);
        }
    }
    
    /// @brief Get entity count
    size_t GetEntityCount() const {
        return entities.size();
    }
    
    /// @brief Find entity index in archetype
    size_t FindEntity(EntityV2 entity) const {
        auto it = std::find(entities.begin(), entities.end(), entity);
        return it != entities.end() ? (it - entities.begin()) : SIZE_MAX;
    }
    
    /// @brief Add entity with components
    void AddEntity(EntityV2 entity, const std::vector<const void*>& components) {
        entities.push_back(entity);
        
        for (size_t i = 0; i < components.size() && i < componentArrays.size(); ++i) {
            componentArrays[i].Add(components[i]);
        }
    }
    
    /// @brief Remove entity by index (swap with last)
    void RemoveEntity(size_t index) {
        if (index >= entities.size()) return;
        
        // Swap entity with last
        if (index < entities.size() - 1) {
            entities[index] = entities.back();
        }
        entities.pop_back();
        
        // Swap components
        for (auto& array : componentArrays) {
            array.RemoveSwap(index);
        }
    }
    
    /// @brief Get component data for entity index
    void* GetComponent(size_t entityIndex, size_t componentIndex) {
        if (componentIndex >= componentArrays.size()) return nullptr;
        return componentArrays[componentIndex].Get(entityIndex);
    }
    
    /// @brief Check if signature matches query
    bool Matches(ComponentMask query) const {
        return (signature & query) == query;
    }
};

/// @brief Archetype-based storage manager
class ArchetypeManager {
public:
    /// @brief Find or create archetype with signature
    Archetype* GetOrCreateArchetype(ComponentMask signature) {
        // Linear search (optimize with hashmap if needed)
        for (auto& archetype : m_Archetypes) {
            if (archetype.signature == signature) {
                return &archetype;
            }
        }
        
        // Create new archetype
        Archetype newArchetype;
        newArchetype.signature = signature;
        m_Archetypes.push_back(std::move(newArchetype));
        return &m_Archetypes.back();
    }
    
    /// @brief Find archetype by signature
    Archetype* FindArchetype(ComponentMask signature) {
        for (auto& archetype : m_Archetypes) {
            if (archetype.signature == signature) {
                return &archetype;
            }
        }
        return nullptr;
    }
    
    /// @brief Query archetypes matching component mask
    std::vector<Archetype*> Query(ComponentMask mask) {
        std::vector<Archetype*> results;
        results.reserve(m_Archetypes.size());
        
        for (auto& archetype : m_Archetypes) {
            if (archetype.Matches(mask)) {
                results.push_back(&archetype);
            }
        }
        
        return results;
    }
    
    /// @brief Get all archetypes
    std::vector<Archetype>& GetArchetypes() { return m_Archetypes; }
    const std::vector<Archetype>& GetArchetypes() const { return m_Archetypes; }
    
    /// @brief Clear all archetypes
    void Clear() {
        m_Archetypes.clear();
    }
    
private:
    std::vector<Archetype> m_Archetypes;
};

} // namespace SAGE::ECS
