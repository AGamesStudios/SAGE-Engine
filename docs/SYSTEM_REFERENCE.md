# SAGE Engine - Справочник систем

**Версия:** Alpha  
**Namespace:** SAGE  
**Базовый класс:** SAGE::ECS::System

---

## Содержание

- [Обзор](#обзор)
- [Базовый класс System](#базовый-класс-system)
- [PhysicsSystem](#physicssystem)
- [AnimationSystem](#animationsystem)
- [RenderSystem](#rendersystem)
- [AudioSystem](#audiosystem)
- [InputManager](#inputmanager)
- [ResourceManager](#resourcemanager)
- [EventBus](#eventbus)

---

## Обзор

Системы в SAGE Engine обрабатывают компоненты и реализуют игровую логику. Каждая система наследуется от базового класса `System` и имеет методы жизненного цикла.

## Базовый класс System

### Определение

```cpp
namespace SAGE::ECS {
    class System {
    public:
        virtual void Init() {}
        virtual void Update(Registry& registry, float deltaTime) {}
        virtual void FixedUpdate(Registry& registry, float fixedDeltaTime) {}
        virtual void Render(Registry& registry) {}
        virtual void Shutdown() {}
        
        void SetActive(bool active);
        bool IsActive() const;
        
        void SetPriority(int priority);
        int GetPriority() const;
        
    protected:
        bool m_active = true;
        int m_priority = 0;
    };
}
```

### Методы жизненного цикла

**Init()**
- Вызывается один раз при создании системы
- Используется для инициализации ресурсов, настройки
- Пример: загрузка шейдеров, создание буферов

**Update(Registry& registry, float deltaTime)**
- Вызывается каждый кадр
- `deltaTime` - время с предыдущего кадра в секундах
- Используется для логики, анимаций, AI

**FixedUpdate(Registry& registry, float fixedDeltaTime)**
- Вызывается с фиксированным шагом (обычно 60 раз в секунду)
- `fixedDeltaTime` - фиксированный временной шаг (обычно 1/60)
- Используется для физики, детерминированной логики

**Render(Registry& registry)**
- Вызывается для рендеринга
- Не принимает deltaTime
- Используется для отрисовки спрайтов, эффектов

**Shutdown()**
- Вызывается при уничтожении системы
- Используется для освобождения ресурсов
- Пример: выгрузка текстур, закрытие файлов

### Управление системой

```cpp
// Активация/деактивация
system.SetActive(false);  // Остановить обновление
bool active = system.IsActive();

// Приоритет (меньше = раньше выполняется)
system.SetPriority(10);
int priority = system.GetPriority();
```

## PhysicsSystem

### Описание

Система физической симуляции, управляющая физическими телами и коллайдерами.

### Определение

```cpp
namespace SAGE::Physics {
    class PhysicsSystem : public System {
    public:
        PhysicsSystem();
        explicit PhysicsSystem(std::unique_ptr<IPhysicsBackend> backend);
        ~PhysicsSystem();
        
        void Init() override;
        void FixedUpdate(Registry& registry, float fixedDeltaTime) override;
        void Shutdown() override;
        
        // Настройки
        void SetGravity(const Vector2& gravity);
        Vector2 GetGravity() const;
        void SetPhysicsSettings(const PhysicsSettings& settings);
        PhysicsSettings GetPhysicsSettings() const;
        
        // Raycast
        bool Raycast(const Vector2& origin, const Vector2& direction,
                     float maxDistance, RaycastHit& hit);
        std::vector<RaycastHit> RaycastAll(const Vector2& origin,
                                           const Vector2& direction,
                                           float maxDistance);
        
        // Queries
        std::vector<Entity> QueryAABB(const Vector2& min, const Vector2& max);
        std::vector<Entity> QueryCircle(const Vector2& center, float radius);
        std::vector<Entity> QueryPoint(const Vector2& point);
        
        // Backend
        IPhysicsBackend* GetBackend() const;
        
    private:
        std::unique_ptr<IPhysicsBackend> m_backend;
        PhysicsSettings m_settings;
    };
}
```

### Структуры данных

**PhysicsSettings**

```cpp
struct PhysicsSettings {
    Vector2 gravity;              // Гравитация (пиксели/с²)
    int velocityIterations;       // Итерации скорости (8 по умолчанию)
    int positionIterations;       // Итерации позиции (3 по умолчанию)
    bool enableSleeping;          // Усыплять неактивные тела
    float timeStep;               // Фиксированный шаг (1/60)
    float linearSleepThreshold;   // Порог для усыпления
    float angularSleepThreshold;
    float timeToSleep;            // Время до усыпления
};
```

**RaycastHit**

```cpp
struct RaycastHit {
    Entity entity;       // Попавший entity
    Vector2 point;       // Точка попадания
    Vector2 normal;      // Нормаль поверхности
    float distance;      // Дистанция от origin
    float fraction;      // Доля от maxDistance (0-1)
};
```

### Использование

```cpp
PhysicsSystem physics;
physics.Init();

// Настройка гравитации
physics.SetGravity(Vector2(0, 980));  // 980 пикселей/с² вниз

// Настройка параметров
PhysicsSettings settings;
settings.gravity = Vector2(0, 980);
settings.velocityIterations = 8;
settings.positionIterations = 3;
settings.enableSleeping = true;
physics.SetPhysicsSettings(settings);

// Обновление (в игровом цикле)
physics.FixedUpdate(registry, 1.0f / 60.0f);

// Raycast
PhysicsSystem::RaycastHit hit;
if (physics.Raycast(Vector2(0, 0), Vector2(1, 0), 1000.0f, hit)) {
    SAGE_INFO("Physics", "Hit entity at ({}, {})", hit.point.x, hit.point.y);
}

// Все попадания
auto hits = physics.RaycastAll(origin, direction, 1000.0f);
for (const auto& hit : hits) {
    // Обработка
}

// AABB Query
std::vector<Entity> entities = physics.QueryAABB(
    Vector2(0, 0),      // min
    Vector2(100, 100)   // max
);

// Circle Query
entities = physics.QueryCircle(Vector2(50, 50), 25.0f);

// Point Query
entities = physics.QueryPoint(Vector2(75, 75));
```

### Примечания

- FixedUpdate должен вызываться с постоянным deltaTime
- Система автоматически синхронизирует Transform и Physics
- Требует PhysicsComponent и ColliderComponent на entity

---

## AnimationSystem

### Описание

Система обновления анимаций спрайтов.

### Определение

```cpp
namespace SAGE::Animation {
    class AnimationSystem : public System {
    public:
        AnimationSystem();
        
        void Update(Registry& registry, float deltaTime) override;
        
        // Глобальное управление
        void SetGlobalSpeed(float speed);
        float GetGlobalSpeed() const;
        
        void PauseAll();
        void ResumeAll();
        
    private:
        float m_globalSpeed = 1.0f;
        bool m_paused = false;
    };
}
```

### Использование

```cpp
AnimationSystem animSystem;

// Обновление (в игровом цикле)
animSystem.Update(registry, deltaTime);

// Глобальное управление скоростью
animSystem.SetGlobalSpeed(2.0f);  // 2x быстрее
animSystem.SetGlobalSpeed(0.5f);  // 2x медленнее

// Пауза всех анимаций
animSystem.PauseAll();
animSystem.ResumeAll();
```

### Работа с AnimationClip

```cpp
// Создание клипа
auto clip = std::make_shared<AnimationClip>("walk");

// Настройка
clip->SetLooping(true);
clip->SetFrameRate(10.0f);  // 10 FPS

// Добавление кадров
for (int i = 0; i < 8; i++) {
    float u = (i % 4) * 0.25f;
    float v = (i / 4) * 0.5f;
    
    clip->AddFrame(
        Float2(u, v),
        Float2(u + 0.25f, v + 0.5f),
        0.1f  // duration
    );
}

// Или из sprite sheet
clip->AddFramesFromSpriteSheet(
    4, 2,      // columns, rows
    0.1f       // duration per frame
);

// Методы клипа
size_t frameCount = clip->GetFrameCount();
float totalDuration = clip->GetTotalDuration();
const AnimationFrame& frame = clip->GetFrame(0);
clip->ClearFrames();
```

### Примечания

- Автоматически обновляет UV координаты SpriteComponent
- Требует AnimationComponent и SpriteComponent
- Поддерживает зацикливание и изменение скорости

---

## RenderSystem

### Описание

Система рендеринга спрайтов и графических элементов.

### Определение

```cpp
namespace SAGE::Graphics {
    class RenderSystem : public System {
    public:
        RenderSystem();
        ~RenderSystem();
        
        void Init() override;
        void Render(Registry& registry) override;
        void Shutdown() override;
        
        // Настройки рендеринга
        void SetCamera(const Camera2D& camera);
        Camera2D& GetCamera();
        
        void EnableFrustumCulling(bool enable);
        bool IsFrustumCullingEnabled() const;
        
        void SetClearColor(const Color& color);
        Color GetClearColor() const;
        
        // Статистика
        struct RenderStats {
            uint32_t drawCalls;
            uint32_t spritesRendered;
            uint32_t spritesCulled;
            uint32_t batchCount;
            float renderTime;
        };
        
        RenderStats GetStats() const;
        void ResetStats();
        
    private:
        std::unique_ptr<BatchRenderer> m_batchRenderer;
        Camera2D m_camera;
        Color m_clearColor;
        bool m_frustumCulling = true;
        RenderStats m_stats;
    };
}
```

### Использование

```cpp
RenderSystem renderSystem;
renderSystem.Init();

// Настройка камеры
Camera2D camera(1280, 720);
camera.SetPosition(Vector2(0, 0));
camera.SetZoom(1.0f);
renderSystem.SetCamera(camera);

// Цвет фона
renderSystem.SetClearColor(Color(30, 30, 40, 255));

// Frustum culling
renderSystem.EnableFrustumCulling(true);

// Рендеринг (в игровом цикле)
renderSystem.Render(registry);

// Статистика
auto stats = renderSystem.GetStats();
SAGE_INFO("Render", "Draw calls: {}, Sprites: {}", 
          stats.drawCalls, stats.spritesRendered);
renderSystem.ResetStats();
```

### Оптимизация рендеринга

**Батчинг:**
- Спрайты с одинаковой текстурой батчатся автоматически
- Сортировка по texture ID минимизирует state changes

**Frustum Culling:**
- Спрайты вне экрана не рендерятся
- Проверка по AABB

**Слои:**
- Рендеринг по возрастанию layer
- Спрайты с одинаковым layer сортируются по текстуре

---

## AudioSystem

### Описание

Система воспроизведения звука и музыки.

### Определение

```cpp
namespace SAGE::Audio {
    class AudioSystem {
    public:
        AudioSystem();
        ~AudioSystem();
        
        bool Init();
        void Shutdown();
        void Update(float deltaTime);
        
        // SFX
        bool LoadSFX(const std::string& name, const std::string& filepath,
                     uint32_t voices = 0, bool streaming = false);
        void UnloadSFX(const std::string& name);
        
        void PlaySFX(const std::string& name, float volume = 1.0f,
                     float pitch = 1.0f, float pan = 0.0f);
        void StopSFX(const std::string& name);
        void StopAllSFX();
        
        bool IsSFXPlaying(const std::string& name) const;
        void SetSFXLooping(const std::string& name, bool looping);
        
        // BGM
        bool LoadBGM(const std::string& name, const std::string& filepath);
        void UnloadBGM(const std::string& name);
        
        void PlayBGM(const std::string& name, float volume = 0.7f,
                     float fadeInDuration = 0.0f);
        void StopBGM(float fadeOutDuration = 0.0f);
        void PauseBGM();
        void ResumeBGM();
        
        bool IsBGMPlaying() const;
        bool IsBGMPaused() const;
        std::string GetCurrentBGM() const;
        
        void SetBGMLooping(bool looping);
        void CrossfadeToBGM(const std::string& name, float duration,
                           float volume = 0.7f);
        
        // Громкость
        void SetMasterVolume(float volume);
        float GetMasterVolume() const;
        
        void SetSFXVolume(float volume);
        float GetSFXVolume() const;
        
        void SetBGMVolume(float volume);
        float GetBGMVolume() const;
        
        void SetCategoryVolume(const std::string& category, float volume);
        float GetCategoryVolume(const std::string& category) const;
        
        // 3D позиционирование
        void SetListenerPosition(float x, float y, float z = 0.0f);
        void SetListenerVelocity(float x, float y, float z = 0.0f);
        void SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                   float upX, float upY, float upZ);
        
        void PlaySFX3D(const std::string& name, const Vector2& position,
                      float volume = 1.0f, float pitch = 1.0f);
        
        // Настройки
        void SetDopplerFactor(float factor);
        void SetSpeedOfSound(float speed);
        void SetMaxDistance(float distance);
        void SetRolloffFactor(float factor);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

### Использование

```cpp
AudioSystem audio;
if (!audio.Init()) {
    SAGE_ERROR("Audio", "Failed to initialize");
    return;
}

// Загрузка SFX
audio.LoadSFX("jump", "assets/sounds/jump.wav", 3);  // 3 голоса
audio.LoadSFX("shoot", "assets/sounds/shoot.wav", 5);
audio.LoadSFX("explosion", "assets/sounds/explosion.wav", 2);

// Воспроизведение SFX
audio.PlaySFX("jump", 1.0f);           // Громкость 100%
audio.PlaySFX("shoot", 0.8f, 1.2f);    // Громкость 80%, pitch 1.2x
audio.PlaySFX("explosion", 1.0f, 1.0f, -0.5f);  // pan влево

// Загрузка BGM
audio.LoadBGM("menu", "assets/music/menu.mp3");
audio.LoadBGM("battle", "assets/music/battle.mp3");

// Воспроизведение BGM
audio.PlayBGM("menu", 0.7f, 2.0f);  // Громкость 70%, fade in 2 сек

// Переключение музыки
audio.CrossfadeToBGM("battle", 3.0f, 0.8f);  // Crossfade 3 сек

// Управление громкостью
audio.SetMasterVolume(0.8f);
audio.SetSFXVolume(1.0f);
audio.SetBGMVolume(0.6f);
audio.SetCategoryVolume("ui", 0.9f);

// 3D звук
audio.SetListenerPosition(playerX, playerY, 0);
audio.PlaySFX3D("explosion", enemyPos, 1.0f);

// Обновление (в игровом цикле)
audio.Update(deltaTime);

// Очистка
audio.Shutdown();
```

### Категории звуков

```cpp
// Загрузка с категорией
audio.LoadSFX("button_click", "click.wav", 1, false);
audio.SetCategoryVolume("ui", 0.9f);

audio.LoadSFX("footstep", "step.wav", 2, false);
audio.SetCategoryVolume("player", 1.0f);
```

---

## InputManager

### Описание

Singleton для управления вводом с клавиатуры, мыши и геймпада.

### Определение

```cpp
namespace SAGE::Input {
    class InputManager {
    public:
        static InputManager& Get();
        
        bool Initialize(IWindow* window);
        void Update();
        void Shutdown();
        
        // Клавиатура
        bool IsKeyDown(int keyCode) const;
        bool IsKeyPressed(int keyCode) const;
        bool IsKeyReleased(int keyCode) const;
        bool IsKeyUp(int keyCode) const;
        
        // Мышь
        bool IsMouseButtonDown(int button) const;
        bool IsMouseButtonPressed(int button) const;
        bool IsMouseButtonReleased(int button) const;
        bool IsMouseButtonUp(int button) const;
        
        Vector2 GetMousePosition() const;
        Vector2 GetMouseDelta() const;
        Vector2 GetMouseWorldPosition(const Camera2D& camera) const;
        
        float GetMouseWheelDelta() const;
        
        void SetCursorMode(CursorMode mode);
        CursorMode GetCursorMode() const;
        
        // Геймпад
        bool IsGamepadConnected(int gamepadId) const;
        int GetConnectedGamepadCount() const;
        
        bool IsGamepadButtonDown(int gamepadId, int button) const;
        bool IsGamepadButtonPressed(int gamepadId, int button) const;
        bool IsGamepadButtonReleased(int gamepadId, int button) const;
        
        float GetGamepadAxis(int gamepadId, int axis) const;
        Vector2 GetGamepadLeftStick(int gamepadId) const;
        Vector2 GetGamepadRightStick(int gamepadId) const;
        
        float GetGamepadLeftTrigger(int gamepadId) const;
        float GetGamepadRightTrigger(int gamepadId) const;
        
        void SetGamepadDeadzone(float deadzone);
        float GetGamepadDeadzone() const;
        
        void SetGamepadVibration(int gamepadId, float leftMotor, float rightMotor);
        
        // Action Mapping
        void MapAction(const std::string& actionName, int keyCode);
        void MapAction(const std::string& actionName, int gamepadId, int button);
        void UnmapAction(const std::string& actionName);
        
        bool IsActionActive(const std::string& actionName) const;
        bool IsActionTriggered(const std::string& actionName) const;
        bool IsActionReleased(const std::string& actionName) const;
        
        // Axis Mapping
        void MapAxis(const std::string& axisName, int positiveKey, int negativeKey);
        void MapAxis(const std::string& axisName, int gamepadId, int axis);
        float GetAxis(const std::string& axisName) const;
        
        // Текстовый ввод
        void StartTextInput();
        void StopTextInput();
        bool IsTextInputActive() const;
        std::string GetInputText() const;
        void ClearInputText();
        
    private:
        InputManager() = default;
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

### Константы

**Клавиши (GLFW):**

```cpp
// Буквы
GLFW_KEY_A, GLFW_KEY_B, ..., GLFW_KEY_Z

// Цифры
GLFW_KEY_0, GLFW_KEY_1, ..., GLFW_KEY_9

// Стрелки
GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT

// Функциональные
GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_ESCAPE
GLFW_KEY_TAB, GLFW_KEY_BACKSPACE, GLFW_KEY_DELETE

// Модификаторы
GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT
GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL
GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT

// F-клавиши
GLFW_KEY_F1, GLFW_KEY_F2, ..., GLFW_KEY_F12
```

**Кнопки мыши:**

```cpp
GLFW_MOUSE_BUTTON_LEFT    // 0
GLFW_MOUSE_BUTTON_RIGHT   // 1
GLFW_MOUSE_BUTTON_MIDDLE  // 2
```

**Кнопки геймпада:**

```cpp
GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B
GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_Y
GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
GLFW_GAMEPAD_BUTTON_START, GLFW_GAMEPAD_BUTTON_BACK
GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN
GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
```

**Оси геймпада:**

```cpp
GLFW_GAMEPAD_AXIS_LEFT_X, GLFW_GAMEPAD_AXIS_LEFT_Y
GLFW_GAMEPAD_AXIS_RIGHT_X, GLFW_GAMEPAD_AXIS_RIGHT_Y
GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
```

### Использование

```cpp
auto& input = InputManager::Get();

// Инициализация
input.Initialize(window);

// Клавиатура
if (input.IsKeyDown(GLFW_KEY_W)) {
    // Движение вперёд (каждый кадр)
}

if (input.IsKeyPressed(GLFW_KEY_SPACE)) {
    // Прыжок (только момент нажатия)
    Jump();
}

// Мышь
Vector2 mousePos = input.GetMousePosition();
Vector2 mouseDelta = input.GetMouseDelta();

if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    Vector2 worldPos = input.GetMouseWorldPosition(camera);
    Shoot(worldPos);
}

float wheel = input.GetMouseWheelDelta();
camera.SetZoom(camera.GetZoom() + wheel * 0.1f);

// Геймпад
if (input.IsGamepadConnected(0)) {
    Vector2 leftStick = input.GetGamepadLeftStick(0);
    player.Move(leftStick);
    
    if (input.IsGamepadButtonPressed(0, GLFW_GAMEPAD_BUTTON_A)) {
        Jump();
    }
    
    float rightTrigger = input.GetGamepadRightTrigger(0);
    if (rightTrigger > 0.1f) {
        Shoot(rightTrigger);
    }
    
    // Вибрация
    input.SetGamepadVibration(0, 0.5f, 0.5f);
}

// Action Mapping
input.MapAction("jump", GLFW_KEY_SPACE);
input.MapAction("jump", 0, GLFW_GAMEPAD_BUTTON_A);
input.MapAction("shoot", GLFW_MOUSE_BUTTON_LEFT);

if (input.IsActionTriggered("jump")) {
    Jump();
}

// Axis Mapping
input.MapAxis("horizontal", GLFW_KEY_D, GLFW_KEY_A);
input.MapAxis("vertical", GLFW_KEY_W, GLFW_KEY_S);

float h = input.GetAxis("horizontal");  // -1 до 1
float v = input.GetAxis("vertical");
player.Move(Vector2(h, v));

// Текстовый ввод
input.StartTextInput();
// ...
std::string text = input.GetInputText();
input.StopTextInput();

// Обновление (в игровом цикле)
input.Update();
```

---

## ResourceManager

### Описание

Singleton для управления ресурсами (текстуры, шрифты, аудио).

### Определение

```cpp
namespace SAGE::Resources {
    class ResourceManager {
    public:
        static ResourceManager& Get();
        
        bool Initialize();
        void Shutdown();
        
        // Текстуры
        Ref<Texture> LoadTexture(const std::string& path);
        Ref<Texture> LoadTextureFromMemory(const uint8_t* data, size_t size,
                                          const std::string& name);
        Ref<Texture> CreateTexture(uint32_t width, uint32_t height,
                                  TextureFormat format, const std::string& name);
        
        // Шрифты
        Ref<Font> LoadFont(const std::string& path, int size);
        Ref<Font> LoadFontFromMemory(const uint8_t* data, size_t dataSize,
                                    int fontSize, const std::string& name);
        
        // Шейдеры
        Ref<Shader> LoadShader(const std::string& vertPath,
                              const std::string& fragPath);
        Ref<Shader> LoadShaderFromSource(const std::string& vertSrc,
                                        const std::string& fragSrc,
                                        const std::string& name);
        
        // Общие методы
        void UnloadResource(const std::string& path);
        void UnloadAllResources();
        void UnloadUnusedResources();
        
        bool IsResourceLoaded(const std::string& path) const;
        size_t GetResourceCount() const;
        
        // Кэш
        void ClearCache();
        void SetCacheSize(size_t sizeInBytes);
        size_t GetCacheSize() const;
        size_t GetCacheUsage() const;
        
        float GetCacheHitRate() const;
        
        // GPU память
        void SetGpuLoadingEnabled(bool enabled);
        bool IsGpuLoadingEnabled() const;
        
        void SetGpuMemoryBudget(size_t sizeInBytes);
        size_t GetGpuMemoryBudget() const;
        size_t GetGpuMemoryUsage() const;
        
        // Асинхронная загрузка
        std::future<Ref<Texture>> LoadTextureAsync(const std::string& path);
        
        void SetAsyncLoadingEnabled(bool enabled);
        bool IsAsyncLoadingEnabled() const;
        
        // Headless режим (для тестов)
        void SetHeadlessMode(bool headless);
        bool IsHeadlessMode() const;
        
        // Статистика
        struct Stats {
            size_t texturesLoaded;
            size_t fontsLoaded;
            size_t shadersLoaded;
            size_t cacheHits;
            size_t cacheMisses;
            size_t totalLoadTime;
        };
        
        Stats GetStats() const;
        void ResetStats();
        
    private:
        ResourceManager() = default;
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

### Использование

```cpp
auto& rm = ResourceManager::Get();
rm.Initialize();

// Загрузка текстур
auto playerTex = rm.LoadTexture("assets/player.png");
auto enemyTex = rm.LoadTexture("assets/enemy.png");

// Повторная загрузка берёт из кэша
auto sameTex = rm.LoadTexture("assets/player.png");  // Кэш хит

// Шрифты
auto font24 = rm.LoadFont("assets/fonts/arial.ttf", 24);
auto font48 = rm.LoadFont("assets/fonts/arial.ttf", 48);

// Шейдеры
auto shader = rm.LoadShader("shaders/sprite.vert", "shaders/sprite.frag");

// GPU память
rm.SetGpuMemoryBudget(512 * 1024 * 1024);  // 512 МБ
size_t usage = rm.GetGpuMemoryUsage();
SAGE_INFO("Resources", "GPU memory: {} MB", usage / (1024 * 1024));

// Выгрузка
rm.UnloadResource("assets/old_texture.png");
rm.UnloadUnusedResources();  // Выгрузить с refcount = 0

// Асинхронная загрузка
auto future = rm.LoadTextureAsync("assets/large_texture.png");
// ... делаем другую работу ...
auto tex = future.get();

// Статистика
auto stats = rm.GetStats();
float hitRate = rm.GetCacheHitRate();
SAGE_INFO("Resources", "Cache hit rate: {:.1f}%", hitRate * 100);

// Очистка
rm.Shutdown();
```

---

## EventBus

### Описание

Singleton для pub/sub системы событий.

### Определение

```cpp
namespace SAGE::Events {
    enum class EventPriority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };
    
    enum class EventCategory {
        None = 0,
        Input = 1 << 0,
        Gameplay = 1 << 1,
        Physics = 1 << 2,
        Audio = 1 << 3,
        UI = 1 << 4,
        Network = 1 << 5,
        Debug = 1 << 6,
        All = 0xFF
    };
    
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
        
        template<typename EventType>
        uint64_t SubscribeWeak(std::weak_ptr<void> owner,
                              std::function<void(const EventType&)> handler,
                              uint32_t group = 0);
        
        // Отписка
        template<typename EventType>
        void Unsubscribe(uint64_t handlerId);
        
        void UnsubscribeGroup(uint32_t group);
        void UnsubscribeAll();
        
        // Публикация
        template<typename EventType>
        void Publish(const EventType& event);
        
        template<typename EventType>
        void Enqueue(const EventType& event,
                    EventPriority priority = EventPriority::Normal);
        
        void Flush();
        void Clear();
        
        // Фильтрация
        void EnableCategory(EventCategory category);
        void DisableCategory(EventCategory category);
        bool IsCategoryEnabled(EventCategory category) const;
        
        void SetCategoryMask(uint32_t mask);
        uint32_t GetCategoryMask() const;
        
        // Статистика
        size_t GetSubscriberCount() const;
        size_t GetQueuedEventCount() const;
        
        struct Stats {
            uint64_t eventsPublished;
            uint64_t eventsQueued;
            uint64_t eventsFlushed;
            uint64_t subscriptions;
            uint64_t unsubscriptions;
        };
        
        Stats GetStats() const;
        void ResetStats();
        
    private:
        EventBus() = default;
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

### Использование

```cpp
auto& bus = EventBus::Get();

// Определение события
struct PlayerDiedEvent {
    Entity player;
    Vector2 position;
    int score;
    EventCategory category = EventCategory::Gameplay;
};

struct EnemySpawnedEvent {
    Entity enemy;
    Vector2 position;
    EventCategory category = EventCategory::Gameplay;
};

// Подписка
uint64_t handlerId = bus.Subscribe<PlayerDiedEvent>(
    [](const PlayerDiedEvent& e) {
        SAGE_INFO("Game", "Player died at ({}, {})", e.position.x, e.position.y);
        // Обработка
    },
    0,    // Группа
    100   // Приоритет (больше = раньше)
);

// Подписка с автоматической отпиской
auto handler = bus.SubscribeScoped<EnemySpawnedEvent>(
    [](const EnemySpawnedEvent& e) {
        SpawnParticles(e.position);
    }
);
// handler автоматически отписывается при выходе из области видимости

// Публикация
PlayerDiedEvent event;
event.player = playerEntity;
event.position = Vector2(100, 200);
event.score = 1000;
bus.Publish(event);  // Немедленная обработка

// Отложенная публикация
bus.Enqueue(event, EventPriority::High);
// ...
bus.Flush();  // Обработать все отложенные

// Группы
uint64_t uiHandler = bus.Subscribe<ButtonClickEvent>(
    [](const ButtonClickEvent& e) { /* ... */ },
    1  // UI группа
);

// Отписать всю UI группу
bus.UnsubscribeGroup(1);

// Категории
bus.EnableCategory(EventCategory::Gameplay);
bus.DisableCategory(EventCategory::Debug);

// Статистика
auto stats = bus.GetStats();
SAGE_INFO("Events", "Published: {}, Queued: {}",
          stats.eventsPublished, stats.eventsQueued);
```

---

## Продолжение следует...

Этот справочник содержит детальное описание всех основных систем. Для дополнительной информации см.:

- **COMPONENT_REFERENCE.md** - все компоненты
- **API_REFERENCE.md** - полный API
- **USER_GUIDE.md** - практические примеры
