#include "TestFramework.h"

#include <SAGE.h>
#include <cmath>

using namespace SAGE;

TEST_CASE(GameObject_PhysicsIntegration) {
    GameObject::DestroyAll();

    GameObject* object = GameObject::Create("dynamic_object");
    REQUIRE(object != nullptr);
    object->physics = true;
    object->gravity = 0.0f;
    object->gravityScale = 0.0f;
    object->friction = 0.0f;
    object->SetMass(2.0f);
    object->ApplyForce(Vector2(4.0f, -2.0f));

    GameObject::UpdateAll(1.0f);

    Vector2 velocity = object->GetVelocity();
    CHECK(std::abs(velocity.x - 2.0f) < 0.0001f);
    CHECK(std::abs(velocity.y + 1.0f) < 0.0001f);
    CHECK(std::abs(object->x - 2.0f) < 0.0001f);
    CHECK(std::abs(object->y + 1.0f) < 0.0001f);

    GameObject::DestroyAll();
}

TEST_CASE(GameObject_CollisionGrounding) {
    GameObject::DestroyAll();

    GameObject* floor = GameObject::Create("floor");
    REQUIRE(floor != nullptr);
    floor->physics = false;
    floor->collision = true;
    floor->solid = true;
    floor->width = 200.0f;
    floor->height = 20.0f;
    floor->MoveTo(0.0f, 150.0f);

    GameObject* player = GameObject::Create("player");
    REQUIRE(player != nullptr);
    player->physics = true;
    player->collision = true;
    player->solid = true;
    player->gravity = 0.0f;
    player->gravityScale = 0.0f;
    player->friction = 0.0f;
    player->width = 50.0f;
    player->height = 50.0f;
    player->MoveTo(10.0f, floor->y - player->height + 10.0f);

    bool collisionEnterCalled = false;
    player->OnCollisionEnter = [&](GameObject* other) {
        if (other == floor) {
            collisionEnterCalled = true;
        }
    };

    GameObject::UpdateAll(0.016f);

    CHECK(collisionEnterCalled);
    CHECK(player->IsGrounded());
    CHECK(std::abs(player->y - (floor->y - player->height)) < 0.0001f);

    GameObject::DestroyAll();
}
