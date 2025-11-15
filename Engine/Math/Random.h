#pragma once

#include <cstdint>
#include <random>

#include "Math/Constants.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

namespace SAGE::Math {

/// @brief Deterministic random number generator utility based on std::mt19937_64
class Random {
public:
    Random();
    explicit Random(uint64_t seed);

    void SetSeed(uint64_t seed);
    [[nodiscard]] uint64_t GetSeed() const { return m_Seed; }

    /// @brief Returns a float in [0, 1)
    float NextFloat();

    /// @brief Returns a float in [min, max) regardless of order of arguments
    float NextRange(float min, float max);

    /// @brief Returns an integer in [min, max] regardless of order of arguments
    int NextInt(int min, int max);

    /// @brief Returns true with the given probability (0..1)
    bool NextBool(float probability = 0.5f);

    /// @brief Returns random unit vector in 2D space
    Vector2 NextUnitVector2();

    /// @brief Returns random unit vector in 3D space
    Vector3 NextUnitVector3();

    /// @brief Global singleton RNG for convenience
    static Random& Global();

private:
    uint64_t m_Seed = 0;
    std::mt19937_64 m_Engine;
};

} // namespace SAGE::Math
