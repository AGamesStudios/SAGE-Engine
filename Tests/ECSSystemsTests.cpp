#include "catch2.hpp"
#include <SAGE/Core/ECS.h>
#include <SAGE/Core/ECSComponents.h>
#include <SAGE/Core/ECSSystems.h>
#include <SAGE/Graphics/Camera2D.h>
#include "OpenGLStub.h"

using namespace SAGE;
using namespace SAGE::ECS;
using Catch::Approx;

TEST_CASE("StatsSystem regenerates and clamps stats", "[ecs][systems]") {
    Registry reg;
    auto e = reg.CreateEntity();
    auto& stats = reg.Add<StatsComponent>(e);
    stats.health = 50;
    stats.maxHealth = 100;
    stats.energy = 90;
    stats.maxEnergy = 100;

    StatsSystem system;
    system.regenHealthPerSec = 25.0f;
    system.regenEnergyPerSec = 20.0f;

    system.Tick(reg, 2.0f); // +50 hp, +40 energy
    REQUIRE(stats.health == 100); // clamped
    REQUIRE(stats.energy == 100); // clamped

    // Проверка нормализации вниз
    stats.health = 150;
    stats.energy = -10;
    system.Tick(reg, 0.0f);
    REQUIRE(stats.health == 100);
    REQUIRE(stats.energy == 0);
}

TEST_CASE("PlayerInputSystem sets velocity from provider", "[ecs][systems]") {
    Registry reg;
    PlayerInputSystem input;

    auto e = reg.CreateEntity();
    reg.Add<PlayerTag>(e);
    auto& vel = reg.Add<VelocityComponent>(e);
    auto& move = reg.Add<PlayerMovementComponent>(e);
    move.moveSpeed = 100.0f;

    input.SetInputProvider([]() {
        PlayerInputSystem::InputState s{};
        s.left = true;
        return s;
    });

    input.Tick(reg, 0.016f);
    REQUIRE(vel.velocity.x == Approx(-100.0f));
    REQUIRE(vel.velocity.y == Approx(0.0f));
}

TEST_CASE("SpriteRenderSystem applies transform and respects layer ordering via callback", "[ecs][systems]") {
    Registry reg;
    SpriteRenderSystem renderer;

    auto tex = std::make_shared<Texture>(); // пустая текстура-стаб

    auto makeEntity = [&](int layer, const Vector2& pos) {
        auto e = reg.CreateEntity();
        auto& t = reg.Add<TransformComponent>(e);
        t.position = pos;
        auto& s = reg.Add<SpriteComponent>(e);
        s.layer = layer;
        s.sprite.SetTexture(tex);
        return e;
    };

    makeEntity(5, {5.0f, 0.0f});   // middle opaque
    makeEntity(1, {1.0f, 0.0f});   // front opaque
    // прозрачный объект, должен рисоваться после opaque, но с тем же порядком по layer
    auto eTransparent = makeEntity(10, {10.0f, 0.0f});
    reg.Get<SpriteComponent>(eTransparent)->transparent = true;

    std::vector<float> drawOrder;
    renderer.SetDrawCallback([&](Sprite& sprite) {
        drawOrder.push_back(sprite.transform.position.x);
    });

    renderer.Tick(reg, 0.016f);

    REQUIRE(drawOrder.size() == 3);
    // Opaque слои: 1 -> 5, затем прозрачный 10
    REQUIRE(drawOrder[0] == Approx(1.0f));
    REQUIRE(drawOrder[1] == Approx(5.0f));
    REQUIRE(drawOrder[2] == Approx(10.0f));
}

TEST_CASE("CameraFollowSystem moves camera toward target", "[ecs][systems]") {
    Registry reg;
    
    // Create Camera Entity
    auto camEntity = reg.CreateEntity();
    auto& camComp = reg.Add<CameraComponent>(camEntity);
    camComp.camera = Camera2D(100.0f, 100.0f);
    camComp.camera.SetPosition({0.0f, 0.0f});
    camComp.active = true;

    auto e = reg.CreateEntity();
    auto& t = reg.Add<TransformComponent>(e);
    t.position = {50.0f, 20.0f};
    auto& follow = reg.Add<CameraFollowComponent>(e);
    follow.smoothness = 1.0f;

    CameraFollowSystem system;
    system.Tick(reg, 1.0f);

    auto camPos = camComp.camera.GetPosition();
    REQUIRE(camPos.x == Approx(50.0f));
    REQUIRE(camPos.y == Approx(20.0f));
}
