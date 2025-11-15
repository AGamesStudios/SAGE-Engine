#pragma once

#include <atomic>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace SAGE::ECS {

namespace Detail {
    inline std::atomic<size_t> g_ComponentTypeCounter{0};
    inline std::atomic<size_t> g_MaxComponentTypeID{0};

    inline size_t GenerateComponentTypeID() {
        const size_t id = g_ComponentTypeCounter.fetch_add(1, std::memory_order_relaxed);

        size_t expected = g_MaxComponentTypeID.load(std::memory_order_relaxed);
        while (id >= expected && !g_MaxComponentTypeID.compare_exchange_weak(
                   expected, id + 1, std::memory_order_relaxed)) {
            // retry with updated expected
        }

        return id;
    }
} // namespace Detail

/// @brief Get unique ID for component type (compile-time optimized)
template<typename T>
inline size_t GetComponentTypeID() {
    static const size_t id = Detail::GenerateComponentTypeID();
    return id;
}

/// @brief Get maximum registered component type ID (one past the last ID)
inline size_t GetMaxComponentTypeID() {
    return Detail::g_MaxComponentTypeID.load(std::memory_order_relaxed);
}

/// @brief Component metadata for reflection
template<typename T>
struct ComponentTraits {
    using Type = T;
    using Dependencies = std::tuple<>;  // Override for dependencies
    
    static constexpr size_t Size = sizeof(T);
    static constexpr size_t Alignment = alignof(T);
    static constexpr size_t ID = GetComponentTypeID<T>();
    
    static constexpr bool IsTriviallyDestructible = std::is_trivially_destructible_v<T>;
    static constexpr bool IsTriviallyCopyable = std::is_trivially_copyable_v<T>;
};

} // namespace SAGE::ECS
