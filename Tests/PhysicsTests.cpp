#include "catch2.hpp"
#include "SAGE/Physics/PhysicsWorld.h"
#include "SAGE/Math/Vector2.h"

using namespace SAGE;
using namespace SAGE::Physics;
using Catch::Approx;

TEST_CASE("PhysicsWorld RayCast", "[physics]") {
    PhysicsWorld world;
    
    // Create a ground box
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = {0.0f, 0.0f};
    BodyHandle ground = world.CreateBody(bodyDef);
    
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2Polygon box = b2MakeBox(10.0f, 1.0f); // 20x2 box centered at 0,0
    b2CreatePolygonShape(ToB2BodyId(ground), &shapeDef, &box);
    
    // Raycast hitting the box
    // Ray from (0, 10) to (0, -10) should hit at (0, 1)
    PhysicsWorld::RayCastHit hit = world.RayCast({0.0f, 10.0f}, {0.0f, -10.0f});
    
    REQUIRE(hit.hit == true);
    REQUIRE(hit.point.x == Approx(0.0f));
    REQUIRE(hit.point.y == Approx(1.0f).margin(0.1f));
    REQUIRE(hit.normal.x == Approx(0.0f));
    REQUIRE(hit.normal.y == Approx(1.0f));
    
    // Raycast missing the box
    PhysicsWorld::RayCastHit miss = world.RayCast({20.0f, 10.0f}, {20.0f, -10.0f});
    REQUIRE(miss.hit == false);
}
