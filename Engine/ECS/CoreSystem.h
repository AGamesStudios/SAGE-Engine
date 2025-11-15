#pragma once

#include "System.h"
#include "Registry.h"
#include "ComponentArray.h"
#include <vector>
#include <cstdint>

namespace SAGE::ECS {

/**
 * @file CoreSystem.h
 * @brief Lightweight base system with pooling and batch processing
 */

/// @brief System configuration for performance tuning
struct SystemConfig {
    size_t initialCapacity = 1024;      // Initial entity buffer size
    size_t batchSize = 64;              // Entities per batch
    bool enablePrefetch = true;         // Hardware prefetching
    bool enableParallel = false;        // Multi-threading (future)
};

/**
 * @brief Base system with optimizations for low-end devices
 * 
 * Features:
 * - Thread-local buffers (zero allocations)
 * - Batch processing
 * - Component caching
 * - Minimal virtual calls
 */
template<typename... Components>
class CoreSystem : public ISystem {
public:
    explicit CoreSystem(const SystemConfig& config = {})
        : m_Config(config)
    {
        m_EntityBuffer.reserve(config.initialCapacity);
    }
    
    virtual ~CoreSystem() = default;
    
    void Update(Registry& registry, float deltaTime) final {
        if (!IsActive()) return;
        
        // Fast path: early exit if no entities
        if (registry.GetEntityCount() == 0) return;
        
        // Gather entities with required components
        GatherEntities(registry);
        
        if (m_EntityBuffer.empty()) return;
        
        // Process in batches for cache efficiency
        ProcessBatches(registry, deltaTime);
        
        // Clear for next frame (no deallocation)
        m_EntityBuffer.clear();
    }
    
protected:
    /// @brief Override this to process a single entity
    virtual void ProcessEntity(Registry& registry, Entity entity, float deltaTime) = 0;
    
    /// @brief Optional: Override for batch processing optimization
    virtual void ProcessBatch(Registry& registry, Entity* entities, size_t count, float deltaTime) {
        // Default: call ProcessEntity for each
        for (size_t i = 0; i < count; ++i) {
            ProcessEntity(registry, entities[i], deltaTime);
        }
    }
    
    /// @brief Get all gathered entities
    const std::vector<Entity>& GetEntities() const { return m_EntityBuffer; }
    
private:
    /// @brief Gather entities with all required components
    void GatherEntities(Registry& registry) {
        m_EntityBuffer.clear();
        
        // Use ComponentArray for fast iteration
        GatherEntitiesImpl<0>(registry);
    }
    
    template<size_t Index>
    void GatherEntitiesImpl(Registry& registry) {
        if constexpr (Index == 0) {
            // First component: iterate over all entities with it
            using FirstComponent = std::tuple_element_t<0, std::tuple<Components...>>;
            
            registry.ForEach<FirstComponent>([this, &registry](Entity entity, const FirstComponent&) {
                // Check if entity has all other components
                if (HasAllComponents<1>(registry, entity)) {
                    m_EntityBuffer.push_back(entity);
                }
            });
        }
    }
    
    template<size_t Index>
    bool HasAllComponents(Registry& registry, Entity entity) {
        if constexpr (Index >= sizeof...(Components)) {
            return true;
        } else {
            using Component = std::tuple_element_t<Index, std::tuple<Components...>>;
            if (!registry.HasComponent<Component>(entity)) {
                return false;
            }
            return HasAllComponents<Index + 1>(registry, entity);
        }
    }
    
    /// @brief Process entities in batches
    void ProcessBatches(Registry& registry, float deltaTime) {
        const size_t totalCount = m_EntityBuffer.size();
        const size_t batchSize = m_Config.batchSize;
        
        for (size_t i = 0; i < totalCount; i += batchSize) {
            const size_t count = std::min(batchSize, totalCount - i);
            ProcessBatch(registry, &m_EntityBuffer[i], count, deltaTime);
        }
    }
    
protected:
    SystemConfig m_Config;
    std::vector<Entity> m_EntityBuffer;
};

/**
 * @brief Simple system that processes entities one by one
 * Use this for simple logic without batch optimization
 */
template<typename... Components>
class SimpleSystem : public CoreSystem<Components...> {
public:
    using CoreSystem<Components...>::CoreSystem;
    
protected:
    // Just implement ProcessEntity in derived class
    void ProcessEntity(Registry& registry, Entity entity, float deltaTime) override = 0;
};

} // namespace SAGE::ECS
