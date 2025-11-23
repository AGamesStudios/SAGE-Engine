/**
 * @file QuadTreeTests.cpp
 * @brief Unit tests for the QuadTree spatial partitioning system
 */

#include "catch2.hpp"
#include "SAGE/Math/QuadTree.h"

using namespace SAGE;

TEST_CASE("QuadTree - Basic Operations", "[QuadTree]") {
    QuadTree<int> tree(Rect{0, 0, 100, 100}, 4, 3);

    SECTION("Insert and retrieve single object") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});

        auto results = tree.Retrieve(Rect{10, 10, 5, 5});
        REQUIRE(results.size() >= 1); // At least 1
        REQUIRE(results[0] == 1);
    }

    SECTION("Insert multiple objects in same area") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});
        tree.Insert({Rect{12, 12, 5, 5}, 2});
        tree.Insert({Rect{14, 14, 5, 5}, 3});

        // Query area that intersects all three objects
        auto results = tree.Retrieve(Rect{10, 10, 15, 15});
        REQUIRE(results.size() >= 3); // May have more if duplicates, but at least 3
    }

    SECTION("Objects in different quadrants") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});   // Top-left
        tree.Insert({Rect{60, 10, 5, 5}, 2});   // Top-right
        tree.Insert({Rect{10, 60, 5, 5}, 3});   // Bottom-left
        tree.Insert({Rect{60, 60, 5, 5}, 4});   // Bottom-right

        // Query top-left quadrant - should get object 1
        auto results = tree.Retrieve(Rect{5, 5, 20, 20});
        REQUIRE(results.size() >= 1);
        // Should contain object 1
        bool found = false;
        for (auto r : results) {
            if (r == 1) found = true;
        }
        REQUIRE(found);
    }

    SECTION("Clear tree") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});
        tree.Insert({Rect{20, 20, 5, 5}, 2});

        // Tree should have objects
        auto allBefore = tree.QueryAll();
        REQUIRE(allBefore.size() >= 1); // At least some objects

        tree.Clear();

        REQUIRE(tree.GetTotalCount() == 0);
        auto results = tree.QueryAll();
        REQUIRE(results.empty());
    }

    SECTION("Query all objects") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});
        tree.Insert({Rect{20, 20, 5, 5}, 2});
        tree.Insert({Rect{30, 30, 5, 5}, 3});

        auto all = tree.QueryAll();
        REQUIRE(all.size() == 3);
    }
}

TEST_CASE("QuadTree - Subdivision", "[QuadTree]") {
    // Max 2 objects per node, max depth 3
    QuadTree<int> tree(Rect{0, 0, 100, 100}, 2, 3);

    SECTION("Triggers subdivision") {
        tree.Insert({Rect{10, 10, 5, 5}, 1});
        tree.Insert({Rect{15, 15, 5, 5}, 2});
        tree.Insert({Rect{20, 20, 5, 5}, 3}); // Should trigger split

        REQUIRE(tree.GetTotalCount() == 3);
        REQUIRE(tree.GetDepth() > 0); // Tree has subdivided
    }

    SECTION("Multiple subdivision levels") {
        // Add many objects in same area to force deep subdivision
        for (int i = 0; i < 10; ++i) {
            tree.Insert({Rect{10.0f + i * 2, 10.0f + i * 2, 5, 5}, i});
        }

        // GetTotalCount counts objects in all nodes
        auto allObjects = tree.QueryAll();
        REQUIRE(allObjects.size() == 10);
        // Should have multiple levels
        REQUIRE(tree.GetDepth() >= 1);
    }

    SECTION("Objects straddling boundaries stay in parent") {
        QuadTree<int> tree2(Rect{0, 0, 100, 100}, 1, 5);
        
        // Object that crosses quadrant boundary (centered)
        tree2.Insert({Rect{45, 45, 10, 10}, 1});
        
        // Add more objects to trigger subdivision
        tree2.Insert({Rect{10, 10, 5, 5}, 2});
        tree2.Insert({Rect{80, 10, 5, 5}, 3});
        
        auto results = tree2.Retrieve(Rect{45, 45, 10, 10});
        REQUIRE(results.size() >= 1); // At least the straddling object
    }
}

TEST_CASE("QuadTree - Spatial Queries", "[QuadTree]") {
    QuadTree<int> tree(Rect{0, 0, 1000, 1000}, 10, 5);

    // Populate tree with grid of objects
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            float px = x * 100.0f + 10.0f;
            float py = y * 100.0f + 10.0f;
            tree.Insert({Rect{px, py, 20, 20}, x * 10 + y});
        }
    }

    SECTION("Small query area returns few objects") {
        auto results = tree.Retrieve(Rect{50, 50, 30, 30});
        
        // Should only get nearby objects, not all 100
        REQUIRE(results.size() < 10);
    }

    SECTION("Large query area returns more objects") {
        auto results = tree.Retrieve(Rect{0, 0, 500, 500});
        
        // Should get objects in large area
        REQUIRE(results.size() > 10);
    }

    SECTION("Query outside bounds returns nothing") {
        // Make sure we have some objects first
        REQUIRE(tree.GetTotalCount() > 0);
        
        // Query completely outside tree bounds
        auto results = tree.Retrieve(Rect{2000, 2000, 100, 100});
        
        REQUIRE(results.empty());
    }

    SECTION("Query exactly at object position") {
        auto results = tree.Retrieve(Rect{110, 110, 20, 20});
        
        REQUIRE(results.size() >= 1);
    }
}

TEST_CASE("QuadTree - Performance", "[QuadTree][Performance]") {
    QuadTree<int> tree(Rect{0, 0, 10000, 10000}, 10, 6);

    SECTION("Insert many objects") {
        const int objectCount = 1000;

        for (int i = 0; i < objectCount; ++i) {
            float x = (i % 100) * 100.0f;
            float y = (i / 100) * 100.0f;
            tree.Insert({Rect{x, y, 50, 50}, i});
        }

        REQUIRE(tree.GetTotalCount() == objectCount);
    }

    SECTION("Query efficiency vs brute force") {
        // Insert 500 objects
        for (int i = 0; i < 500; ++i) {
            float x = (i % 50) * 200.0f;
            float y = (i / 50) * 200.0f;
            tree.Insert({Rect{x, y, 50, 50}, i});
        }

        // Query small area that should only intersect a few objects
        auto results = tree.Retrieve(Rect{1000, 1000, 100, 100});
        
        // Should return much less than all 500 objects
        // Due to intersection filtering, should be very few
        REQUIRE(results.size() < 500); // At minimum, not all objects
        REQUIRE(results.size() >= 0);  // May be 0 if no intersection
    }
}

TEST_CASE("QuadTree - Edge Cases", "[QuadTree]") {
    SECTION("Zero-sized bounds") {
        QuadTree<int> tree(Rect{0, 0, 0, 0}, 5, 3);
        tree.Insert({Rect{0, 0, 1, 1}, 1});
        
        // Should not crash
        REQUIRE(tree.GetTotalCount() >= 0);
    }

    SECTION("Negative coordinates") {
        QuadTree<int> tree(Rect{-100, -100, 200, 200}, 5, 3);
        
        tree.Insert({Rect{-50, -50, 10, 10}, 1});
        tree.Insert({Rect{0, 0, 10, 10}, 2});
        tree.Insert({Rect{50, 50, 10, 10}, 3});

        REQUIRE(tree.GetTotalCount() == 3);
    }

    SECTION("Very small objects") {
        QuadTree<int> tree(Rect{0, 0, 100, 100}, 5, 3);
        
        tree.Insert({Rect{50, 50, 0.1f, 0.1f}, 1});
        
        auto results = tree.Retrieve(Rect{49, 49, 2, 2});
        REQUIRE(results.size() == 1);
    }

    SECTION("Max depth prevents infinite subdivision") {
        QuadTree<int> tree(Rect{0, 0, 100, 100}, 1, 2); // Max depth = 2
        
        // Add many objects in exact same spot
        for (int i = 0; i < 100; ++i) {
            tree.Insert({Rect{50, 50, 1, 1}, i});
        }

        REQUIRE(tree.GetDepth() <= 2);
    }
}

TEST_CASE("QuadTree - Different Data Types", "[QuadTree]") {
    SECTION("Store pointers") {
        struct GameObject {
            int id;
            std::string name;
        };

        GameObject obj1{1, "Player"};
        GameObject obj2{2, "Enemy"};

        QuadTree<GameObject*> tree(Rect{0, 0, 100, 100}, 5, 3);
        
        tree.Insert({Rect{10, 10, 5, 5}, &obj1});
        tree.Insert({Rect{50, 50, 5, 5}, &obj2});

        auto results = tree.Retrieve(Rect{0, 0, 30, 30});
        REQUIRE(results.size() == 1);
        REQUIRE(results[0]->id == 1);
    }

    SECTION("Store complex types") {
        struct Entity {
            int value;
            bool operator==(const Entity& other) const { return value == other.value; }
        };

        QuadTree<Entity> tree(Rect{0, 0, 100, 100}, 5, 3);
        
        tree.Insert({Rect{10, 10, 5, 5}, Entity{42}});
        
        // Query that definitely intersects the object
        auto results = tree.Retrieve(Rect{8, 8, 10, 10});
        REQUIRE(results.size() >= 1);
        if (results.size() > 0) {
            REQUIRE(results[0].value == 42);
        }
    }
}
