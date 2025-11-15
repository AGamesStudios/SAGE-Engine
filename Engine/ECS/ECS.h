#pragma once

/// @file ECS.h
/// @brief Главный заголовочный файл для Entity Component System
/// 
/// Подключите этот файл для использования ECS в вашем проекте:
/// @code
/// #include "ECS/ECS.h"
/// 
/// using namespace SAGE::ECS;
/// 
/// Registry registry;
/// Entity player = registry.CreateEntity();
/// registry.AddComponent(player, TransformComponent(100, 200));
/// @endcode

// Core ECS infrastructure
#include "Entity.h"
#include "Registry.h"
#include "ComponentPool.h"
#include "System.h"

// === CORE COMPONENTS ===
#include "Components/Core/TransformComponent.h"
#include "Components/Core/ScriptComponent.h"

// === PHYSICS COMPONENTS ===
#include "Components/Physics/PhysicsComponent.h"
#include "Components/Physics/ColliderComponent.h"

// === VISUAL COMPONENTS ===
#include "Components/Visual/SpriteComponent.h"
#include "Components/Visual/SpriteComponentLoader.h"
#include "Components/Visual/CameraComponent.h"
#include "Components/Visual/AnimationComponent.h"
#include "Components/Visual/TilemapComponent.h"

// === AUDIO COMPONENTS ===
#include "Components/Audio/AudioComponent.h"

// === EFFECTS COMPONENTS ===
#include "Components/Effects/ParticleSystemComponent.h"
#include "Components/Effects/ScreenEffectsComponent.h"
#include "Components/Effects/TrailComponent.h"

// === UI COMPONENTS ===
#include "Components/UI/NineSliceComponent.h"

// === GAMEPLAY COMPONENTS ===
#include "Components/Gameplay/PlayerMovementComponent.h"
#include "Components/Gameplay/InventoryComponent.h"

// === SYSTEMS ===
// Core Systems
#include "Systems/Core/ScriptSystem.h"

// Visual Systems
#include "Systems/Visual/RenderSystem.h"
#include "Systems/Visual/AnimationSystem.h"
#include "Systems/Visual/NineSliceRenderSystem.h"

// Physics Systems
#include "Systems/Physics/PhysicsSystem.h"

// Audio Systems
#include "Systems/Audio/AudioPlaybackSystem.h"

// Effects Systems
#include "Systems/Effects/ParticleUpdateSystem.h"
#include "Systems/Effects/ScreenEffectsSystem.h"
#include "Systems/Effects/TrailUpdateSystem.h"

// Gameplay Systems
// (None currently - PlayerMovementSystem can be added later)

namespace SAGE::ECS {

/// @brief Создать сущность с Transform и Sprite компонентами
/// @param registry Реестр ECS
/// @param x Позиция X
/// @param y Позиция Y
/// @param texturePath Путь к текстуре спрайта
/// @return ID созданной сущности
inline Entity CreateSprite(Registry& registry, float x, float y, const std::string& texturePath) {
    Entity entity = registry.CreateEntity();
    registry.AddComponent(entity, TransformComponent(x, y));
    SpriteComponent sprite(texturePath);
    Detail::ResolveSpriteTexture(sprite);
    registry.AddComponent(entity, sprite);
    return entity;
}

} // namespace SAGE::ECS
