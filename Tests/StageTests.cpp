#include "TestFramework.h"

#include <SAGE.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

using namespace sage2d;

namespace {
    std::vector<std::string>* gOrder = nullptr;

    void ScriptPre(Object&, float) {
        if (gOrder) {
            gOrder->push_back("ScriptPreObject");
        }
    }

    void ScriptPost(Object&, float) {
        if (gOrder) {
            gOrder->push_back("ScriptPostObject");
        }
    }
}

TEST_CASE(StageSpawnPopulatesCapabilities) {
    Vault vault;

    Role role;
    role.name = "Hero";
    Sprite sprite;
    sprite.image = vault.image("Assets/hero.png");
    sprite.animation = vault.animation("Assets/hero.anim");
    role.sprite = sprite;

    Physics physics;
    physics.velocity = Vec2(5.0f, 0.0f);
    role.physics = physics;

    Collider collider;
    collider.w = 1.0f;
    collider.h = 2.0f;
    role.collider = collider;

    Controls controls;
    controls.jump = 'J';
    role.controls = controls;

    Script script;
    script.update = nullptr;
    role.script = script;

    const ResId roleId = vault.registerRole("Hero", role);

    Stage stage("Gameplay", vault);

    const ObjectId id = stage.spawn("Player", roleId);
    REQUIRE(id != kInvalidObjectId);
    CHECK(stage.objectCount() == 1);

    auto physicsSlice = stage.physics();
    CHECK(physicsSlice.owners.size() == 1);
    CHECK(physicsSlice.owners[0] == id);

    auto spriteSlice = stage.sprites();
    CHECK(spriteSlice.owners.size() == 1);
    CHECK(spriteSlice.owners[0] == id);

    CHECK(stage.has(id, Category::Controls));
    CHECK(stage.has(id, Category::Collider));
    CHECK(stage.has(id, Category::Physics));
    CHECK(stage.has(id, Category::Sprite));

    const Vec2 initialPosition = stage.position(id);
    stage.update(1.0f / 60.0f);
    const Vec2 afterPosition = stage.position(id);
    CHECK(afterPosition.x > initialPosition.x);
}

TEST_CASE(StageRemovePurgesCapabilities) {
    Vault vault;

    Role role;
    role.name = "Crate";
    Physics physics;
    physics.velocity = Vec2(0.0f, -1.0f);
    role.physics = physics;
    const ResId roleId = vault.registerRole("Crate", role);

    Stage stage("Gameplay", vault);
    const ObjectId id = stage.spawn("Falling", roleId);
    REQUIRE(id != kInvalidObjectId);
    CHECK(stage.objectCount() == 1);

    stage.update(1.0f / 60.0f);
    CHECK(stage.remove(id));
    CHECK(stage.objectCount() == 0);
    CHECK(stage.physics().owners.empty());
    CHECK(!stage.contains(id));
}

TEST_CASE(StageManagerStackOperations) {
    Vault vault;

    StageManager manager(vault);
    manager.registerStage("Gameplay", [](Vault& v) {
        return std::make_unique<Stage>("Gameplay", v);
    });

    Skin skin;
    skin.name = "Neon";
    const ResId skinId = vault.registerSkin("Neon", skin);
    manager.setSkinOverride("Gameplay", skinId);

    Stage& stage = manager.push("Gameplay");
    CHECK(manager.stackSize() == 1);
    CHECK(stage.defaultSkin() == skinId);

    manager.update(0.016f);

    manager.pop();
    CHECK(manager.stackSize() == 0);
}

TEST_CASE(StageLifecycleOrder) {
    Vault vault;

    Role role;
    role.name = "Hero";
    Physics physics;
    physics.velocity = Vec2(0.0f, 0.0f);
    role.physics = physics;

    Script script;
    script.preUpdate = &ScriptPre;
    script.postUpdate = &ScriptPost;
    role.script = script;

    const ResId roleId = vault.registerRole("Hero", role);

    Stage stage("Gameplay", vault);

    std::vector<std::string> order;

    stage.addPhaseCallback(StagePhase::Input, [&order](Stage&, float) { order.push_back("Input"); });
    stage.addPhaseCallback(StagePhase::Timers, [&order](Stage&, float) { order.push_back("Timers"); });
    stage.addPhaseCallback(StagePhase::ScriptPre, [&order](Stage&, float) { order.push_back("ScriptPre"); });
    stage.addPhaseCallback(StagePhase::Physics, [&order](Stage&, float) { order.push_back("Physics"); });
    stage.addPhaseCallback(StagePhase::Collision, [&order](Stage&, float) { order.push_back("Collision"); });
    stage.addPhaseCallback(StagePhase::ScriptPost, [&order](Stage&, float) { order.push_back("ScriptPost"); });
    stage.addPhaseCallback(StagePhase::Culling, [&order](Stage&, float) { order.push_back("Culling"); });
    stage.addPhaseCallback(StagePhase::Render, [&order](Stage&, float) { order.push_back("Render"); });

    const ObjectId id = stage.spawn("Player", roleId);
    REQUIRE(id != kInvalidObjectId);

    gOrder = &order;
    stage.update(1.0f / 60.0f);
    gOrder = nullptr;

    const std::vector<std::string> expected = {
        "Input",
        "Timers",
        "ScriptPre",
        "ScriptPreObject",
        "Physics",
        "Collision",
        "ScriptPostObject",
        "ScriptPost",
        "Culling",
        "Render"
    };

    CHECK(order.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        CHECK(order[i] == expected[i]);
    }
}

TEST_CASE(StageEventBusSignals) {
    Vault vault;

    Role role;
    role.name = "Entity";
    const ResId roleId = vault.registerRole("Entity", role);

    Stage stage("Gameplay", vault);
    const ObjectId id = stage.spawn("Listener", roleId);
    REQUIRE(id != kInvalidObjectId);

    Object object = stage.makeObject(id);
    REQUIRE(object.valid());

    std::vector<EventType> received;
    std::vector<float> tickValues;
    std::vector<std::uint32_t> useTags;

    object.on(EventType::Start, [&received](Object& self, const Event& event) {
        (void)self;
        received.push_back(event.type);
    });

    object.on(EventType::Tick, [&received, &tickValues](Object&, const Event& event) {
        received.push_back(event.type);
        tickValues.push_back(event.payload.value);
    });

    object.on(EventType::Use, [&received, &useTags](Object&, const Event& event) {
        received.push_back(event.type);
        useTags.push_back(event.payload.data);
    });

    object.use(kInvalidObjectId, 99);

    const float deltaTime = 0.1f;
    stage.update(deltaTime);

    CHECK(std::count(received.begin(), received.end(), EventType::Start) == 1);
    CHECK(std::count(received.begin(), received.end(), EventType::Tick) == 1);
    CHECK(std::count(received.begin(), received.end(), EventType::Use) == 1);

    REQUIRE_FALSE(tickValues.empty());
    CHECK(tickValues.front() == Approx(deltaTime).margin(0.0001f));

    REQUIRE_FALSE(useTags.empty());
    CHECK(useTags.front() == 99u);
}

TEST_CASE(StageCollisionEvents) {
    Vault vault;

    Role role;
    role.name = "Collider";
    Collider collider;
    collider.w = 1.0f;
    collider.h = 1.0f;
    collider.layer = 0x1u;
    collider.mask = 0x1u;
    role.collider = collider;

    const ResId roleId = vault.registerRole("Collider", role);

    Stage stage("Gameplay", vault);
    const ObjectId aId = stage.spawn("A", roleId);
    const ObjectId bId = stage.spawn("B", roleId);
    REQUIRE(aId != kInvalidObjectId);
    REQUIRE(bId != kInvalidObjectId);

    Object objectA = stage.makeObject(aId);
    Object objectB = stage.makeObject(bId);

    struct LoggedEvent {
        EventType type;
        ObjectId self;
        ObjectId other;
    };

    std::vector<LoggedEvent> events;

    auto logEvent = [&events](Object& self, const Event& event) {
        events.push_back({ event.type, self.id(), event.payload.other });
    };

    objectA.on(EventType::Enter, logEvent);
    objectA.on(EventType::Hit, logEvent);
    objectA.on(EventType::Exit, logEvent);

    objectB.on(EventType::Enter, logEvent);
    objectB.on(EventType::Hit, logEvent);
    objectB.on(EventType::Exit, logEvent);

    stage.setPosition(aId, Vec2(0.0f, 0.0f));
    stage.setPosition(bId, Vec2(0.5f, 0.0f));
    stage.update(0.016f);

    stage.setPosition(bId, Vec2(5.0f, 0.0f));
    stage.update(0.016f);

    auto countFor = [&events](EventType type, ObjectId objectId) {
        return std::count_if(events.begin(), events.end(), [=](const LoggedEvent& e) {
            return e.type == type && e.self == objectId;
        });
    };

    CHECK(countFor(EventType::Enter, aId) >= 1);
    CHECK(countFor(EventType::Enter, bId) >= 1);
    CHECK(countFor(EventType::Hit, aId) >= 1);
    CHECK(countFor(EventType::Hit, bId) >= 1);
    CHECK(countFor(EventType::Exit, aId) >= 1);
    CHECK(countFor(EventType::Exit, bId) >= 1);
}

TEST_CASE(StageTimerEvents) {
    Vault vault;

    Role role;
    role.name = "Timer";
    const ResId roleId = vault.registerRole("Timer", role);

    Stage stage("Gameplay", vault);
    const ObjectId id = stage.spawn("TimerObject", roleId);
    REQUIRE(id != kInvalidObjectId);

    Object object = stage.makeObject(id);
    REQUIRE(object.valid());

    std::vector<std::uint32_t> tags;
    std::vector<float> durations;

    object.on(EventType::Timer, [&tags, &durations](Object&, const Event& event) {
        tags.push_back(event.payload.data);
        durations.push_back(event.payload.value);
    });

    const float timerDuration = 0.05f;
    const std::uint32_t timerTag = 123u;
    const std::uint32_t timerId = object.addTimer(timerDuration, false, timerTag);
    REQUIRE(timerId != 0u);

    stage.update(0.02f);
    stage.update(0.02f);
    stage.update(0.02f);

    REQUIRE(tags.size() == 1);
    CHECK(tags.front() == timerTag);
    REQUIRE_FALSE(durations.empty());
    CHECK(durations.front() == Approx(timerDuration).margin(0.0001f));
    CHECK_FALSE(stage.cancelTimer(timerId));
}
