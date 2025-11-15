# SAGE Engine - API Reference

**Версия:** Alpha  
**Язык:** C++17  
**Namespace:** SAGE

> **Важно:** Все примеры предполагают использование `using namespace SAGE;` для краткости.
> В production коде рекомендуется использовать полные квалификаторы.

---

## Обработка ошибок

### Общие принципы

SAGE Engine использует следующие подходы к обработке ошибок:

1. **Возврат nullptr** - для методов получения ресурсов/компонентов
2. **Возврат bool** - для операций успех/неудача
3. **Assertions** - для критических ошибок в Debug сборке
4. **Логирование** - для некритических проблем

### Примеры проверки ошибок

```cpp
// Проверка компонента
auto* transform = registry.GetComponent<TransformComponent>(entity);
if (!transform) {
    SAGE_WARNING("Game", "Entity {} has no TransformComponent", entity);
    return;
}

// Проверка загрузки ресурса
auto texture = ResourceManager::Get().LoadTexture("sprite.png");
if (!texture) {
    SAGE_ERROR("Resources", "Failed to load texture");
    // Использовать placeholder текстуру
    texture = ResourceManager::Get().LoadTexture("error.png");
}

// Проверка инициализации системы
if (!audioSystem.Init()) {
    SAGE_ERROR("Audio", "Failed to initialize audio system");
    // Продолжить без звука или завершить
}
```

### Макросы логирования

```cpp
SAGE_TRACE("Category", "Message {}", arg);    // Детальная отладка
SAGE_INFO("Category", "Message {}", arg);     // Информация
SAGE_WARNING("Category", "Message {}", arg);  // Предупреждение
SAGE_ERROR("Category", "Message {}", arg);    // Ошибка
```

---

## Потоко-безопасность

### Thread-Safe компоненты

**Потоко-безопасные:**
- `ResourceManager` - внутренняя синхронизация через mutex
- `EventBus` - lock-free для immediate dispatch
- `Logger` - потоко-безопасная запись

**НЕ потоко-безопасные:**
- `Registry` - требует внешней синхронизации
- `PhysicsSystem` - должен обновляться в одном потоке
- `RenderSystem` - только в основном потоке
- `InputManager` - только в основном потоке

### Рекомендации

```cpp
// Хорошо - все ECS операции в одном потоке
void Update() {
    registry.ForEach<TransformComponent>([](Entity e, TransformComponent& t) {
        // Безопасно
    });
}

// Плохо - доступ к Registry из разных потоков
std::thread worker([&registry]() {
    // ОПАСНО! Undefined behavior
    registry.CreateEntity();
});

// Правильно - результаты в потоке, применение в главном
struct Task { Entity entity; Vector2 newPosition; };
std::queue<Task> taskQueue;
std::mutex queueMutex;

// В рабочем потоке
std::thread([&]() {
    Task task;
    task.entity = someEntity;
    task.newPosition = CalculatePosition();
    
    std::lock_guard lock(queueMutex);
    taskQueue.push(task);
});

// В главном потоке
void Update() {
    std::lock_guard lock(queueMutex);
    while (!taskQueue.empty()) {
        auto task = taskQueue.front();
        taskQueue.pop();
        
        auto* t = registry.GetComponent<TransformComponent>(task.entity);
        if (t) t->position = task.newPosition;
    }
}
```

---

## Core API

### Entity Component System

#### Registry

Центральное хранилище всех entity и компонентов.

```cpp
#include <SAGE/ECS/Registry.h>

namespace SAGE::ECS {
    class Registry;
}
```

**Методы:**

```cpp
// Создание и удаление entity
Entity CreateEntity();
void DestroyEntity(Entity entity);
bool IsValid(Entity entity) const;

// Управление компонентами
template<typename T>
void AddComponent(Entity entity, T component);

template<typename T>
void RemoveComponent(Entity entity);

template<typename T>
T* GetComponent(Entity entity);

template<typename T>
const T* GetComponent(Entity entity) const;

template<typename T>
bool HasComponent(Entity entity) const;

// Итерация по компонентам
template<typename... Components>
void ForEach(std::function<void(Entity, Components&...)> func);

// Получение всех entity с определёнными компонентами
template<typename... Components>
std::vector<Entity> GetEntitiesWith();

// Очистка
void Clear();
size_t GetEntityCount() const;
```

**Пример использования:**

```cpp
using namespace SAGE::ECS;

Registry registry;

// Создание entity
Entity player = registry.CreateEntity();

// Добавление компонентов
TransformComponent transform;
transform.position = Vector2(100, 200);
registry.AddComponent(player, transform);

SpriteComponent sprite;
sprite.texturePath = "player.png";
registry.AddComponent(player, sprite);

// Получение компонента
auto* trans = registry.GetComponent<TransformComponent>(player);
if (trans) {
    trans->position.x += 10;
}

// Итерация
registry.ForEach<TransformComponent, SpriteComponent>(
    [](Entity e, TransformComponent& t, SpriteComponent& s) {
        // Обработка каждого entity с этими компонентами
    }
);

// Удаление
registry.DestroyEntity(player);
```

#### Entity

Уникальный идентификатор игрового объекта.

```cpp
using Entity = uint64_t;
constexpr Entity NullEntity = std::numeric_limits<Entity>::max();
```

**Вспомогательные функции:**

```cpp
uint32_t GetEntityID(Entity entity);      // Извлечь ID
uint32_t GetEntityVersion(Entity entity); // Извлечь версию
Entity MakeEntity(uint32_t id, uint32_t version);
bool IsValid(Entity entity);
```

#### System

Базовый класс для систем обработки.

```cpp
class System {
public:
    virtual void Init() {}
    virtual void Update(Registry& registry, float deltaTime) {}
    virtual void FixedUpdate(Registry& registry, float fixedDeltaTime) {}
    virtual void Shutdown() {}
    
    void SetActive(bool active);
    bool IsActive() const;
    
    void SetPriority(int priority);
    int GetPriority() const;
};
```

**Встроенные системы:**

- **PhysicsSystem** - физическая симуляция
- **AnimationSystem** - анимация спрайтов
- **RenderSystem** - рендеринг

### Компоненты

#### TransformComponent

Позиция, поворот и размер объекта.

```cpp
struct TransformComponent {
    Vector2 position;    // Мировая позиция (центр объекта)
    Vector2 scale;       // Множитель размера
    Vector2 size;        // Базовый размер
    Vector2 pivot;       // Точка вращения (0.5, 0.5 = центр)
    float rotation;      // Угол в градусах
    
    TransformComponent();
    TransformComponent(float x, float y, float rot = 0.0f);
    
    Vector2 GetWorldPosition() const;
    Vector2 GetRenderPosition() const;
    Matrix4 GetTransformMatrix() const;
};
```

#### SpriteComponent

Визуальное представление объекта.

```cpp
struct SpriteComponent {
    std::string texturePath;
    Ref<Texture> texture;
    Ref<Material> material;
    
    Color tint;          // Цвет тинта (включает alpha)
    bool visible;
    bool flipX;
    bool flipY;
    
    int layer;           // Слой рендеринга
    
    Float2 uvMin;        // UV координаты (для sprite sheet)
    Float2 uvMax;
    Float2 pivot;
    
    SpriteComponent();
    explicit SpriteComponent(const std::string& path);
    
    void SetUVRegion(float texW, float texH, float x, float y, float w, float h);
};
```

#### PhysicsComponent

Физические свойства объекта.

```cpp
enum class PhysicsBodyType {
    Static,    // Не двигается (стены, платформы)
    Dynamic,   // Полная симуляция
    Kinematic  // Управляется скриптом, но влияет на другие тела
};

struct PhysicsComponent {
    PhysicsBodyType type;
    
    float mass;
    float inverseMass;
    float linearDamping;
    float angularDamping;
    float gravityScale;
    
    float restitution;      // Упругость (0 = не отскакивает, 1 = полностью)
    float staticFriction;
    float dynamicFriction;
    
    Vector2 velocity;
    Vector2 force;
    float angularVelocity;
    float torque;
    
    bool fixedRotation;
    bool isBullet;          // Continuous collision detection
    bool isAwake;
    bool isSensor;          // Триггер без физического столкновения
    
    void SetType(PhysicsBodyType newType);
    void SetMass(float m);
    void UpdateInverseMass();
    void ApplyForce(const Vector2& f);
    void ApplyImpulse(const Vector2& impulse);
};
```

#### ColliderComponent

Форма для столкновений.

```cpp
enum class ColliderShape {
    Box,
    Circle,
    Polygon
};

struct ColliderComponent {
    ColliderShape shape;
    
    Vector2 offset;      // Смещение относительно Transform
    
    // Box
    Vector2 boxSize;
    
    // Circle
    float radius;
    
    // Polygon
    std::vector<Vector2> vertices;
    
    float density;
    
    static ColliderComponent CreateBox(const Vector2& size);
    static ColliderComponent CreateCircle(float r);
    static ColliderComponent CreatePolygon(const std::vector<Vector2>& verts);
    
    void SetBox(float width, float height);
    void SetCircle(float r);
};
```

#### AnimationComponent

Анимация спрайта.

```cpp
struct AnimationComponent {
    Ref<AnimationClip> clip;
    
    size_t currentFrame;
    float timeAccumulator;
    bool isPlaying;
    bool isLooping;
    float playbackSpeed;
    
    AnimationComponent();
    
    void SetClip(Ref<AnimationClip> newClip);
    void Play();
    void Pause();
    void Stop();
    void Reset();
    bool IsPlaying() const;
};
```

### AnimationClip

Последовательность кадров анимации.

```cpp
struct AnimationFrame {
    Float2 uvMin;
    Float2 uvMax;
    float duration;      // Длительность кадра в секундах
};

class AnimationClip {
public:
    AnimationClip(const std::string& name);
    
    void AddFrame(const Float2& uvMin, const Float2& uvMax, float duration);
    void ClearFrames();
    
    const AnimationFrame& GetFrame(size_t index) const;
    size_t GetFrameCount() const;
    
    void SetLooping(bool loop);
    bool IsLooping() const;
    
    float GetTotalDuration() const;
    
    const std::string& GetName() const;
};
```

## Graphics API

### RenderContext

Главный интерфейс рендеринга (Singleton).

```cpp
class RenderContext {
public:
    static RenderContext& Instance();
    
    bool Initialize(int width, int height);
    void Shutdown();
    
    void Clear(const Color& color);
    void SetViewport(int x, int y, int width, int height);
    
    void BeginFrame();
    void EndFrame();
    
    void SetCamera(const Camera2D& camera);
    Camera2D& GetCamera();
};
```

### Camera2D

Камера для 2D сцены.

```cpp
enum class ProjectionMode {
    Orthographic,
    Perspective
};

class Camera2D {
public:
    Camera2D();
    Camera2D(float width, float height);
    
    void SetPosition(const Vector2& pos);
    Vector2 GetPosition() const;
    
    void SetZoom(float z);
    float GetZoom() const;
    
    void SetRotation(float angle);
    float GetRotation() const;
    
    void SetViewportSize(float width, float height);
    
    Matrix4 GetViewMatrix() const;
    Matrix4 GetProjectionMatrix() const;
    Matrix4 GetViewProjectionMatrix() const;
    
    Vector2 ScreenToWorld(const Vector2& screenPos) const;
    Vector2 WorldToScreen(const Vector2& worldPos) const;
};
```

### Texture

GPU текстура.

```cpp
class Texture : public IResource {
public:
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetHandle() const;  // OpenGL texture ID
    
    TextureFormat GetFormat() const;
    TextureFilter GetFilter() const;
    TextureWrap GetWrap() const;
    
    void Bind(uint32_t slot = 0) const;
    void Unbind() const;
    
    size_t GetGpuMemoryUsage() const override;
};
```

### Shader

Шейдерная программа.

```cpp
class Shader {
public:
    bool LoadFromFiles(const std::string& vertPath, const std::string& fragPath);
    bool LoadFromSource(const std::string& vertSrc, const std::string& fragSrc);
    
    void Bind() const;
    void Unbind() const;
    
    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, const Vector2& value);
    void SetUniform(const std::string& name, const Vector3& value);
    void SetUniform(const std::string& name, const Vector4& value);
    void SetUniform(const std::string& name, const Matrix4& value);
    
    uint32_t GetProgram() const;
};
```

## Physics API

### PhysicsSystem

Система физической симуляции.

```cpp
class PhysicsSystem : public System {
public:
    PhysicsSystem();
    explicit PhysicsSystem(std::unique_ptr<IPhysicsBackend> backend);
    
    void Init() override;
    void FixedUpdate(Registry& registry, float fixedDeltaTime) override;
    void Shutdown() override;
    
    void SetGravity(const Vector2& gravity);
    Vector2 GetGravity() const;
    
    void SetPhysicsSettings(const Physics::PhysicsSettings& settings);
    
    // Raycast
    struct RaycastHit {
        Entity entity;
        Vector2 point;
        Vector2 normal;
        float distance;
    };
    
    bool Raycast(const Vector2& origin, const Vector2& direction, 
                 float maxDistance, RaycastHit& hit);
    
    std::vector<RaycastHit> RaycastAll(const Vector2& origin, 
                                       const Vector2& direction,
                                       float maxDistance);
    
    // AABB Query
    std::vector<Entity> QueryAABB(const Vector2& min, const Vector2& max);
};
```

### Physics::PhysicsSettings

Настройки физики.

```cpp
namespace Physics {
    struct PhysicsSettings {
        Vector2 gravity;              // м/с² (обычно 0, 980 для пикселей)
        int velocityIterations;       // Точность (обычно 8)
        int positionIterations;       // Точность (обычно 3)
        bool enableSleeping;          // Усыплять неактивные тела
        float timeStep;               // Фиксированный шаг (обычно 1/60)
    };
}
```

## Audio API

### AudioSystem

Система воспроизведения звука.

```cpp
class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();
    
    bool Init();
    void Shutdown();
    void Update(float deltaTime);
    
    // SFX (короткие звуки)
    bool LoadSFX(const std::string& name, const std::string& filepath,
                 uint32_t voices = 0, bool streaming = false);
    void PlaySFX(const std::string& name, float volume = 1.0f, 
                 float pitch = 1.0f, float pan = 0.0f);
    void StopSFX(const std::string& name);
    void StopAllSFX();
    
    // BGM (фоновая музыка)
    bool LoadBGM(const std::string& name, const std::string& filepath);
    void PlayBGM(const std::string& name, float volume = 0.7f, 
                 float fadeInDuration = 0.0f);
    void StopBGM(float fadeOutDuration = 0.0f);
    void PauseBGM();
    void ResumeBGM();
    bool IsBGMPlaying() const;
    
    // Громкость
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void SetSFXVolume(float volume);
    float GetSFXVolume() const;
    void SetBGMVolume(float volume);
    float GetBGMVolume() const;
    
    // 3D позиционирование
    void SetListenerPosition(float x, float y, float z);
    void SetListenerVelocity(float x, float y, float z);
    void SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                float upX, float upY, float upZ);
};
```

## Resource Management API

### ResourceManager

Менеджер ресурсов (Singleton).

```cpp
class ResourceManager {
public:
    static ResourceManager& Get();
    
    // Загрузка ресурсов
    Ref<Texture> LoadTexture(const std::string& path);
    Ref<Font> LoadFont(const std::string& path, int size);
    Ref<Shader> LoadShader(const std::string& vertPath, 
                           const std::string& fragPath);
    
    // Управление кэшем
    void ClearCache();
    void UnloadResource(const std::string& path);
    bool IsResourceLoaded(const std::string& path) const;
    
    // GPU память
    void SetGpuLoadingEnabled(bool enabled);
    bool IsGpuLoadingEnabled() const;
    
    size_t GetGpuMemoryUsage() const;
    size_t GetGpuMemoryBudget() const;
    
    // Статистика
    size_t GetCacheSize() const;
    float GetCacheHitRate() const;
};
```

## Event System API

### EventBus

Система событий (Singleton).

```cpp
class EventBus {
public:
    static EventBus& Get();
    
    // Подписка
    template<typename EventType>
    uint64_t Subscribe(std::function<void(const EventType&)> handler,
                      uint32_t group = 0,
                      int priority = 0);
    
    template<typename EventType>
    uint64_t SubscribeScoped(std::function<void(const EventType&)> handler,
                            uint32_t group = 0);
    
    // Отписка
    template<typename EventType>
    void Unsubscribe(uint64_t handlerId);
    
    void UnsubscribeGroup(uint32_t group);
    
    // Публикация
    template<typename EventType>
    void Publish(const EventType& event);
    
    template<typename EventType>
    void Enqueue(const EventType& event, EventPriority priority = EventPriority::Normal);
    
    void Flush();
    
    // Фильтрация
    void EnableCategory(EventCategory category);
    void DisableCategory(EventCategory category);
    bool IsCategoryEnabled(EventCategory category) const;
};
```

**Пример:**

```cpp
// Определение события
struct PlayerDiedEvent {
    Entity player;
    Vector2 position;
};

// Подписка
auto& bus = EventBus::Get();
uint64_t handlerId = bus.Subscribe<PlayerDiedEvent>(
    [](const PlayerDiedEvent& e) {
        // Обработка
    }
);

// Публикация
PlayerDiedEvent event;
event.player = playerEntity;
event.position = Vector2(100, 200);
bus.Publish(event);

// Отписка
bus.Unsubscribe<PlayerDiedEvent>(handlerId);
```

## Input API

### InputManager

Управление вводом (Singleton).

```cpp
class InputManager {
public:
    static InputManager& Get();
    
    bool Initialize(IWindow* window);
    void Update();
    void Shutdown();
    
    // Клавиатура
    bool IsKeyDown(int keyCode) const;
    bool IsKeyPressed(int keyCode) const;  // В этом кадре нажата
    bool IsKeyReleased(int keyCode) const; // В этом кадре отпущена
    
    // Мышь
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;
    
    Vector2 GetMousePosition() const;
    Vector2 GetMouseDelta() const;
    float GetMouseWheelDelta() const;
    
    // Геймпад
    bool IsGamepadConnected(int gamepadId) const;
    bool IsGamepadButtonDown(int gamepadId, int button) const;
    float GetGamepadAxis(int gamepadId, int axis) const;
    
    // Action mapping
    void MapAction(const std::string& actionName, int keyCode);
    bool IsActionActive(const std::string& actionName) const;
    bool IsActionTriggered(const std::string& actionName) const;
};
```

**Коды клавиш:**

```cpp
// Используйте GLFW константы
GLFW_KEY_SPACE
GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D
GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT
GLFW_KEY_ESCAPE
// и т.д.
```

## Math API

### Vector2

2D вектор.

```cpp
struct Vector2 {
    float x, y;
    
    Vector2();
    Vector2(float x, float y);
    
    static Vector2 Zero();
    static Vector2 One();
    static Vector2 UnitX();
    static Vector2 UnitY();
    
    float Length() const;
    float LengthSquared() const;
    Vector2 Normalized() const;
    void Normalize();
    
    float Dot(const Vector2& other) const;
    float Cross(const Vector2& other) const;
    
    float Distance(const Vector2& other) const;
    float Angle(const Vector2& other) const;
    
    Vector2 Rotate(float angleDegrees) const;
    Vector2 Perpendicular() const;
    
    // Операторы
    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;
};
```

### Matrix4

4x4 матрица (column-major для OpenGL).

```cpp
class Matrix4 {
public:
    Matrix4();
    static Matrix4 Identity();
    
    static Matrix4 Translate(const Vector2& translation);
    static Matrix4 Rotate(float angleDegrees);
    static Matrix4 Scale(const Vector2& scale);
    
    static Matrix4 Ortho(float left, float right, float bottom, float top);
    static Matrix4 Perspective(float fov, float aspect, float near, float far);
    
    Matrix4 operator*(const Matrix4& other) const;
    Vector2 Apply(const Vector2& point) const;
    
    Matrix4 Inverse() const;
    Matrix4 Transpose() const;
    
    const float* Data() const;  // Для передачи в OpenGL
};
```

### Random

Генератор случайных чисел.

```cpp
namespace Math {
    class Random {
    public:
        static Random& Global();
        
        void SetSeed(uint32_t seed);
        uint32_t GetSeed() const;
        
        int NextInt(int min, int max);              // [min, max]
        float NextRange(float min, float max);      // [min, max)
        float NextFloat();                          // [0, 1)
        bool NextBool();
        
        Vector2 NextUnitVector();
        Vector2 NextInCircle(float radius);
        
        template<typename T>
        const T& Choose(const std::vector<T>& items);
    };
}
```

## Utility API

### Color

RGBA цвет.

```cpp
struct Color {
    uint8_t r, g, b, a;
    
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    
    static Color White();
    static Color Black();
    static Color Red();
    static Color Green();
    static Color Blue();
    static Color Transparent();
    
    static Color FromHex(uint32_t hex);
    uint32_t ToHex() const;
    
    Vector4 ToFloat4() const;  // Для шейдеров
};
```

### Logger

Система логирования.

```cpp
// Макросы для логирования
SAGE_TRACE(category, format, ...);
SAGE_INFO(category, format, ...);
SAGE_WARNING(category, format, ...);
SAGE_ERROR(category, format, ...);

// Пример
SAGE_INFO("Game", "Player spawned at ({}, {})", pos.x, pos.y);
SAGE_ERROR("Physics", "Invalid body type");
```

**Категории:**
- "core" - ядро движка
- "physics" - физика
- "graphics" - рендеринг
- "audio" - аудио
- "game" - игровая логика

## Application Framework

### Application

Базовый класс для игры.

```cpp
class Application {
public:
    Application(const std::string& title, int width, int height);
    virtual ~Application();
    
    int Run();
    void Quit();
    
protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}
    
    float GetDeltaTime() const;
    float GetFPS() const;
    
    bool IsRunning() const;
};
```

**Пример:**

```cpp
class MyGame : public Application {
public:
    MyGame() : Application("My Game", 1280, 720) {}
    
protected:
    void OnInit() override {
        // Инициализация
        player = registry.CreateEntity();
        // ...
    }
    
    void OnUpdate(float deltaTime) override {
        // Обновление логики
        physicsSystem.FixedUpdate(registry, deltaTime);
    }
    
    void OnRender() override {
        // Рендеринг
        renderSystem.Render(registry);
    }
    
private:
    Registry registry;
    PhysicsSystem physicsSystem;
    Entity player;
};

int main() {
    MyGame game;
    return game.Run();
}
```
