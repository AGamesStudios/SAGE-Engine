#include "Math/Random.h"

#include <algorithm>
#include <cmath>

namespace SAGE::Math {

Random::Random() : Random(static_cast<uint64_t>(std::random_device{}())) {}

Random::Random(uint64_t seed) {
    SetSeed(seed);
}

void Random::SetSeed(uint64_t seed) {
    m_Seed = seed;
    m_Engine.seed(seed);
}

float Random::NextFloat() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(m_Engine);
}

float Random::NextRange(float min, float max) {
    if (min == max) {
        return min;
    }
    const float low = std::min(min, max);
    const float high = std::max(min, max);
    std::uniform_real_distribution<float> dist(low, high);
    return dist(m_Engine);
}

int Random::NextInt(int min, int max) {
    if (min == max) {
        return min;
    }
    const int low = std::min(min, max);
    const int high = std::max(min, max);
    std::uniform_int_distribution<int> dist(low, high);
    return dist(m_Engine);
}

bool Random::NextBool(float probability) {
    const float p = std::clamp(probability, 0.0f, 1.0f);
    std::bernoulli_distribution dist(p);
    return dist(m_Engine);
}

Vector2 Random::NextUnitVector2() {
    const float angle = NextRange(0.0f, Constants::TWO_PI);
    return { std::cos(angle), std::sin(angle) };
}

Vector3 Random::NextUnitVector3() {
    // Pick random Z then random angle for XY circle (Marsaglia method)
    const float z = NextRange(-1.0f, 1.0f);
    const float angle = NextRange(0.0f, Constants::TWO_PI);
    const float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    return { r * std::cos(angle), r * std::sin(angle), z };
}

Random& Random::Global() {
    static Random s_global(static_cast<uint64_t>(std::random_device{}()));
    return s_global;
}

} // namespace SAGE::Math
