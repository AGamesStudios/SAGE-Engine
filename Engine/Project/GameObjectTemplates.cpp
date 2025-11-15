#include "GameObjectTemplates.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "ECS/Components/Visual/CameraComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "ECS/Components/Gameplay/PlayerMovementComponent.h"
#include "Core/Logger.h"

namespace SAGE {

using namespace ECS;

Entity GameObjectTemplates::CreateCamera(
    Registry& registry,
    const Vector2& position,
    float width,
    float height,
    bool isMain)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    registry.AddComponent(entity, transform);
    
    // Camera
    CameraComponent camera;
    camera.width = static_cast<int>(width);
    camera.height = static_cast<int>(height);
    camera.zoom = 1.0f;
    camera.isMain = isMain;
    registry.AddComponent(entity, camera);
    
    SAGE_INFO("✓ Camera created at ({}, {})", position.x, position.y);
    return entity;
}

Entity GameObjectTemplates::CreatePlayer(
    Registry& registry,
    const Vector2& position,
    const Vector2& size)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = size;
    registry.AddComponent(entity, transform);
    
    // Sprite (синий цвет для игрока)
    SpriteComponent sprite;
    sprite.tint = Color(0, 128, 255, 255); // Синий
    sprite.layer = 10; // Выше других объектов
    registry.AddComponent(entity, sprite);
    
    // Physics
    PhysicsComponent physics;
    physics.bodyType = PhysicsBodyType::Dynamic;
    physics.fixedRotation = true; // Не поворачивается
    physics.density = 1.0f;
    physics.friction = 0.3f;
    registry.AddComponent(entity, physics);
    
    // Player Movement
    PlayerMovementComponent movement;
    movement.speed = 200.0f;
    movement.jumpForce = 400.0f;
    movement.mode = PlayerMovementMode::Platformer;
    registry.AddComponent(entity, movement);
    
    SAGE_INFO("✓ Player created at ({}, {})", position.x, position.y);
    return entity;
}

Entity GameObjectTemplates::CreatePlatform(
    Registry& registry,
    const Vector2& position,
    const Vector2& size)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = size;
    registry.AddComponent(entity, transform);
    
    // Sprite (серый цвет)
    SpriteComponent sprite;
    sprite.tint = Color(100, 100, 100, 255);
    sprite.layer = 0;
    registry.AddComponent(entity, sprite);
    
    // Physics (статическая платформа)
    PhysicsComponent physics;
    physics.bodyType = PhysicsBodyType::Static;
    physics.friction = 0.8f;
    registry.AddComponent(entity, physics);
    
    SAGE_INFO("✓ Platform created at ({}, {}) size ({}, {})", 
              position.x, position.y, size.x, size.y);
    return entity;
}

Entity GameObjectTemplates::CreateEnemy(
    Registry& registry,
    const Vector2& position,
    const Vector2& size)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = size;
    registry.AddComponent(entity, transform);
    
    // Sprite (красный цвет для врага)
    SpriteComponent sprite;
    sprite.tint = Color(255, 50, 50, 255); // Красный
    sprite.layer = 10;
    registry.AddComponent(entity, sprite);
    
    // Physics
    PhysicsComponent physics;
    physics.bodyType = PhysicsBodyType::Dynamic;
    physics.fixedRotation = true;
    physics.density = 0.8f;
    registry.AddComponent(entity, physics);
    
    SAGE_INFO("✓ Enemy created at ({}, {})", position.x, position.y);
    return entity;
}

Entity GameObjectTemplates::CreateCollectible(
    Registry& registry,
    const Vector2& position)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = Vector2(20, 20);
    registry.AddComponent(entity, transform);
    
    // Sprite (желтый цвет - монета)
    SpriteComponent sprite;
    sprite.tint = Color(255, 215, 0, 255); // Золотой
    sprite.layer = 5;
    registry.AddComponent(entity, sprite);
    
    // Physics (sensor - проходимый триггер)
    PhysicsComponent physics;
    physics.bodyType = PhysicsBodyType::Static;
    physics.isSensor = true;
    registry.AddComponent(entity, physics);
    
    SAGE_INFO("✓ Collectible created at ({}, {})", position.x, position.y);
    return entity;
}

Entity GameObjectTemplates::CreateSprite(
    Registry& registry,
    const Vector2& position,
    const Vector2& size,
    const Color& color,
    const std::string& texturePath)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = size;
    registry.AddComponent(entity, transform);
    
    // Sprite
    SpriteComponent sprite;
    sprite.tint = color;
    sprite.texturePath = texturePath;
    sprite.layer = 0;
    registry.AddComponent(entity, sprite);
    
    SAGE_INFO("✓ Sprite created at ({}, {})", position.x, position.y);
    return entity;
}

Entity GameObjectTemplates::CreateText(
    Registry& registry,
    const std::string& text,
    const Vector2& position,
    float fontSize)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    registry.AddComponent(entity, transform);
    
    // TODO: TextComponent когда будет готов
    SAGE_INFO("✓ Text created: '{}'", text);
    return entity;
}

Entity GameObjectTemplates::CreateBackground(
    Registry& registry,
    const std::string& texturePath,
    int layer)
{
    Entity entity = registry.CreateEntity();
    
    // Transform (большой размер для фона)
    TransformComponent transform;
    transform.position = Vector2::Zero();
    transform.size = Vector2(1920, 1080); // Стандартный размер
    registry.AddComponent(entity, transform);
    
    // Sprite
    SpriteComponent sprite;
    sprite.texturePath = texturePath;
    sprite.layer = layer; // Отрицательный - на заднем плане
    sprite.tint = Color::White();
    registry.AddComponent(entity, sprite);
    
    SAGE_INFO("✓ Background created: {}", texturePath);
    return entity;
}

Entity GameObjectTemplates::CreateButton(
    Registry& registry,
    const std::string& text,
    const Vector2& position,
    const Vector2& size)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    transform.size = size;
    registry.AddComponent(entity, transform);
    
    // Sprite (фон кнопки)
    SpriteComponent sprite;
    sprite.tint = Color(70, 70, 70, 255); // Тёмно-серый
    sprite.layer = 100; // UI слой
    registry.AddComponent(entity, sprite);
    
    // TODO: UIButton component
    SAGE_INFO("✓ Button created: '{}'", text);
    return entity;
}

Entity GameObjectTemplates::CreateParticleEffect(
    Registry& registry,
    const Vector2& position,
    int particleCount)
{
    Entity entity = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = position;
    registry.AddComponent(entity, transform);
    
    // TODO: ParticleSystem component
    SAGE_INFO("✓ Particle effect created at ({}, {})", position.x, position.y);
    return entity;
}

} // namespace SAGE
