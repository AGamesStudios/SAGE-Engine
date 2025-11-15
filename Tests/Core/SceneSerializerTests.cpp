#include "TestFramework.h"
#include "Core/SceneSerializer.h"
#include "Core/Scene.h"
#include "ECS/Registry.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/SpriteComponent.h"

#include <fstream>
#include <filesystem>

using namespace SAGE;

TEST_CASE("SceneSerializer: Save and Load Basic Scene") {
    // Создать тестовую сцену
    Scene scene("TestScene");
    auto& registry = scene.GetECS().GetRegistry();
    
    // Создать сущность с компонентами
    ECS::Entity entity = registry.CreateEntity();
    
    auto& transform = registry.AddComponent(entity, ECS::TransformComponent{});
    transform.position = Vector2(100.0f, 200.0f);
    transform.rotation = 45.0f;
    transform.scale = Vector2(2.0f, 2.0f);
    
    auto& sprite = registry.AddComponent(entity, ECS::SpriteComponent{});
    sprite.texturePath = "player.png";
    sprite.tint = Color(255, 128, 64, 255);
    sprite.alpha = 0.8f;
    sprite.visible = true;
    sprite.layer = 10;
    
    // Сохранить
    const std::string testFile = "test_scene.json";
    bool saved = SceneSerializer::SaveToFile(&scene, testFile);
    ASSERT_TRUE(saved, "Scene should save successfully");
    ASSERT_TRUE(std::filesystem::exists(testFile), "Scene file should exist");
    
    // Загрузить в новую сцену
    Scene loadedScene("LoadedScene");
    bool loaded = SceneSerializer::LoadFromFile(&loadedScene, testFile);
    ASSERT_TRUE(loaded, "Scene should load successfully");
    
    // Проверить данные
    auto& loadedRegistry = loadedScene.GetECS().GetRegistry();
    auto loadedEntities = loadedRegistry.GetEntities();
    ASSERT_EQ(loadedEntities.size(), 1, "Should have 1 entity");
    
    ECS::Entity loadedEntity = loadedEntities[0];
    
    auto* loadedTransform = loadedRegistry.GetComponent<ECS::TransformComponent>(loadedEntity);
    ASSERT_NOT_NULL(loadedTransform, "Transform component should exist");
    ASSERT_EQ(loadedTransform->position.x, 100.0f, "Position X should match");
    ASSERT_EQ(loadedTransform->position.y, 200.0f, "Position Y should match");
    ASSERT_EQ(loadedTransform->rotation, 45.0f, "Rotation should match");
    
    auto* loadedSprite = loadedRegistry.GetComponent<ECS::SpriteComponent>(loadedEntity);
    ASSERT_NOT_NULL(loadedSprite, "Sprite component should exist");
    ASSERT_EQ(loadedSprite->texturePath, "player.png", "Texture path should match");
    ASSERT_EQ(loadedSprite->layer, 10, "Layer should match");
    
    // Cleanup
    std::filesystem::remove(testFile);
    
    PASS();
}

TEST_CASE("SceneSerializer: Save Multiple Entities") {
    Scene scene("MultiEntityScene");
    auto& registry = scene.GetECS().GetRegistry();
    
    // Создать 3 сущности
    for (int i = 0; i < 3; i++) {
        ECS::Entity e = registry.CreateEntity();
        auto& t = registry.AddComponent(e, ECS::TransformComponent{});
        t.position = Vector2(static_cast<float>(i * 50), static_cast<float>(i * 100));
    }
    
    const std::string testFile = "multi_entity_scene.json";
    bool saved = SceneSerializer::SaveToFile(&scene, testFile);
    ASSERT_TRUE(saved, "Multi-entity scene should save");
    
    Scene loadedScene("Loaded");
    bool loaded = SceneSerializer::LoadFromFile(&loadedScene, testFile);
    ASSERT_TRUE(loaded, "Multi-entity scene should load");
    
    auto& loadedRegistry = loadedScene.GetECS().GetRegistry();
    ASSERT_EQ(loadedRegistry.GetEntities().size(), 3, "Should have 3 entities");
    
    std::filesystem::remove(testFile);
    PASS();
}

TEST_CASE("SceneSerializer: Invalid File Handling") {
    Scene scene("Test");
    
    // Попытка загрузить несуществующий файл
    bool loaded = SceneSerializer::LoadFromFile(&scene, "non_existent_file.json");
    ASSERT_FALSE(loaded, "Should fail to load non-existent file");
    
    // Попытка сохранить null сцену
    bool saved = SceneSerializer::SaveToFile(nullptr, "test.json");
    ASSERT_FALSE(saved, "Should fail to save null scene");
    
    PASS();
}

TEST_CASE("SceneSerializer: Empty Scene Handling") {
    Scene scene("EmptyScene");
    
    const std::string testFile = "empty_scene.json";
    bool saved = SceneSerializer::SaveToFile(&scene, testFile);
    ASSERT_TRUE(saved, "Empty scene should save");
    
    Scene loadedScene("Loaded");
    bool loaded = SceneSerializer::LoadFromFile(&loadedScene, testFile);
    ASSERT_TRUE(loaded, "Empty scene should load");
    
    auto& loadedRegistry = loadedScene.GetECS().GetRegistry();
    ASSERT_EQ(loadedRegistry.GetEntities().size(), 0, "Should have 0 entities");
    
    std::filesystem::remove(testFile);
    PASS();
}

TEST_CASE("SceneSerializer: JSON Format Validation") {
    Scene scene("FormatTest");
    auto& registry = scene.GetECS().GetRegistry();
    
    ECS::Entity e = registry.CreateEntity();
    auto& t = registry.AddComponent(e, ECS::TransformComponent{});
    t.position = Vector2(10.0f, 20.0f);
    
    const std::string testFile = "format_test.json";
    SceneSerializer::SaveToFile(&scene, testFile);
    
    // Проверить формат JSON вручную
    std::ifstream file(testFile);
    ASSERT_TRUE(file.is_open(), "File should be readable");
    
    nlohmann::json j = nlohmann::json::parse(file);
    file.close();
    
    ASSERT_TRUE(j.contains("name"), "JSON should contain 'name' field");
    ASSERT_TRUE(j.contains("entities"), "JSON should contain 'entities' field");
    ASSERT_TRUE(j["entities"].is_array(), "Entities should be array");
    ASSERT_EQ(j["entities"].size(), 1, "Should have 1 entity in JSON");
    
    auto& entityJson = j["entities"][0];
    ASSERT_TRUE(entityJson.contains("transform"), "Entity should have transform");
    ASSERT_EQ(entityJson["transform"]["position"][0], 10.0f, "Position X in JSON should match");
    
    std::filesystem::remove(testFile);
    PASS();
}

void RegisterSceneSerializerTests() {
    RUN_TEST(SceneSerializer: Save and Load Basic Scene);
    RUN_TEST(SceneSerializer: Save Multiple Entities);
    RUN_TEST(SceneSerializer: Invalid File Handling);
    RUN_TEST(SceneSerializer: Empty Scene Handling);
    RUN_TEST(SceneSerializer: JSON Format Validation);
}
