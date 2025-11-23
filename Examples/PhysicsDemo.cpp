#include "SAGE/SAGE.h"
#include "SAGE/Core/ECSGame.h"
#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Core/ECSComponents.h"

using namespace SAGE;
using namespace SAGE::ECS;

class PhysicsDemo : public ECSGame {
public:
    PhysicsDemo() : ECSGame({ "Physics Demo", 1280, 720 }) {}

protected:
    void OnInit() override {
        ECSGame::OnInit();
        
        SetDebugPhysics(true);
        
        auto& reg = GetRegistry();
        
        // Create Floor
        Entity floor = reg.CreateEntity();
        auto& floorTrans = reg.Add<TransformComponent>(floor);
        floorTrans.position = {640.0f, 680.0f};
        
        reg.Add<RigidBodyComponent>(floor, BodyType::Static);
        auto& floorCol = reg.Add<PhysicsColliderComponent>(floor);
        floorCol.shape = ColliderShape::Box;
        floorCol.size = {1200.0f, 50.0f};
        floorCol.material.friction = 0.5f;

        // Test Collision Callback
        floorCol.onCollisionEnter = [&reg](Entity other) {
            if (auto* sprite = reg.Get<SpriteComponent>(other)) {
                sprite->sprite.tint = Color::Green();
            }
        };

        // Create Sensor Trigger
        Entity sensor = reg.CreateEntity();
        auto& sensorTrans = reg.Add<TransformComponent>(sensor);
        sensorTrans.position = {640.0f, 400.0f};
        
        reg.Add<RigidBodyComponent>(sensor, BodyType::Static);
        auto& sensorCol = reg.Add<PhysicsColliderComponent>(sensor);
        sensorCol.shape = ColliderShape::Box;
        sensorCol.size = {800.0f, 100.0f};
        sensorCol.isSensor = true; // Make it a trigger
        
        // Visual for sensor
        auto& sensorSprite = reg.Add<SpriteComponent>(sensor);
        sensorSprite.sprite = Sprite(Texture::CreateWhiteTexture());
        sensorSprite.sprite.tint = {1.0f, 1.0f, 0.0f, 0.3f}; // Transparent Yellow
        sensorSprite.sprite.transform.scale = {800.0f, 100.0f};
        sensorSprite.transparent = true;

        // Test Trigger Callbacks
        sensorCol.onTriggerEnter = [&reg](Entity other) {
            if (auto* sprite = reg.Get<SpriteComponent>(other)) {
                sprite->sprite.tint = Color::Blue();
            }
        };

        sensorCol.onTriggerExit = [&reg](Entity other) {
            if (auto* sprite = reg.Get<SpriteComponent>(other)) {
                sprite->sprite.tint = Color::Red();
            }
        };
        
        // Create falling boxes
        for (int i = 0; i < 10; ++i) {
            Entity box = reg.CreateEntity();
            auto& boxTrans = reg.Add<TransformComponent>(box);
            boxTrans.position = {640.0f + (i % 2 == 0 ? -10.0f : 10.0f) * i, 100.0f - i * 60.0f};
            
            reg.Add<RigidBodyComponent>(box, BodyType::Dynamic);
            auto& boxCol = reg.Add<PhysicsColliderComponent>(box);
            boxCol.shape = ColliderShape::Box;
            boxCol.size = {40.0f, 40.0f};
            boxCol.material.density = 1.0f;
            boxCol.material.friction = 0.3f;
            boxCol.material.restitution = 0.5f; // Bouncy
            
            // Add sprite for visualization
            auto& sprite = reg.Add<SpriteComponent>(box);
            sprite.sprite = Sprite(Texture::CreateWhiteTexture());
            sprite.sprite.tint = Color::Red();
            sprite.sprite.transform.scale = {40.0f, 40.0f};
            sprite.sprite.transform.origin = {0.5f, 0.5f};
        }
    }

    void OnUpdate(double deltaTime) override {
        ECSGame::OnUpdate(deltaTime);

        // Test Raycast / Clicking
        if (Input::IsMouseButtonPressed(MouseButton::Left)) {
            Vector2 mousePos = Input::GetMousePosition();
            if (auto* raycastSys = GetRaycastSystem()) {
                Entity hit = raycastSys->RaycastFromScreen(GetRegistry(), mousePos, GetCamera());
                if (hit != kInvalidEntity) {
                    Logger::Info("Clicked Entity: {}", hit);
                    if (auto* sprite = GetRegistry().Get<SpriteComponent>(hit)) {
                        // Random color on click
                        sprite->sprite.tint = {
                            static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX,
                            1.0f
                        };
                    }
                }
            }
        }
    }
};

int main() {
    PhysicsDemo app;
    app.Run();
    return 0;
}
