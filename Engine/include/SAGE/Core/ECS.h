#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>

namespace SAGE::ECS {

// =========================================================
// Entity handle (index + version packed into 32 bits)
// =========================================================
using Entity = uint32_t;
constexpr Entity kInvalidEntity = 0;

namespace detail {
    constexpr uint32_t kIndexBits = 24;
    constexpr uint32_t kIndexMask = (1u << kIndexBits) - 1u;
    constexpr uint32_t kVersionShift = kIndexBits;
    constexpr uint32_t kInvalidSparse = std::numeric_limits<uint32_t>::max();

    inline uint32_t DecodeIndex(Entity e) { return e & kIndexMask; }
    inline uint32_t DecodeVersion(Entity e) { return e >> kVersionShift; }
    inline Entity Encode(uint32_t index, uint32_t version) {
        return (version << kVersionShift) | index;
    }

    // Static Component Type ID Generator
    inline uint32_t GetNextComponentTypeID() {
        static uint32_t typeID = 0;
        return typeID++;
    }

    template<typename T>
    inline uint32_t GetComponentTypeID() {
        static uint32_t typeID = GetNextComponentTypeID();
        return typeID;
    }
} // namespace detail

struct EntityData {
    uint32_t version = 1;
    bool alive = false;
};

// =========================================================
// Component pools (sparse set)
// =========================================================
struct IPool {
    virtual ~IPool() = default;
    virtual void Remove(Entity e) = 0;
    virtual bool Contains(Entity e) const = 0;
    virtual void Clear() = 0;
    virtual size_t Size() const = 0;
    virtual const std::vector<Entity>& Entities() const = 0;
};

template<typename T>
class ComponentPool : public IPool {
public:
    using OnRemoveCallback = std::function<void(Entity, T&)>;

    void SetOnRemove(OnRemoveCallback cb) { m_OnRemove = std::move(cb); }

    template<typename... Args>
    T& Emplace(Entity e, Args&&... args) {
        EnsureSparse(e);
        const uint32_t denseIndex = static_cast<uint32_t>(m_Dense.size());
        m_Sparse[detail::DecodeIndex(e)] = denseIndex;
        m_Entities.push_back(e);
        m_Dense.emplace_back(std::forward<Args>(args)...);
        return m_Dense.back();
    }

    bool Contains(Entity e) const override {
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Sparse.size()) {
            return false;
        }
        const uint32_t denseIndex = m_Sparse[idx];
        return denseIndex != detail::kInvalidSparse
            && denseIndex < m_Dense.size()
            && m_Entities[denseIndex] == e;
    }

    T* Get(Entity e) {
        // Direct access optimization: assume caller checked Contains or is sure
        // But for safety in Get(), we check bounds via sparse array
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Sparse.size()) return nullptr;
        
        const uint32_t denseIndex = m_Sparse[idx];
        if (denseIndex == detail::kInvalidSparse || denseIndex >= m_Dense.size() || m_Entities[denseIndex] != e) {
            return nullptr;
        }
        return &m_Dense[denseIndex];
    }

    const T* Get(Entity e) const {
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Sparse.size()) return nullptr;

        const uint32_t denseIndex = m_Sparse[idx];
        if (denseIndex == detail::kInvalidSparse || denseIndex >= m_Dense.size() || m_Entities[denseIndex] != e) {
            return nullptr;
        }
        return &m_Dense[denseIndex];
    }

    void Remove(Entity e) override {
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Sparse.size()) return;

        const uint32_t denseIndex = m_Sparse[idx];
        if (denseIndex == detail::kInvalidSparse || denseIndex >= m_Dense.size() || m_Entities[denseIndex] != e) {
            return;
        }

        // Callback before removal
        if (m_OnRemove) {
            m_OnRemove(e, m_Dense[denseIndex]);
        }

        const uint32_t lastIndex = static_cast<uint32_t>(m_Dense.size() - 1);

        if (denseIndex != lastIndex) {
            Entity lastEntity = m_Entities[lastIndex];
            std::swap(m_Dense[denseIndex], m_Dense[lastIndex]);
            std::swap(m_Entities[denseIndex], m_Entities[lastIndex]);
            m_Sparse[detail::DecodeIndex(lastEntity)] = denseIndex;
        }

        m_Dense.pop_back();
        m_Entities.pop_back();
        m_Sparse[idx] = detail::kInvalidSparse;
    }

    void Clear() override {
        if (m_OnRemove) {
            for (size_t i = 0; i < m_Entities.size(); ++i) {
                m_OnRemove(m_Entities[i], m_Dense[i]);
            }
        }
        m_Dense.clear();
        m_Entities.clear();
        m_Sparse.clear();
    }

    size_t Size() const override { return m_Dense.size(); }
    const std::vector<Entity>& Entities() const override { return m_Entities; }

private:
    void EnsureSparse(Entity e) {
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Sparse.size()) {
            m_Sparse.resize(idx + 1, detail::kInvalidSparse);
        }
    }

    std::vector<T> m_Dense;
    std::vector<Entity> m_Entities;
    std::vector<uint32_t> m_Sparse;
    OnRemoveCallback m_OnRemove;
};

// =========================================================
// Registry / World
// =========================================================
class Registry {
public:
    Registry() {
        // Reserve index 0 as invalid
        m_Entities.push_back({});
    }

    Entity CreateEntity() {
        uint32_t index = 0;
        if (!m_FreeList.empty()) {
            index = m_FreeList.back();
            m_FreeList.pop_back();
        } else {
            index = static_cast<uint32_t>(m_Entities.size());
            m_Entities.push_back({});
        }

        EntityData& data = m_Entities[index];
        data.alive = true;
        Entity e = detail::Encode(index, data.version);
        ++m_AliveCount;
        return e;
    }

    bool IsAlive(Entity e) const {
        const uint32_t idx = detail::DecodeIndex(e);
        if (idx >= m_Entities.size()) {
            return false;
        }
        const EntityData& data = m_Entities[idx];
        return data.alive && data.version == detail::DecodeVersion(e);
    }

    void DestroyEntity(Entity e) {
        if (!IsAlive(e)) {
            return;
        }

        // Remove from all pools
        // Optimization: We could track which pools an entity is in, but iterating all pools is safe for now
        for (auto& pool : m_Pools) {
            if (pool) pool->Remove(e);
        }

        const uint32_t idx = detail::DecodeIndex(e);
        EntityData& data = m_Entities[idx];
        data.alive = false;
        ++data.version; // invalidate stale handles
        m_FreeList.push_back(idx);
        if (m_AliveCount > 0) {
            --m_AliveCount;
        }
    }

    void Destroy(Entity e) {
        DestroyEntity(e);
    }

    void Clear() {
        for (auto& pool : m_Pools) {
            if (pool) pool->Clear();
        }
        // Do not clear m_Pools vector to preserve allocated pools
        
        m_Entities.clear();
        m_FreeList.clear();
        m_AliveCount = 0;
        
        // Reserve index 0 as invalid
        m_Entities.push_back({});
    }

    size_t AliveCount() const { return m_AliveCount; }

    void ForEachEntity(std::function<void(Entity)> fn) {
        for (size_t i = 1; i < m_Entities.size(); ++i) {
            if (m_Entities[i].alive) {
                Entity e = detail::Encode(static_cast<uint32_t>(i), m_Entities[i].version);
                fn(e);
            }
        }
    }

    template<typename T>
    using ComponentCallback = std::function<void(Entity, T&)>;

    template<typename T>
    void SetOnComponentRemoved(ComponentCallback<T> cb) {
        GetOrCreatePool<T>().SetOnRemove(std::move(cb));
    }

    template<typename T, typename... Args>
    T& Add(Entity e, Args&&... args) {
        static_assert(std::is_default_constructible_v<T> || sizeof...(Args) > 0, "Component must be constructible");
        auto& pool = GetOrCreatePool<T>();
        if (pool.Contains(e)) {
            return *pool.Get(e);
        }
        return pool.Emplace(e, std::forward<Args>(args)...);
    }

    template<typename T>
    bool Has(Entity e) const {
        const auto* pool = GetPool<T>();
        return pool && pool->Contains(e);
    }

    template<typename T>
    T* Get(Entity e) {
        auto* pool = GetPool<T>();
        return pool ? pool->Get(e) : nullptr;
    }

    template<typename T>
    const T* Get(Entity e) const {
        const auto* pool = GetPool<T>();
        return pool ? pool->Get(e) : nullptr;
    }

    template<typename T>
    void Remove(Entity e) {
        auto* pool = GetPool<T>();
        if (pool) {
            pool->Remove(e);
        }
    }

    // Iterate entities with a required component set
    template<typename... Components, typename Fn>
    void ForEach(Fn&& fn) {
        static_assert(sizeof...(Components) > 0, "ForEach requires at least one component");

        auto* smallest = GetSmallestPool<Components...>();
        if (!smallest) {
            return;
        }

        const auto& entities = smallest->Entities();
        for (Entity e : entities) {
            // Check if entity is still valid (might have been destroyed during iteration if not careful, 
            // but generally we assume alive in pool implies alive in registry unless deferred)
            // However, pool might contain stale entities if we don't sync perfectly, 
            // but our DestroyEntity removes from all pools immediately.
            
            if (HasAll<Components...>(e)) {
                fn(e, *Get<Components>(e)...);
            }
        }
    }

    // View API for cleaner iteration
    template<typename... Components>
    auto View() {
        return [this](auto func) {
            this->ForEach<Components...>(func);
        };
    }

private:
    template<typename T>
    ComponentPool<T>* GetPool() {
        const uint32_t typeId = detail::GetComponentTypeID<T>();
        if (typeId >= m_Pools.size() || !m_Pools[typeId]) {
            return nullptr;
        }
        return static_cast<ComponentPool<T>*>(m_Pools[typeId].get());
    }

    template<typename T>
    const ComponentPool<T>* GetPool() const {
        const uint32_t typeId = detail::GetComponentTypeID<T>();
        if (typeId >= m_Pools.size() || !m_Pools[typeId]) {
            return nullptr;
        }
        return static_cast<const ComponentPool<T>*>(m_Pools[typeId].get());
    }

    template<typename T>
    ComponentPool<T>& GetOrCreatePool() {
        const uint32_t typeId = detail::GetComponentTypeID<T>();
        if (typeId >= m_Pools.size()) {
            m_Pools.resize(typeId + 1);
        }
        if (!m_Pools[typeId]) {
            m_Pools[typeId] = std::make_unique<ComponentPool<T>>();
        }
        return *static_cast<ComponentPool<T>*>(m_Pools[typeId].get());
    }

    template<typename T>
    bool HasOne(Entity e) const {
        return Has<T>(e);
    }

    template<typename T, typename U, typename... Rest>
    bool HasOne(Entity e) const {
        return Has<T>(e) && HasOne<U, Rest...>(e);
    }

    template<typename... Cs>
    bool HasAll(Entity e) const {
        return HasOne<Cs...>(e);
    }

    template<typename T>
    IPool* Smallest(IPool* current) {
        IPool* candidate = GetPool<T>();
        if (!candidate) {
            return nullptr;
        }
        if (!current || candidate->Size() < current->Size()) {
            current = candidate;
        }
        return current;
    }

    template<typename T, typename U, typename... Rest>
    IPool* Smallest(IPool* current) {
        IPool* candidate = GetPool<T>();
        if (!candidate) {
            return nullptr;
        }
        if (!current || candidate->Size() < current->Size()) {
            current = candidate;
        }
        return Smallest<U, Rest...>(current);
    }

    template<typename... Cs>
    IPool* GetSmallestPool() {
        IPool* pool = nullptr;
        pool = Smallest<Cs...>(pool);
        return pool;
    }

private:
    std::vector<EntityData> m_Entities;
    std::vector<uint32_t> m_FreeList;
    // Optimized: Vector instead of map for O(1) access
    std::vector<std::unique_ptr<IPool>> m_Pools;
    size_t m_AliveCount = 0;
};

// =========================================================
// Systems & scheduler
// =========================================================
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void Tick(Registry& registry, float deltaTime) = 0;
    virtual void FixedTick(Registry& /*registry*/, float /*fixedDeltaTime*/) {}
};

class SystemScheduler {
public:
    template<typename TSystem, typename... Args>
    TSystem& AddSystem(Args&&... args) {
        static_assert(std::is_base_of_v<ISystem, TSystem>, "System must inherit from ISystem");
        auto sys = std::make_unique<TSystem>(std::forward<Args>(args)...);
        auto& ref = *sys;
        m_Systems.push_back(std::move(sys));
        return ref;
    }

    void UpdateAll(Registry& registry, float deltaTime) {
        for (auto& sys : m_Systems) {
            sys->Tick(registry, deltaTime);
        }
    }

    void FixedUpdateAll(Registry& registry, float fixedDeltaTime) {
        for (auto& sys : m_Systems) {
            sys->FixedTick(registry, fixedDeltaTime);
        }
    }

    void Clear() { m_Systems.clear(); }

private:
    std::vector<std::unique_ptr<ISystem>> m_Systems;
};

} // namespace SAGE::ECS
