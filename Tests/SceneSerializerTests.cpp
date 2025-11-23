#include "SAGE/Core/Scene.h"
#include "SAGE/Core/SceneSerializer.h"
#include "SAGE/Core/ECSComponents.h"
#include "catch2.hpp"
#include <filesystem>

TEST_CASE("Scene Serialization", "[Scene]") {
    using namespace SAGE;
    using namespace SAGE::ECS;

    class TestScene : public Scene {
    public:
        TestScene(const std::string& name) : Scene(name) {}
        void OnEnter(const TransitionContext& context) override {}
        void OnExit() override {}
        void OnRender() override {}
    };

    SECTION("Serialize and Deserialize") {
        auto scene = std::make_shared<TestScene>("TestScene");
        Entity entity = scene->CreateEntity();
        
        // Add Tag
        auto& tag = scene->GetRegistry().Add<TagComponent>(entity);
        tag.tag = "Test Entity";

        // Add Transform
        auto& tc = scene->GetRegistry().Add<TransformComponent>(entity);
        tc.position = { 1.0f, 2.0f };
        tc.scale = { 2.0f, 2.0f };
        tc.rotation = 45.0f;

        // Add Sprite
        auto& sc = scene->GetRegistry().Add<SpriteComponent>(entity);
        sc.sprite.tint = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red

        // Serialize
        SceneSerializer serializer(scene.get());
        serializer.Serialize("test_scene.json");

        // Create new scene for deserialization
        auto newScene = std::make_shared<TestScene>("TestScene");
        SceneSerializer newSerializer(newScene.get());

        // Deserialize
        bool success = newSerializer.Deserialize("test_scene.json");
        REQUIRE(success);

        // Verify
        bool found = false;
        newScene->GetRegistry().ForEachEntity([&](Entity e) {
            if (newScene->GetRegistry().Has<TagComponent>(e)) {
                auto* tagComp = newScene->GetRegistry().Get<TagComponent>(e);
                if (tagComp) {
                    if (tagComp->tag == "Test Entity") {
                        found = true;
                        
                        // Check Transform
                        REQUIRE(newScene->GetRegistry().Has<TransformComponent>(e));
                        auto* tc2 = newScene->GetRegistry().Get<TransformComponent>(e);
                        REQUIRE(tc2 != nullptr);
                        REQUIRE(tc2->position.x == Catch::Approx(1.0f));
                        REQUIRE(tc2->position.y == Catch::Approx(2.0f));
                        REQUIRE(tc2->scale.x == Catch::Approx(2.0f));
                        REQUIRE(tc2->scale.y == Catch::Approx(2.0f));
                        REQUIRE(tc2->rotation == Catch::Approx(45.0f));

                        // Check Sprite
                        REQUIRE(newScene->GetRegistry().Has<SpriteComponent>(e));
                        auto* sc2 = newScene->GetRegistry().Get<SpriteComponent>(e);
                        REQUIRE(sc2 != nullptr);
                        REQUIRE(sc2->sprite.tint.r == Catch::Approx(1.0f));
                        REQUIRE(sc2->sprite.tint.g == Catch::Approx(0.0f));
                        REQUIRE(sc2->sprite.tint.b == Catch::Approx(0.0f));
                        REQUIRE(sc2->sprite.tint.a == Catch::Approx(1.0f));
                    }
                }
            }
        });

        REQUIRE(found);

        // Cleanup
        std::filesystem::remove("test_scene.json");
    }
}
