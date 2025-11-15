#include "TestFramework.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Core/SceneStack.h"
#include "ECS/GameObjectECSBridge.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"

using namespace SAGE;
using namespace SAGE::ECS;

class BridgeTestScene : public Scene {
public:
    BridgeTestScene() : Scene("BridgeScene") {}
};

TEST_CASE(ECS_GameObjectBridge_CreateAndSync) {
    // Создаём сцену вручную (без приложения)
    BridgeTestScene scene;

    // Создаём игровой объект
    GameObject* go = GameObject::Create("Player");
    go->x = 42.0f;
    go->y = 17.0f;
    go->width = 64.0f;
    go->height = 32.0f;
    go->angle = 90.0f;
    go->color = Color::Red();
    go->alpha = 0.5f;

    // Принудительно назначаем сцену (эмуляция создания при активной сцене)
    // Normally set in Create when Application exists; here manually.
    // Ensure entity creation
    GameObjectECSBridge::EnsureEntity(&scene, go);
    GameObjectECSBridge::Sync(&scene, go);

    auto& registry = scene.GetECS().GetRegistry();

    // Ищем сущность через внутреннюю карту (проверяем косвенно через компонент)
    auto entitiesWithTransform = registry.GetAllWith<TransformComponent>();
    ASSERT_EQ(entitiesWithTransform.size(), 1u);

    Entity e = entitiesWithTransform[0].entity;
    ASSERT_TRUE(registry.ContainsEntity(e));

    auto* t = registry.GetComponent<TransformComponent>(e);
    ASSERT_NOT_NULL(t);
    ASSERT_EQ(t->position.x, 42.0f);
    ASSERT_EQ(t->position.y, 17.0f);
    ASSERT_EQ(t->GetRotation(), 90.0f);
    ASSERT_EQ(t->size.x, 64.0f);
    ASSERT_EQ(t->size.y, 32.0f);

    auto* s = registry.GetComponent<SpriteComponent>(e);
    ASSERT_NOT_NULL(s);
    ASSERT_EQ(s->tint.a, 0.5f);
    ASSERT_EQ(s->tint.r, Color::Red().r);
}

TEST_CASE(ECS_GameObjectBridge_DestroyRemovesEntity) {
    BridgeTestScene scene;
    GameObject* go = GameObject::Create("Temp");
    GameObjectECSBridge::EnsureEntity(&scene, go);
    GameObjectECSBridge::Sync(&scene, go);

    auto& registry = scene.GetECS().GetRegistry();
    ASSERT_EQ(registry.GetEntities().size(), 1u);

    go->Destroy();
    GameObject::DestroyMarked();

    // После удаления - сущность также должна исчезнуть
    ASSERT_EQ(registry.GetEntities().size(), 0u);
}
