#include "catch2.hpp"
#include <SAGE/Core/ECS.h>

using namespace SAGE::ECS;
using Catch::Approx;

struct Transform { float x = 0.0f; float y = 0.0f; };
struct Velocity { float vx = 0.0f; float vy = 0.0f; };
struct TagA {};
struct TagB {};

TEST_CASE("Registry create/destroy entities", "[ecs][registry]") {
    Registry reg;
    auto e1 = reg.CreateEntity();
    auto e2 = reg.CreateEntity();

    REQUIRE(reg.IsAlive(e1));
    REQUIRE(reg.IsAlive(e2));
    REQUIRE(reg.AliveCount() == 2);

    reg.DestroyEntity(e1);
    REQUIRE_FALSE(reg.IsAlive(e1));
    REQUIRE(reg.IsAlive(e2));
    REQUIRE(reg.AliveCount() == 1);

    auto e3 = reg.CreateEntity();
    REQUIRE(reg.IsAlive(e3));
    REQUIRE(reg.AliveCount() == 2);
}

TEST_CASE("Add/Get/Remove components", "[ecs][components]") {
    Registry reg;
    auto e = reg.CreateEntity();

    auto& t = reg.Add<Transform>(e);
    t.x = 10.0f;
    t.y = -5.0f;

    REQUIRE(reg.Has<Transform>(e));
    REQUIRE(reg.Get<Transform>(e)->x == Approx(10.0f));
    REQUIRE(reg.Get<Transform>(e)->y == Approx(-5.0f));

    reg.Remove<Transform>(e);
    REQUIRE_FALSE(reg.Has<Transform>(e));
    REQUIRE(reg.Get<Transform>(e) == nullptr);
}

TEST_CASE("ForEach iterates only matching signature", "[ecs][foreach]") {
    Registry reg;
    auto e1 = reg.CreateEntity();
    auto e2 = reg.CreateEntity();
    auto e3 = reg.CreateEntity();

    reg.Add<Transform>(e1);
    reg.Add<Transform>(e2);
    reg.Add<Transform>(e3);

    reg.Add<Velocity>(e1);
    reg.Add<Velocity>(e2);
    // e3 has only Transform

    int processed = 0;
    reg.ForEach<Transform, Velocity>([&](Entity, Transform& t, Velocity& v) {
        t.x += 1.0f;
        v.vx += 2.0f;
        processed++;
    });

    REQUIRE(processed == 2);
    REQUIRE(reg.Get<Transform>(e1)->x == Approx(1.0f));
    REQUIRE(reg.Get<Velocity>(e1)->vx == Approx(2.0f));
    REQUIRE(reg.Get<Transform>(e3)->x == Approx(0.0f)); // untouched
}

TEST_CASE("ForEach uses smallest pool heuristic", "[ecs][foreach]") {
    Registry reg;
    constexpr int count = 50;

    // Most entities have only TagA
    for (int i = 0; i < count; ++i) {
        auto e = reg.CreateEntity();
        reg.Add<TagA>(e);
        if (i % 10 == 0) { // few have TagB
            reg.Add<TagB>(e);
        }
    }

    int processed = 0;
    reg.ForEach<TagA, TagB>([&](Entity, TagA&, TagB&) {
        processed++;
    });

    REQUIRE(processed == count / 10 + (count % 10 == 0 ? 0 : 1));
}

TEST_CASE("SystemScheduler executes systems in order", "[ecs][scheduler]") {
    struct Counter {
        int first = 0;
        int second = 0;
    } counter;

    class FirstSystem : public ISystem {
    public:
        explicit FirstSystem(Counter& c) : ref(c) {}
        void Tick(Registry&, float) override { ref.first++; }
    private:
        Counter& ref;
    };

    class SecondSystem : public ISystem {
    public:
        explicit SecondSystem(Counter& c) : ref(c) {}
        void Tick(Registry&, float) override { ref.second = ref.first; }
    private:
        Counter& ref;
    };

    Registry reg;
    SystemScheduler sched;
    sched.AddSystem<FirstSystem>(counter);
    sched.AddSystem<SecondSystem>(counter);

    sched.UpdateAll(reg, 0.016f);
    REQUIRE(counter.first == 1);
    REQUIRE(counter.second == 1);
}
