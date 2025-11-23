/**
 * @file ParticleEmitterTests.cpp
 * @brief Unit tests for the ParticleEmitter system
 */

#include "catch2.hpp"
#include "SAGE/Graphics/ParticleEmitter.h"

using namespace SAGE;

TEST_CASE("ParticleEmitter - Basic Functionality", "[ParticleEmitter]") {
    SECTION("Construction and initialization") {
        ParticleEmitter emitter(100);
        
        REQUIRE_FALSE(emitter.IsActive());
        REQUIRE_FALSE(emitter.IsPaused());
        REQUIRE(emitter.GetActiveParticleCount() == 0);
    }

    SECTION("Start and stop") {
        ParticleEmitter emitter(100);
        
        emitter.Start();
        REQUIRE(emitter.IsActive());
        
        emitter.Stop();
        REQUIRE_FALSE(emitter.IsActive());
    }

    SECTION("Pause and resume") {
        ParticleEmitter emitter(100);
        
        emitter.Start();
        emitter.Pause();
        
        REQUIRE(emitter.IsActive());
        REQUIRE(emitter.IsPaused());
        
        emitter.Resume();
        REQUIRE_FALSE(emitter.IsPaused());
    }

    SECTION("Manual burst emission") {
        ParticleEmitter emitter(100);
        auto config = emitter.GetConfig();
        config.autoEmit = false;
        emitter.SetConfig(config);
        
        emitter.Start();
        emitter.Burst(10);
        
        REQUIRE(emitter.GetActiveParticleCount() == 10);
    }
}

TEST_CASE("ParticleEmitter - Configuration", "[ParticleEmitter]") {
    SECTION("Set custom configuration") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.emissionRate = 50.0f;
        config.lifetimeMin = 1.0f;
        config.lifetimeMax = 2.0f;
        config.startColor = Color{1.0f, 0.0f, 0.0f, 1.0f};
        
        emitter.SetConfig(config);
        
        const auto& retrievedConfig = emitter.GetConfig();
        REQUIRE(retrievedConfig.emissionRate == 50.0f);
        REQUIRE(retrievedConfig.lifetimeMin == 1.0f);
        REQUIRE(retrievedConfig.startColor.r == 1.0f);
    }

    SECTION("Zero emission rate gets corrected") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.emissionRate = 0.0f; // Invalid
        
        emitter.SetConfig(config);
        
        const auto& retrievedConfig = emitter.GetConfig();
        REQUIRE(retrievedConfig.emissionRate == 1.0f); // Should be corrected
    }
}

TEST_CASE("ParticleEmitter - Emission Shapes", "[ParticleEmitter]") {
    SECTION("Point emitter") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.shape = EmitterShape::Point;
        config.position = {100.0f, 200.0f};
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Burst(1);
        
        const auto& particles = emitter.GetParticles();
        // Find active particle
        for (const auto& p : particles) {
            if (p.active) {
                REQUIRE(p.position.x == 100.0f);
                REQUIRE(p.position.y == 200.0f);
                break;
            }
        }
    }

    SECTION("Circle emitter") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.shape = EmitterShape::Circle;
        config.position = {0.0f, 0.0f};
        config.radius = 50.0f;
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Burst(10);
        
        // All particles should be roughly 50 units from origin
        const auto& particles = emitter.GetParticles();
        for (const auto& p : particles) {
            if (p.active) {
                float dist = std::sqrt(p.position.x * p.position.x + 
                                      p.position.y * p.position.y);
                REQUIRE(dist >= 45.0f); // Allow some tolerance
                REQUIRE(dist <= 55.0f);
            }
        }
    }

    SECTION("Box emitter") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.shape = EmitterShape::Box;
        config.position = {0.0f, 0.0f};
        config.boxSize = {100.0f, 100.0f};
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Burst(50);
        
        // All particles should be within box bounds
        const auto& particles = emitter.GetParticles();
        for (const auto& p : particles) {
            if (p.active) {
                REQUIRE(p.position.x >= -50.0f);
                REQUIRE(p.position.x <= 50.0f);
                REQUIRE(p.position.y >= -50.0f);
                REQUIRE(p.position.y <= 50.0f);
            }
        }
    }

    SECTION("Cone emitter") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.shape = EmitterShape::Cone;
        config.position = {0.0f, 0.0f};
        config.direction = {0.0f, -1.0f}; // Upward
        config.coneAngle = 45.0f;
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Burst(10);
        
        REQUIRE(emitter.GetActiveParticleCount() == 10);
    }
}

TEST_CASE("ParticleEmitter - Update Behavior", "[ParticleEmitter]") {
    SECTION("Continuous emission") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.emissionRate = 10.0f; // 10 particles per second
        config.autoEmit = true;
        config.burstCount = 0; // Continuous mode
        
        emitter.SetConfig(config);
        emitter.Start();
        
        // Update for 0.5 seconds = should emit ~5 particles
        emitter.Update(0.5f);
        
        REQUIRE(emitter.GetActiveParticleCount() >= 4);
        REQUIRE(emitter.GetActiveParticleCount() <= 6);
    }

    SECTION("Burst mode") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.burstCount = 10;
        config.burstInterval = 0.5f;
        config.autoEmit = true;
        config.loop = true;
        
        emitter.SetConfig(config);
        emitter.Start();
        
        // Multiple updates to get particles
        for (int i = 0; i < 5; ++i) {
            emitter.Update(0.2f);
        }
        
        size_t count = emitter.GetActiveParticleCount();
        
        // Should have emitted at least one burst worth
        REQUIRE(count >= 1); // At least some particles
    }

    SECTION("Paused emitter doesn't emit") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.emissionRate = 100.0f;
        config.autoEmit = true;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Pause();
        
        emitter.Update(1.0f); // Would emit 100 particles if not paused
        
        REQUIRE(emitter.GetActiveParticleCount() == 0);
    }

    SECTION("Particle lifetime") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.lifetimeMin = 0.1f;
        config.lifetimeMax = 0.1f;
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        emitter.Burst(10);
        
        REQUIRE(emitter.GetActiveParticleCount() == 10);
        
        // Update past lifetime
        emitter.Update(0.2f);
        
        REQUIRE(emitter.GetActiveParticleCount() == 0);
    }

    SECTION("Non-looping emitter stops after duration") {
        ParticleEmitter emitter(100);
        
        ParticleEmitterConfig config;
        config.loop = false;
        config.duration = 0.5f;
        config.autoEmit = true;
        config.emissionRate = 10.0f;
        
        emitter.SetConfig(config);
        emitter.Start();
        
        REQUIRE(emitter.IsActive());
        
        emitter.Update(0.6f); // Past duration
        
        REQUIRE_FALSE(emitter.IsActive());
    }
}

TEST_CASE("ParticleEmitter - Preset Configurations", "[ParticleEmitter]") {
    SECTION("Fire emitter preset") {
        auto config = ParticleEmitter::CreateFireEmitter();
        
        REQUIRE(config.emissionRate > 0.0f);
        REQUIRE(config.acceleration.y < 0.0f); // Rises up
        REQUIRE(config.startColor.r >= 0.8f); // Orangeish
    }

    SECTION("Smoke emitter preset") {
        auto config = ParticleEmitter::CreateSmokeEmitter();
        
        REQUIRE(config.shape == EmitterShape::Circle);
        REQUIRE(config.sizeOverLifetime > 1.0f); // Grows
        REQUIRE(config.startColor.r <= 0.6f); // Grayish
    }

    SECTION("Explosion emitter preset") {
        auto config = ParticleEmitter::CreateExplosionEmitter();
        
        REQUIRE(config.burstCount > 0);
        REQUIRE_FALSE(config.loop);
        REQUIRE_FALSE(config.autoEmit); // Manual burst
    }

    SECTION("Rain emitter preset") {
        auto config = ParticleEmitter::CreateRainEmitter();
        
        REQUIRE(config.shape == EmitterShape::Box);
        REQUIRE(config.acceleration.y > 0.0f); // Falls down
        REQUIRE(config.boxSize.x > config.boxSize.y); // Wide area
    }

    SECTION("Snow emitter preset") {
        auto config = ParticleEmitter::CreateSnowEmitter();
        
        REQUIRE(config.shape == EmitterShape::Box);
        REQUIRE(config.startColor.r >= 0.9f); // White
        REQUIRE(config.velocityMax.y < 100.0f); // Slow fall
    }
}

TEST_CASE("ParticleEmitter - Maximum Capacity", "[ParticleEmitter]") {
    SECTION("Cannot emit more than max particles") {
        ParticleEmitter emitter(10); // Max 10 particles
        
        emitter.Start();
        emitter.Burst(20); // Try to emit 20
        
        REQUIRE(emitter.GetActiveParticleCount() <= 10);
    }

    SECTION("Reuses dead particles") {
        ParticleEmitter emitter(10);
        
        ParticleEmitterConfig config;
        config.lifetimeMin = 0.01f;
        config.lifetimeMax = 0.01f;
        config.autoEmit = false;
        
        emitter.SetConfig(config);
        emitter.Start();
        
        emitter.Burst(10);
        REQUIRE(emitter.GetActiveParticleCount() == 10);
        
        emitter.Update(0.02f); // Kill all particles
        REQUIRE(emitter.GetActiveParticleCount() == 0);
        
        emitter.Burst(10); // Should reuse slots
        REQUIRE(emitter.GetActiveParticleCount() == 10);
    }
}
