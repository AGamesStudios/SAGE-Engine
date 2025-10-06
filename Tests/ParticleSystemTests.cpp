#include "TestFramework.h"

#include <SAGE.h>

#include <cmath>

using namespace SAGE;

namespace {
    ParticleEmitterConfig MakeBasicConfig() {
        ParticleEmitterConfig config;
        config.maxParticles = 32;
        config.emissionRate = 0.0f;
        config.minLifetime = 1.0f;
        config.maxLifetime = 1.0f;
        config.velocityMin = Vector2(-1.0f, -1.0f);
        config.velocityMax = Vector2(1.0f, 1.0f);
        config.startSize = 4.0f;
        config.endSize = 1.0f;
        config.startColor = Color::White();
        config.endColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
        config.spawnRadius = 0.0f;
        config.worldSpace = true;
        config.randomSeed = 1337u;
        return config;
    }
}

TEST_CASE(ParticleEmitter_BurstRespectsMaxParticles) {
    auto config = MakeBasicConfig();
    config.maxParticles = 5;
    ParticleEmitter emitter(config, Vector2(0.0f, 0.0f));

    emitter.Burst(20);
    CHECK(emitter.GetAliveCount() == 5);

    for (const Particle& particle : emitter.GetParticles()) {
        CHECK(std::abs(particle.lifetime - config.maxLifetime) < 0.0001f);
        CHECK(std::abs(particle.remainingLifetime - config.maxLifetime) < 0.0001f);
        CHECK(std::abs(particle.size - config.startSize) < 0.0001f);
        CHECK(std::abs(particle.color.a - config.startColor.a) < 0.0001f);
    }
}

TEST_CASE(ParticleEmitter_UpdateRemovesExpiredParticles) {
    auto config = MakeBasicConfig();
    config.maxParticles = 4;
    ParticleEmitter emitter(config, Vector2(0.0f, 0.0f));
    emitter.Burst(4);

    emitter.Update(0.25f);
    CHECK(emitter.GetAliveCount() == 4);

    emitter.Update(0.85f);
    CHECK(emitter.GetAliveCount() == 0);
}

TEST_CASE(ParticleEmitter_EmissionRateSpawnsOverTime) {
    auto config = MakeBasicConfig();
    config.emissionRate = 6.0f;
    config.maxParticles = 10;

    ParticleEmitter emitter(config, Vector2(0.0f, 0.0f));

    emitter.Update(0.1f);
    CHECK(emitter.GetAliveCount() == 0);

    emitter.Update(0.1f);
    CHECK(emitter.GetAliveCount() == 1);

    emitter.Update(0.5f);
    CHECK(emitter.GetAliveCount() == 4);
}

TEST_CASE(ParticleEmitter_ColorAndSizeInterpolate) {
    auto config = MakeBasicConfig();
    config.minLifetime = 2.0f;
    config.maxLifetime = 2.0f;
    config.startSize = 8.0f;
    config.endSize = 2.0f;
    config.startColor = Color(1.0f, 0.5f, 0.25f, 1.0f);
    config.endColor = Color(0.5f, 0.25f, 1.0f, 0.0f);

    ParticleEmitter emitter(config, Vector2(3.0f, 4.0f));
    emitter.Burst(1);
    REQUIRE(emitter.GetAliveCount() == 1);

    emitter.Update(1.0f);
    REQUIRE(emitter.GetAliveCount() == 1);

    const Particle& particle = emitter.GetParticles().front();
    CHECK(particle.size > 4.9f && particle.size < 5.1f);
    CHECK(particle.color.r > 0.74f && particle.color.r < 0.76f);
    CHECK(particle.color.g > 0.37f && particle.color.g < 0.39f);
    CHECK(particle.color.b > 0.61f && particle.color.b < 0.63f);
    CHECK(particle.color.a > 0.48f && particle.color.a < 0.52f);
}

TEST_CASE(ParticleEmitter_WorldSpaceVersusLocalSpace) {
    auto worldConfig = MakeBasicConfig();
    worldConfig.spawnRadius = 0.0f;
    worldConfig.worldSpace = true;
    ParticleEmitter worldEmitter(worldConfig, Vector2(10.0f, -2.0f));
    worldEmitter.Burst(1);
    REQUIRE(worldEmitter.GetAliveCount() == 1);
    const Particle& worldParticle = worldEmitter.GetParticles().front();
    CHECK(std::abs(worldParticle.position.x - 10.0f) < 0.0001f);
    CHECK(std::abs(worldParticle.position.y + 2.0f) < 0.0001f);

    auto localConfig = MakeBasicConfig();
    localConfig.spawnRadius = 0.0f;
    localConfig.worldSpace = false;
    ParticleEmitter localEmitter(localConfig, Vector2(5.0f, 5.0f));
    localEmitter.Burst(1);
    REQUIRE(localEmitter.GetAliveCount() == 1);
    const Particle& localParticle = localEmitter.GetParticles().front();
    CHECK(std::abs(localParticle.position.x) < 0.0001f);
    CHECK(std::abs(localParticle.position.y) < 0.0001f);
}
