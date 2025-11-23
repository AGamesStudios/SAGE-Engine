#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Animation.h"
#include "SAGE/Graphics/ParticleSystem.h"
#include "SAGE/Graphics/ParticleEmitter.h"
#include "SAGE/Graphics/Tilemap.h"
#include "SAGE/Audio/Audio.h"
#include "SAGE/Physics/PhysicsCommon.h"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Scripting/ScriptableEntity.h"
#include "SAGE/Math/Path.h"

#include <string>
#include <memory>
#include <functional>

namespace SAGE::ECS {

struct TagComponent {
    std::string tag;
};

enum class BodyType {
    Static = 0,
    Kinematic = 1,
    Dynamic = 2
};

struct RigidBodyComponent {
    BodyType type = BodyType::Static;
    bool fixedRotation = false;
    float gravityScale = 1.0f;
    bool awake = true;
    
    Physics::BodyHandle bodyHandle;

    // Optimization: Track last synced transform to avoid unnecessary updates
    Vector2 lastSyncedPosition{0.0f, 0.0f};
    float lastSyncedRotation = 0.0f;

    bool IsValid() const { return bodyHandle.IsValid(); }
};

enum class ColliderShape {
    Box = 0,
    Circle = 1
};

struct PhysicsColliderComponent {
    ColliderShape shape = ColliderShape::Box;
    Vector2 size{32.0f, 32.0f};
    float radius = 16.0f;
    Vector2 offset{0.0f, 0.0f};
    bool isSensor = false;
    bool colliding = false;
    std::vector<Entity> contacts;
    Physics::PhysicsMaterial material;

    // Callbacks
    std::function<void(Entity)> onCollisionEnter;
    std::function<void(Entity)> onCollisionExit;
    std::function<void(Entity)> onTriggerEnter;
    std::function<void(Entity)> onTriggerExit;
};

struct PathFollowerComponent {
    std::shared_ptr<Path> path;
    float speed = 0.5f; // Speed in "t" per second (0.0 to 1.0)
    float currentT = 0.0f;
    bool active = true;
    bool loop = true;
    bool pingPong = false;
    bool reverse = false;
};

// Простой трансформ для ECS-объектов
struct TransformComponent {
    Vector2 position{0.0f, 0.0f};
    Vector2 scale{1.0f, 1.0f};
    float rotation = 0.0f;
    Vector2 origin{0.5f, 0.5f}; // pivot внутри спрайта (0.5, 0.5 = центр)

    enum class Pivot {
        TopLeft, TopCenter, TopRight,
        CenterLeft, Center, CenterRight,
        BottomLeft, BottomCenter, BottomRight
    };

    void SetPivot(Pivot pivot) {
        switch (pivot) {
            case Pivot::TopLeft:      origin = {0.0f, 0.0f}; break;
            case Pivot::TopCenter:    origin = {0.5f, 0.0f}; break;
            case Pivot::TopRight:     origin = {1.0f, 0.0f}; break;
            case Pivot::CenterLeft:   origin = {0.0f, 0.5f}; break;
            case Pivot::Center:       origin = {0.5f, 0.5f}; break;
            case Pivot::CenterRight:  origin = {1.0f, 0.5f}; break;
            case Pivot::BottomLeft:   origin = {0.0f, 1.0f}; break;
            case Pivot::BottomCenter: origin = {0.5f, 1.0f}; break;
            case Pivot::BottomRight:  origin = {1.0f, 1.0f}; break;
        }
    }
};

// Отображаемый спрайт
struct SpriteComponent {
    Sprite sprite;
    bool visible = true;
    int layer = 0;
    bool transparent = false; // подсказка для сортировки: прозрачные рендерятся после непрозрачных
};

struct PlatformBehaviorComponent {
    bool stayOnPlatform = true;
    float edgeLookAhead = 20.0f; // How far ahead to check for edge
};

struct AnimationComponent {
    Animator animator;
    std::string currentClip;
    bool playing = true;
};

// Простейшая физика / движение
struct VelocityComponent {
    Vector2 velocity{0.0f, 0.0f};
    float angularVelocity = 0.0f;
};

// Здоровье
struct HealthComponent {
    int maxHealth = 100;
    int currentHealth = 100;
    bool IsDead() const { return currentHealth <= 0; }
};

// Общая статистика сущности
struct StatsComponent {
    int health = 100;
    int maxHealth = 100;
    int energy = 100;
    int maxEnergy = 100;
};

// Маркеры
struct PlayerTag {};
struct EnemyTag {};

// Камера следует за сущностью (обычно Player)
struct CameraFollowComponent {
    float smoothness = 5.0f;
};

struct CameraComponent {
    Camera2D camera;
    bool active = true;
};

// Проигрывание звука
struct AudioComponent {
    std::string path; // Path to sound file
    std::shared_ptr<Sound> sound;
    bool loop = false;
    bool playRequested = false; // выставляется игровой логикой
    float volume = 1.0f;
    
    // Spatial Audio
    bool spatial = false;
    float minDistance = 100.0f;
    float maxDistance = 1000.0f;
};

struct PlayerMovementComponent {
    float moveSpeed = 250.0f;
    float jumpImpulse = 0.0f;
    bool canJump = false;
};

// Простой collider (AABB + Circle)
struct ColliderComponent {
    ColliderShape shape = ColliderShape::Box;
    Vector2 size{32.0f, 32.0f};
    float radius = 16.0f;
    Vector2 offset{0.0f, 0.0f};
    bool isTrigger = false;
    bool colliding = false;
    
    // Physics properties
    float density = 1.0f;
    float friction = 0.3f;
    float restitution = 0.0f;
};

// Компонент ввода (заполняется InputSystem)
struct InputComponent {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool jump = false;
    bool attack = false;
};

// Частица (для систем частиц)
struct ParticleComponent {
    std::shared_ptr<ParticleSystem> system;
    bool active = true;
};

struct ParticleEmitterComponent {
    ParticleEmitter emitter;
    std::shared_ptr<ParticleSystem> system;
    bool active = true;
    bool playing = true;
    float emissionTimer = 0.0f;
    float emissionRate = 10.0f;
};

struct DamageOnCollisionComponent {
    int damageAmount = 10;
    float cooldown = 1.0f;
    float timeSinceLastDamage = 0.0f;
    bool damageOnce = false;
    bool hasDealtDamage = false;
};

// Tilemap component
struct TilemapComponent {
    std::shared_ptr<Tilemap> tilemap;
    bool visible = true;
};

struct NativeScriptComponent {
    std::shared_ptr<ScriptableEntity> Instance;
    std::function<ScriptableEntity*()> InstantiateScript;

    template<typename T>
    void Bind() {
        InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
    }
};

// Script component (for data-driven scripts like Lua/Python)
struct ScriptComponent {
    std::string scriptName;
    bool active = true;
};

// Tile component (for individual tile entities)
struct TileComponent {
    int gridX = 0;
    int gridY = 0;
    int tileId = 0;
    int layer = 0;
    bool collision = false;
};

// UI Component
enum class UIType {
    Panel,
    Button,
    Text,
    Image,
    Slider,
    Checkbox,
    Input
};

struct UIComponent {
    UIType type = UIType::Panel;
    
    // Layout
    Vector2 size{100.0f, 30.0f};
    Vector2 anchor{0.5f, 0.5f}; // 0,0 = top-left, 1,1 = bottom-right relative to parent/screen
    
    // Visuals
    Color color{1.0f, 1.0f, 1.0f, 1.0f};
    Color hoverColor{0.9f, 0.9f, 0.9f, 1.0f};
    Color pressedColor{0.7f, 0.7f, 0.7f, 1.0f};
    std::shared_ptr<Texture> texture; // For Image or Button background
    
    // Text
    std::string text;
    std::string fontPath;
    float fontSize = 20.0f;
    Color textColor{0.0f, 0.0f, 0.0f, 1.0f};
    Vector2 textPadding{5.0f, 5.0f};
    
    // State
    bool interactable = true;
    bool isHovered = false;
    bool isPressed = false;
    bool isFocused = false;
    bool isChecked = false; // Checkbox
    float value = 0.0f;     // Slider
    float minValue = 0.0f;
    float maxValue = 1.0f;
    
    // Callbacks (names for serialization/binding)
    std::string onClickMethod;
    std::string onValueChangedMethod;
    std::string onSubmitMethod;
};

} // namespace SAGE::ECS
