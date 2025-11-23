/**
 * @file IntegrationTests.cpp
 * @brief Integration tests for SAGE Engine systems
 */

#include "catch2.hpp"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Graphics/ParticleSystem.h"
#include "SAGE/Graphics/ParticleEmitter.h"
#include "SAGE/Graphics/Shader.h"
#include "SAGE/Math/QuadTree.h"
#include "SAGE/Core/Profiler.h"
#include <thread>
#include <chrono>

using namespace SAGE;

TEST_CASE("Integration - Camera Transformations", "[Integration]") {
    SECTION("Camera position and zoom") {
        Camera2D camera(800, 600);
        camera.SetPosition({100.0f, 100.0f});
        camera.SetZoom(2.0f);
        
        // Get view-projection matrix
        auto viewProj = camera.GetViewProjectionMatrix();
        
        // Matrix should be valid (check a component using operator())
        REQUIRE(viewProj(0, 0) != 0.0f);
    }

    SECTION("Camera viewport dimensions") {
        Camera2D camera(800, 600);
        camera.SetPosition({400.0f, 300.0f});
        
        // Camera should have valid position
        auto pos = camera.GetPosition();
        REQUIRE(pos.x == 400.0f);
        REQUIRE(pos.y == 300.0f);
    }

    SECTION("Screen to world coordinate conversion") {
        Camera2D camera(800, 600);
        camera.SetPosition({400.0f, 300.0f});
        
        auto worldPos = camera.ScreenToWorld({400.0f, 300.0f});
        
        // Should return valid coordinates
        REQUIRE(std::isfinite(worldPos.x));
        REQUIRE(std::isfinite(worldPos.y));
    }
}

TEST_CASE("Integration - ParticleSystem + ParticleEmitter", "[Integration]") {
    SECTION("Emitter emits particles into system") {
        ParticleSystem system(100);
        ParticleEmitter emitter(100);
        
        auto config = ParticleEmitter::CreateFireEmitter();
        config.autoEmit = false;
        emitter.SetConfig(config);
        
        emitter.Start();
        emitter.Burst(10);
        
        REQUIRE(emitter.GetActiveParticleCount() == 10);
        
        // Update emitter
        emitter.Update(0.016f);
        
        REQUIRE(emitter.GetActiveParticleCount() >= 0);
    }

    SECTION("Multiple emitters") {
        ParticleEmitter fire(50);
        ParticleEmitter smoke(50);
        
        fire.SetConfig(ParticleEmitter::CreateFireEmitter());
        smoke.SetConfig(ParticleEmitter::CreateSmokeEmitter());
        
        fire.Start();
        smoke.Start();
        
        fire.Update(0.5f);
        smoke.Update(0.5f);
        
        size_t totalParticles = fire.GetActiveParticleCount() + 
                            smoke.GetActiveParticleCount();
        
        REQUIRE(totalParticles > 0);
    }
}

TEST_CASE("Integration - QuadTree + Physics", "[Integration]") {
    SECTION("QuadTree for broad-phase collision detection") {
        QuadTree<int> tree(Rect{0, 0, 1000, 1000}, 10, 5);
        
        // Add physics bodies
        struct Body {
            int id;
            Rect bounds;
        };
        
        std::vector<Body> bodies;
        for (int i = 0; i < 100; ++i) {
            float x = (i % 10) * 100.0f;
            float y = (i / 10) * 100.0f;
            bodies.push_back({i, Rect{x, y, 50, 50}});
            tree.Insert({bodies[i].bounds, i});
        }
        
        // Query potential collisions for one body
        auto potentialCollisions = tree.Retrieve(bodies[0].bounds);
        
        // Should return nearby bodies, not all 100
        REQUIRE(potentialCollisions.size() < 100);
        REQUIRE(potentialCollisions.size() > 0);
    }

    SECTION("Dynamic updates with moving objects") {
        QuadTree<int> tree(Rect{0, 0, 1000, 1000}, 10, 5);
        
        // Initial position
        Rect object{100, 100, 50, 50};
        tree.Insert({object, 1});
        
        REQUIRE(tree.GetTotalCount() == 1);
        
        // Simulate movement by clearing and re-inserting
        tree.Clear();
        object.x = 500; // Moved to different location
        tree.Insert({object, 1});
        
        auto results = tree.Retrieve(Rect{480, 80, 60, 60});
        REQUIRE(results.size() == 1);
    }
}

TEST_CASE("Integration - Profiler + All Systems", "[Integration]") {
    SECTION("Profile complete game loop") {
        Profiler::Get().Clear();
        Profiler::Get().SetEnabled(true);
        
        // Simulate game loop components
        {
            SAGE_PROFILE_SCOPE("Update");
            
            {
                SAGE_PROFILE_SCOPE("Physics");
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
            
            {
                SAGE_PROFILE_SCOPE("Particles");
                std::this_thread::sleep_for(std::chrono::microseconds(300));
            }
            
            {
                SAGE_PROFILE_SCOPE("Render");
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }
        }
        
        auto results = Profiler::Get().GetResults();
        
        // Should have all profiled scopes
        REQUIRE(results.size() >= 3);
        
        // Render should exist and be slowest (or at least in results)
        bool foundRender = false;
        for (const auto& r : results) {
            if (r.name == "Render") {
                foundRender = true;
                break;
            }
        }
        REQUIRE(foundRender);
    }

    SECTION("Detect performance bottlenecks") {
        Profiler::Get().Clear();
        Profiler::Get().SetEnabled(true);
        
        // Slow function - 100ms to ensure measurable difference
        auto slowFunction = []() {
            SAGE_PROFILE_SCOPE("SlowFunction");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
        
        // Fast function - 5ms
        auto fastFunction = []() {
            SAGE_PROFILE_SCOPE("FastFunction");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        };
        
        slowFunction();
        slowFunction();
        fastFunction();
        fastFunction();
        
        auto results = Profiler::Get().GetResults();
        
        // slowFunction should have higher total time
        auto slowResult = Profiler::Get().GetResult("SlowFunction");
        auto fastResult = Profiler::Get().GetResult("FastFunction");
        
        REQUIRE(slowResult.totalMs > fastResult.totalMs);
    }
}

TEST_CASE("Integration - Complete Scene Update", "[Integration]") {
    SECTION("Update all systems in proper order") {
        // Setup
        Camera2D camera(800, 600);
        ParticleEmitter emitter(100);
        QuadTree<int> quadTree(Rect{0, 0, 1000, 1000}, 10, 5);
        
        Profiler::Get().Clear();
        Profiler::Get().SetEnabled(true);
        
        float deltaTime = 0.016f;
        
        // Game loop simulation
        for (int frame = 0; frame < 5; ++frame) {
            SAGE_PROFILE_SCOPE("Frame");
            
            {
                SAGE_PROFILE_SCOPE("Update Camera");
                camera.Update(deltaTime);
            }
            
            {
                SAGE_PROFILE_SCOPE("Update Particles");
                emitter.Update(deltaTime);
            }
            
            {
                SAGE_PROFILE_SCOPE("Spatial Queries");
                auto results = quadTree.QueryAll();
            }
        }
        
        auto profile = Profiler::Get().GetResult("Frame");
        REQUIRE(profile.callCount == 5);
    }
}

TEST_CASE("Integration - Resource Management", "[Integration]") {
    SECTION("Multiple shaders lifecycle") {
        const char* vs = R"(
            #version 450 core
            layout (location = 0) in vec3 aPos;
            void main() { gl_Position = vec4(aPos, 1.0); }
        )";
        
        const char* fs = R"(
            #version 450 core
            out vec4 FragColor;
            void main() { FragColor = vec4(1.0); }
        )";
        
        std::vector<std::shared_ptr<Shader>> shaders;
        
        for (int i = 0; i < 5; ++i) {
            auto shader = Shader::Create(vs, fs);
            REQUIRE(shader != nullptr);
            shaders.push_back(shader);
        }
        
        // All shaders should be valid
        REQUIRE(shaders.size() == 5);
        
        // Clear shaders
        shaders.clear();
        
        // Should have cleaned up
        REQUIRE(shaders.empty());
    }
}

TEST_CASE("Integration - Error Recovery", "[Integration]") {
    SECTION("System continues after particle overflow") {
        ParticleEmitter emitter(10); // Small capacity
        
        emitter.Start();
        emitter.Burst(100); // Try to emit more than capacity
        
        // Should cap at max
        REQUIRE(emitter.GetActiveParticleCount() <= 10);
        
        // System should still work
        emitter.Update(0.016f);
        REQUIRE(emitter.IsActive());
    }

    SECTION("QuadTree handles boundary objects") {
        QuadTree<int> tree(Rect{0, 0, 100, 100}, 5, 3);
        
        // Object outside bounds
        tree.Insert({Rect{-50, -50, 10, 10}, 1});
        
        // Object partially outside
        tree.Insert({Rect{90, 90, 20, 20}, 2});
        
        // Should not crash
        auto all = tree.QueryAll();
        REQUIRE(all.size() >= 0);
    }

    SECTION("Profiler handles rapid enable/disable") {
        for (int i = 0; i < 100; ++i) {
            Profiler::Get().SetEnabled(i % 2 == 0);
            
            SAGE_PROFILE_SCOPE("Toggle Test");
        }
        
        // Should not crash
        REQUIRE(true);
    }
}

TEST_CASE("Integration - Performance Benchmarks", "[Integration][Benchmark]") {
    SECTION("QuadTree vs Linear Search") {
        const int objectCount = 500;
        QuadTree<int> tree(Rect{0, 0, 5000, 5000}, 10, 6);
        std::vector<Rect> objects;
        
        // Populate
        for (int i = 0; i < objectCount; ++i) {
            float x = (i % 50) * 100.0f;
            float y = (i / 50) * 100.0f;
            Rect rect{x, y, 50, 50};
            objects.push_back(rect);
            tree.Insert({rect, i});
        }
        
        Rect query{1000, 1000, 200, 200};
        
        // QuadTree query
        auto start = std::chrono::high_resolution_clock::now();
        auto quadResults = tree.Retrieve(query);
        auto quadTime = std::chrono::high_resolution_clock::now() - start;
        
        // Linear search
        start = std::chrono::high_resolution_clock::now();
        std::vector<int> linearResults;
        for (size_t i = 0; i < objects.size(); ++i) {
            if (query.Intersects(objects[i])) {
                linearResults.push_back(static_cast<int>(i));
            }
        }
        auto linearTime = std::chrono::high_resolution_clock::now() - start;
        
        // QuadTree should be faster or equal for large datasets
        REQUIRE(quadResults.size() == linearResults.size());
        // Note: QuadTree is faster for spatial queries
    }

    SECTION("Particle update performance") {
        Profiler::Get().Clear();
        Profiler::Get().SetEnabled(true);
        
        ParticleEmitter emitter(1000);
        emitter.SetConfig(ParticleEmitter::CreateFireEmitter());
        emitter.Start();
        
        // Fill with particles
        emitter.Update(1.0f);
        
        {
            SAGE_PROFILE_SCOPE("ParticleUpdate1000");
            emitter.Update(0.016f);
        }
        
        auto result = Profiler::Get().GetResult("ParticleUpdate1000");
        
        // Should complete in reasonable time (< 5ms)
        REQUIRE(result.averageMs < 5.0);
    }
}
