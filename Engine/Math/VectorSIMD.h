#pragma once

#include <cmath>

// Enable SSE2 by default on x86/x64
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
    #define SAGE_USE_SSE2 1
    #include <emmintrin.h>  // SSE2
#endif

namespace SAGE {

/**
 * @brief SIMD-optimized vector operations
 * 
 * Uses SSE2 intrinsics for 4-wide parallel operations.
 * Falls back to scalar code if SIMD not available.
 */
namespace VectorSIMD {

#ifdef SAGE_USE_SSE2

    /**
     * @brief Add 4 Vector2s in parallel
     * 
     * Processes 4 2D vectors simultaneously using SSE2.
     * 
     * Layout in memory:
     *   v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, v4.x, v4.y
     */
    inline void Add4Vec2(const float* a, const float* b, float* out) {
        __m128 a1 = _mm_loadu_ps(a);      // Load a[0..3]
        __m128 a2 = _mm_loadu_ps(a + 4);  // Load a[4..7]
        __m128 b1 = _mm_loadu_ps(b);
        __m128 b2 = _mm_loadu_ps(b + 4);
        
        __m128 r1 = _mm_add_ps(a1, b1);
        __m128 r2 = _mm_add_ps(a2, b2);
        
        _mm_storeu_ps(out, r1);
        _mm_storeu_ps(out + 4, r2);
    }
    
    /**
     * @brief Multiply 4 Vector2s by scalar in parallel
     */
    inline void Scale4Vec2(const float* v, float scale, float* out) {
        __m128 s = _mm_set1_ps(scale);    // Broadcast scalar to all 4 lanes
        __m128 v1 = _mm_loadu_ps(v);
        __m128 v2 = _mm_loadu_ps(v + 4);
        
        __m128 r1 = _mm_mul_ps(v1, s);
        __m128 r2 = _mm_mul_ps(v2, s);
        
        _mm_storeu_ps(out, r1);
        _mm_storeu_ps(out + 4, r2);
    }
    
    /**
     * @brief Compute squared distance for 4 Vector2 pairs in parallel
     * 
     * Returns 4 float distances in 'out' array.
     */
    inline void DistanceSqr4Vec2(const float* a, const float* b, float* out) {
        // Load vectors
        __m128 a1 = _mm_loadu_ps(a);      // a1.x, a1.y, a2.x, a2.y
        __m128 a2 = _mm_loadu_ps(a + 4);  // a3.x, a3.y, a4.x, a4.y
        __m128 b1 = _mm_loadu_ps(b);
        __m128 b2 = _mm_loadu_ps(b + 4);
        
        // Compute deltas
        __m128 d1 = _mm_sub_ps(a1, b1);   // dx1, dy1, dx2, dy2
        __m128 d2 = _mm_sub_ps(a2, b2);   // dx3, dy3, dx4, dy4
        
        // Square deltas
        __m128 d1sq = _mm_mul_ps(d1, d1); // dx1², dy1², dx2², dy2²
        __m128 d2sq = _mm_mul_ps(d2, d2);
        
        // Horizontal add pairs (dx² + dy²)
        // Shuffle to get x components and y components in separate registers
        __m128 x1 = _mm_shuffle_ps(d1sq, d2sq, _MM_SHUFFLE(2, 0, 2, 0)); // dx1², dx2², dx3², dx4²
        __m128 y1 = _mm_shuffle_ps(d1sq, d2sq, _MM_SHUFFLE(3, 1, 3, 1)); // dy1², dy2², dy3², dy4²
        
        __m128 result = _mm_add_ps(x1, y1); // dx² + dy² for all 4
        _mm_storeu_ps(out, result);
    }
    
    /**
     * @brief Normalize 4 Vector2s in parallel
     */
    inline void Normalize4Vec2(const float* v, float* out) {
        __m128 v1 = _mm_loadu_ps(v);
        __m128 v2 = _mm_loadu_ps(v + 4);
        
        // Square
        __m128 v1sq = _mm_mul_ps(v1, v1);
        __m128 v2sq = _mm_mul_ps(v2, v2);
        
        // Horizontal add
        __m128 x = _mm_shuffle_ps(v1sq, v2sq, _MM_SHUFFLE(2, 0, 2, 0));
        __m128 y = _mm_shuffle_ps(v1sq, v2sq, _MM_SHUFFLE(3, 1, 3, 1));
        __m128 lengthSq = _mm_add_ps(x, y);
        
        // Inverse square root (fast approximation)
        __m128 invLen = _mm_rsqrt_ps(lengthSq);
        
        // Duplicate each component (x, x, y, y)
        __m128 invLen1 = _mm_shuffle_ps(invLen, invLen, _MM_SHUFFLE(1, 1, 0, 0));
        __m128 invLen2 = _mm_shuffle_ps(invLen, invLen, _MM_SHUFFLE(3, 3, 2, 2));
        
        // Normalize
        __m128 r1 = _mm_mul_ps(v1, invLen1);
        __m128 r2 = _mm_mul_ps(v2, invLen2);
        
        _mm_storeu_ps(out, r1);
        _mm_storeu_ps(out + 4, r2);
    }
    
    /**
     * @brief Dot product for 4 Vector2 pairs in parallel
     */
    inline void Dot4Vec2(const float* a, const float* b, float* out) {
        __m128 a1 = _mm_loadu_ps(a);
        __m128 a2 = _mm_loadu_ps(a + 4);
        __m128 b1 = _mm_loadu_ps(b);
        __m128 b2 = _mm_loadu_ps(b + 4);
        
        // Multiply
        __m128 m1 = _mm_mul_ps(a1, b1);
        __m128 m2 = _mm_mul_ps(a2, b2);
        
        // Horizontal add
        __m128 x = _mm_shuffle_ps(m1, m2, _MM_SHUFFLE(2, 0, 2, 0));
        __m128 y = _mm_shuffle_ps(m1, m2, _MM_SHUFFLE(3, 1, 3, 1));
        __m128 result = _mm_add_ps(x, y);
        
        _mm_storeu_ps(out, result);
    }

#else
    // Scalar fallbacks for non-SSE platforms
    
    inline void Add4Vec2(const float* a, const float* b, float* out) {
        for (int i = 0; i < 8; ++i) {
            out[i] = a[i] + b[i];
        }
    }
    
    inline void Scale4Vec2(const float* v, float scale, float* out) {
        for (int i = 0; i < 8; ++i) {
            out[i] = v[i] * scale;
        }
    }
    
    inline void DistanceSqr4Vec2(const float* a, const float* b, float* out) {
        for (int i = 0; i < 4; ++i) {
            float dx = a[i*2] - b[i*2];
            float dy = a[i*2+1] - b[i*2+1];
            out[i] = dx*dx + dy*dy;
        }
    }
    
    inline void Normalize4Vec2(const float* v, float* out) {
        for (int i = 0; i < 4; ++i) {
            float x = v[i*2];
            float y = v[i*2+1];
            float len = std::sqrt(x*x + y*y);
            if (len > 1e-6f) {
                out[i*2] = x / len;
                out[i*2+1] = y / len;
            } else {
                out[i*2] = 0.0f;
                out[i*2+1] = 0.0f;
            }
        }
    }
    
    inline void Dot4Vec2(const float* a, const float* b, float* out) {
        for (int i = 0; i < 4; ++i) {
            out[i] = a[i*2] * b[i*2] + a[i*2+1] * b[i*2+1];
        }
    }

#endif

} // namespace VectorSIMD

} // namespace SAGE
