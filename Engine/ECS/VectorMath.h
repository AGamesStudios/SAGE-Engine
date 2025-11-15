#pragma once

#include "ECS/Entity.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include <immintrin.h> // AVX2/SSE
#include <cstdint>
#include <cstring>

namespace SAGE::ECS {

/**
 * @file VectorMath.h
 * @brief Batch vector operations for ECS components
 */

// Compiler-specific force inline
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE inline
#endif

// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#endif

/// @brief Transform batch operations
struct TransformOps {
    /// @brief Update positions with velocity
    FORCE_INLINE static void UpdatePositions(
        Vector2* __restrict positions,
        const Vector2* __restrict velocities,
        float deltaTime,
        size_t count)
    {
#if defined(__AVX2__)
        // AVX2 path: Process 4 Vector2 (8 floats) simultaneously
        size_t simdCount = (count / 4) * 4;
        const __m256 dt = _mm256_set1_ps(deltaTime);
        
        for (size_t i = 0; i < simdCount; i += 4) {
            // Prefetch next batch (64 bytes ahead)
            if (LIKELY(i + 8 < count)) {
                _mm_prefetch(reinterpret_cast<const char*>(&positions[i + 8]), _MM_HINT_T0);
                _mm_prefetch(reinterpret_cast<const char*>(&velocities[i + 8]), _MM_HINT_T0);
            }
            
            // Load 4 positions (x1,y1,x2,y2,x3,y3,x4,y4)
            __m256 pos = _mm256_loadu_ps(reinterpret_cast<const float*>(&positions[i]));
            
            // Load 4 velocities
            __m256 vel = _mm256_loadu_ps(reinterpret_cast<const float*>(&velocities[i]));
            
            // FMA: position += velocity * deltaTime (single instruction!)
            #if defined(__FMA__)
                pos = _mm256_fmadd_ps(vel, dt, pos);
            #else
                pos = _mm256_add_ps(pos, _mm256_mul_ps(vel, dt));
            #endif
            
            // Store result
            _mm256_storeu_ps(reinterpret_cast<float*>(&positions[i]), pos);
        }
        
        // Scalar remainder
        for (size_t i = simdCount; i < count; ++i) {
            positions[i].x += velocities[i].x * deltaTime;
            positions[i].y += velocities[i].y * deltaTime;
        }
#elif defined(__SSE2__)
        // Process 2 Vector2 (4 floats) at once with SSE2
        size_t simdCount = (count / 2) * 2;
        __m128 dt = _mm_set1_ps(deltaTime);
        
        for (size_t i = 0; i < simdCount; i += 2) {
            __m128 pos = _mm_loadu_ps(reinterpret_cast<const float*>(&positions[i]));
            __m128 vel = _mm_loadu_ps(reinterpret_cast<const float*>(&velocities[i]));
            
            __m128 delta = _mm_mul_ps(vel, dt);
            pos = _mm_add_ps(pos, delta);
            
            _mm_storeu_ps(reinterpret_cast<float*>(&positions[i]), pos);
        }
        
        for (size_t i = simdCount; i < count; ++i) {
            positions[i].x += velocities[i].x * deltaTime;
            positions[i].y += velocities[i].y * deltaTime;
        }
#else
        // Scalar fallback
        for (size_t i = 0; i < count; ++i) {
            positions[i].x += velocities[i].x * deltaTime;
            positions[i].y += velocities[i].y * deltaTime;
        }
#endif
    }
    
    /// @brief Apply gravity to all velocities
    static void ApplyGravity(
        Vector2* velocities,
        Vector2 gravity,
        float deltaTime,
        size_t count)
    {
#if defined(__AVX2__)
        size_t simdCount = (count / 4) * 4;
        __m256 gravX = _mm256_set1_ps(gravity.x * deltaTime);
        __m256 gravY = _mm256_set1_ps(gravity.y * deltaTime);
        
        for (size_t i = 0; i < simdCount; i += 4) {
            // Load velocities
            __m256 vel = _mm256_loadu_ps(reinterpret_cast<float*>(&velocities[i]));
            
            // Deinterleave x and y components (complex, simplified here)
            // vel += gravity * deltaTime
            __m256 grav = _mm256_setr_ps(
                gravity.x * deltaTime, gravity.y * deltaTime,
                gravity.x * deltaTime, gravity.y * deltaTime,
                gravity.x * deltaTime, gravity.y * deltaTime,
                gravity.x * deltaTime, gravity.y * deltaTime
            );
            
            vel = _mm256_add_ps(vel, grav);
            _mm256_storeu_ps(reinterpret_cast<float*>(&velocities[i]), vel);
        }
        
        for (size_t i = simdCount; i < count; ++i) {
            velocities[i].x += gravity.x * deltaTime;
            velocities[i].y += gravity.y * deltaTime;
        }
#else
        for (size_t i = 0; i < count; ++i) {
            velocities[i].x += gravity.x * deltaTime;
            velocities[i].y += gravity.y * deltaTime;
        }
#endif
    }
    
    /// @brief Scale all vectors by scalar
    static void ScaleVectors(
        Vector2* vectors,
        float scale,
        size_t count)
    {
#if defined(__AVX2__)
        size_t simdCount = (count / 4) * 4;
        __m256 s = _mm256_set1_ps(scale);
        
        for (size_t i = 0; i < simdCount; i += 4) {
            __m256 vec = _mm256_loadu_ps(reinterpret_cast<float*>(&vectors[i]));
            vec = _mm256_mul_ps(vec, s);
            _mm256_storeu_ps(reinterpret_cast<float*>(&vectors[i]), vec);
        }
        
        for (size_t i = simdCount; i < count; ++i) {
            vectors[i].x *= scale;
            vectors[i].y *= scale;
        }
#else
        for (size_t i = 0; i < count; ++i) {
            vectors[i].x *= scale;
            vectors[i].y *= scale;
        }
#endif
    }
};

/// @brief Distance calculations
struct DistanceOps {
    /// @brief Calculate squared distances between two arrays of points
    static void CalculateSquaredDistances(
        const Vector2* points1,
        const Vector2* points2,
        float* outDistances,
        size_t count)
    {
#if defined(__AVX2__)
        size_t simdCount = (count / 4) * 4;
        
        for (size_t i = 0; i < simdCount; i += 4) {
            // Load points (4 x Vector2)
            __m256 p1 = _mm256_loadu_ps(reinterpret_cast<const float*>(&points1[i]));
            __m256 p2 = _mm256_loadu_ps(reinterpret_cast<const float*>(&points2[i]));
            
            // delta = p2 - p1
            __m256 delta = _mm256_sub_ps(p2, p1);
            
            // delta * delta
            __m256 sq = _mm256_mul_ps(delta, delta);
            
            // Horizontal add: (dx^2 + dy^2) for each pair
            // Simplified: just sum adjacent pairs
            // Full implementation would use _mm256_hadd_ps
            
            // For now, scalar finish
            alignas(32) float temp[8];
            _mm256_store_ps(temp, sq);
            
            outDistances[i + 0] = temp[0] + temp[1];
            outDistances[i + 1] = temp[2] + temp[3];
            outDistances[i + 2] = temp[4] + temp[5];
            outDistances[i + 3] = temp[6] + temp[7];
        }
        
        for (size_t i = simdCount; i < count; ++i) {
            float dx = points2[i].x - points1[i].x;
            float dy = points2[i].y - points1[i].y;
            outDistances[i] = dx * dx + dy * dy;
        }
#else
        for (size_t i = 0; i < count; ++i) {
            float dx = points2[i].x - points1[i].x;
            float dy = points2[i].y - points1[i].y;
            outDistances[i] = dx * dx + dy * dy;
        }
#endif
    }
};

/// @brief Memory prefetching utilities
struct MemoryOps {
    /// @brief Prefetch memory into cache (read)
    template<typename T>
    static void PrefetchRead(const T* ptr) {
        _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0);
    }
    
    /// @brief Prefetch for write
    template<typename T>
    static void PrefetchWrite(const T* ptr) {
#if defined(__AVX2__)
        _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0);
#endif
    }
    
    /// @brief Prefetch range of memory
    template<typename T>
    static void PrefetchRange(const T* start, size_t count) {
        constexpr size_t cacheLineSize = 64;
        constexpr size_t elementsPerLine = cacheLineSize / sizeof(T);
        
        for (size_t i = 0; i < count; i += elementsPerLine) {
            _mm_prefetch(reinterpret_cast<const char*>(&start[i]), _MM_HINT_T0);
        }
    }
};

/// @brief Compiler hints for branch prediction
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/// @brief Force inline for hot paths
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE inline
#endif

/// @brief Alignment macros for SIMD
#define ALIGN_16 alignas(16)
#define ALIGN_32 alignas(32)
#define ALIGN_64 alignas(64)

} // namespace SAGE::ECS
