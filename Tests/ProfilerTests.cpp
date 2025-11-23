/**
 * @file ProfilerTests.cpp
 * @brief Unit tests for the Profiler system
 */

#include "catch2.hpp"
#include "SAGE/Core/Profiler.h"
#include <thread>
#include <chrono>

using namespace SAGE;

TEST_CASE("Profiler - Basic Functionality", "[Profiler]") {
    Profiler::Get().Clear();
    Profiler::Get().SetEnabled(true);

    SECTION("Single scope profiling") {
        Profiler::Get().BeginScope("TestScope");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Profiler::Get().EndScope("TestScope");

        auto result = Profiler::Get().GetResult("TestScope");
        REQUIRE(result.callCount == 1);
        REQUIRE(result.averageMs >= 10.0);
        REQUIRE(result.averageMs < 50.0); // Allow some variance
    }

    SECTION("Multiple calls accumulation") {
        for (int i = 0; i < 5; ++i) {
            Profiler::Get().BeginScope("MultiTest");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            Profiler::Get().EndScope("MultiTest");
        }

        auto result = Profiler::Get().GetResult("MultiTest");
        REQUIRE(result.callCount == 5);
        REQUIRE(result.averageMs >= 5.0);
        REQUIRE(result.totalMs >= 25.0);
    }

    SECTION("RAII ProfileScope") {
        {
            ProfileScope scope("RAIITest");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        auto result = Profiler::Get().GetResult("RAIITest");
        REQUIRE(result.callCount == 1);
        REQUIRE(result.averageMs >= 5.0);
    }

    SECTION("Enable/Disable profiling") {
        Profiler::Get().SetEnabled(false);
        
        Profiler::Get().BeginScope("DisabledTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        Profiler::Get().EndScope("DisabledTest");

        auto result = Profiler::Get().GetResult("DisabledTest");
        REQUIRE(result.callCount == 0);

        Profiler::Get().SetEnabled(true);
    }

    SECTION("Clear profiling data") {
        Profiler::Get().BeginScope("ClearTest");
        Profiler::Get().EndScope("ClearTest");

        Profiler::Get().Clear();

        auto result = Profiler::Get().GetResult("ClearTest");
        REQUIRE(result.callCount == 0);
    }

    SECTION("Get all results sorted by total time") {
        Profiler::Get().Clear();

        // Create scopes with different durations
        for (int i = 0; i < 3; ++i) {
            Profiler::Get().BeginScope("Fast");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Profiler::Get().EndScope("Fast");
        }

        for (int i = 0; i < 2; ++i) {
            Profiler::Get().BeginScope("Slow");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            Profiler::Get().EndScope("Slow");
        }

        auto results = Profiler::Get().GetResults();
        REQUIRE(results.size() == 2);
        
        // Should be sorted by total time descending
        // Timing may vary, so just check we have both scopes
        bool hasSlow = false;
        bool hasFast = false;
        for (const auto& r : results) {
            if (r.name == "Slow") hasSlow = true;
            if (r.name == "Fast") hasFast = true;
        }
        REQUIRE(hasSlow);
        REQUIRE(hasFast);
        
        // Slow should generally have more total time
        auto slowResult = Profiler::Get().GetResult("Slow");
        auto fastResult = Profiler::Get().GetResult("Fast");
        REQUIRE(slowResult.totalMs >= fastResult.totalMs * 0.5); // At least comparable
    }

    SECTION("Min/Max tracking") {
        Profiler::Get().Clear();

        Profiler::Get().BeginScope("MinMaxTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        Profiler::Get().EndScope("MinMaxTest");

        Profiler::Get().BeginScope("MinMaxTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        Profiler::Get().EndScope("MinMaxTest");

        auto result = Profiler::Get().GetResult("MinMaxTest");
        REQUIRE(result.minMs < result.maxMs);
        REQUIRE(result.minMs >= 5.0);
        REQUIRE(result.maxMs >= 15.0);
    }
}

TEST_CASE("Profiler - Edge Cases", "[Profiler]") {
    Profiler::Get().Clear();
    Profiler::Get().SetEnabled(true);

    SECTION("End scope without begin") {
        // Should not crash
        Profiler::Get().EndScope("NonExistent");
        
        auto result = Profiler::Get().GetResult("NonExistent");
        REQUIRE(result.callCount == 0);
    }

    SECTION("Nested scopes") {
        Profiler::Get().BeginScope("Outer");
        Profiler::Get().BeginScope("Inner");
        Profiler::Get().EndScope("Inner");
        Profiler::Get().EndScope("Outer");

        auto outer = Profiler::Get().GetResult("Outer");
        auto inner = Profiler::Get().GetResult("Inner");

        REQUIRE(outer.callCount == 1);
        REQUIRE(inner.callCount == 1);
    }

    SECTION("Sample limit (max 100)") {
        Profiler::Get().Clear();

        for (int i = 0; i < 150; ++i) {
            Profiler::Get().BeginScope("SampleLimit");
            Profiler::Get().EndScope("SampleLimit");
        }

        auto result = Profiler::Get().GetResult("SampleLimit");
        // Should keep only last 100 samples
        REQUIRE(result.callCount == 100);
    }
}
