#include "TestFramework.h"
#include "ECS/ECS.h"

#include <cstdint>

using namespace SAGE;
using namespace SAGE::ECS;

namespace {

struct SimpleComponent {
    int value = 0;
};

} // namespace

TEST(ECS_CreatesAndDestroysEntities) {
    Registry registry;

    Entity entity = registry.CreateEntity();
    ASSERT_NE(entity, NullEntity);
    CHECK(registry.ContainsEntity(entity));

    registry.DestroyEntity(entity);
    CHECK_FALSE(registry.ContainsEntity(entity));
}

TEST(ECS_AddsAndRetrievesComponent) {
    Registry registry;
    Entity entity = registry.CreateEntity();

    SimpleComponent component;
    component.value = 42;
    registry.AddComponent(entity, component);

    CHECK(registry.HasComponent<SimpleComponent>(entity));

    auto* fetched = registry.GetComponent<SimpleComponent>(entity);
    ASSERT_NOT_NULL(fetched);
    CHECK_EQ(fetched->value, 42);
}

TEST(ECS_RemovesComponent) {
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent(entity, SimpleComponent{1337});
    CHECK(registry.HasComponent<SimpleComponent>(entity));

    registry.RemoveComponent<SimpleComponent>(entity);
    CHECK_FALSE(registry.HasComponent<SimpleComponent>(entity));
    CHECK(registry.GetComponent<SimpleComponent>(entity) == nullptr);
}

TEST(ECS_ForEachVisitsAllComponents) {
    Registry registry;
    constexpr int kEntityCount = 5;

    for (int i = 0; i < kEntityCount; ++i) {
        Entity entity = registry.CreateEntity();
        registry.AddComponent(entity, SimpleComponent{i});
    }

    int visitCount = 0;
    registry.ForEach<SimpleComponent>([&](Entity entity, SimpleComponent& comp) {
        CHECK(registry.ContainsEntity(entity));
        comp.value += 1;
        ++visitCount;
    });

    CHECK_EQ(visitCount, kEntityCount);
}

TEST(ECS_EntityVersionChangesAfterReuse) {
    Registry registry;

    Entity first = registry.CreateEntity();
    const std::uint32_t firstId = GetEntityID(first);
    const std::uint32_t firstVersion = GetEntityVersion(first);

    registry.DestroyEntity(first);

    Entity second = registry.CreateEntity();
    if (GetEntityID(second) == firstId) {
        CHECK(GetEntityVersion(second) > firstVersion);
    }

    CHECK_FALSE(registry.ContainsEntity(first));
    CHECK(registry.ContainsEntity(second));
}

extern "C" void __sage_force_link_ECSCoreTests() {}
