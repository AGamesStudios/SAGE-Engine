# SAGE Engine - Руководство пользователя

## Быстрый старт

### Требования

**Минимальные системные требования:**
- Windows 10/11 или Linux (Ubuntu 20.04+)
- OpenGL 3.3+
- 2 ГБ RAM
- 500 МБ свободного места на диске

**Инструменты разработки:**
- CMake 3.15+
- Visual Studio 2022 (Windows) или GCC 9+ (Linux)
- Git

### Установка

1. Клонирование репозитория:

```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
```

2. Конфигурация проекта:

**Windows:**
```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
```

**Linux:**
```bash
cmake -S . -B build -G "Unix Makefiles"
```

3. Сборка:

**Windows:**
```powershell
cmake --build build --config Release
```

**Linux:**
```bash
cmake --build build --config Release -- -j$(nproc)
```

4. Запуск тестов:

```bash
cd build/bin/Release
./SAGETests
```

Ожидаемый результат: `[  PASSED  ] 99 test(s)` (или больше в зависимости от версии)

### Первый проект

Создайте файл `main.cpp`:

```cpp
#include <SAGE/Core/Application.h>
#include <SAGE/ECS/Registry.h>
#include <SAGE/ECS/Components.h>
#include <SAGE/Graphics/RenderContext.h>

using namespace SAGE;

class MyFirstGame : public Application {
public:
    MyFirstGame() : Application("My First Game", 800, 600) {}

protected:
    void OnInit() override {
        // Создаём игрока
        player = registry.CreateEntity();
        
        // Позиция
        TransformComponent transform;
        transform.position = Vector2(400, 300);
        transform.size = Vector2(32, 32);
        registry.AddComponent(player, transform);
        
        // Спрайт
        SpriteComponent sprite;
        sprite.texturePath = "assets/player.png";
        sprite.tint = Color::White();
        registry.AddComponent(player, sprite);
    }
    
    void OnUpdate(float deltaTime) override {
        // Управление стрелками
        auto* transform = registry.GetComponent<TransformComponent>(player);
        if (!transform) return;
        
        float speed = 200.0f;
        auto& input = InputManager::Get();
        
        if (input.IsKeyDown(GLFW_KEY_LEFT))
            transform->position.x -= speed * deltaTime;
        if (input.IsKeyDown(GLFW_KEY_RIGHT))
            transform->position.x += speed * deltaTime;
        if (input.IsKeyDown(GLFW_KEY_UP))
            transform->position.y -= speed * deltaTime;
        if (input.IsKeyDown(GLFW_KEY_DOWN))
            transform->position.y += speed * deltaTime;
    }
    
    void OnRender() override {
        auto& ctx = RenderContext::Instance();
        ctx.Clear(Color(30, 30, 40));
        
        // Рендеринг спрайтов автоматически через RenderSystem
    }

private:
    Registry registry;
    Entity player;
};

int main() {
    MyFirstGame game;
    return game.Run();
}
```

Компиляция:

```bash
g++ main.cpp -o MyGame -lSAGE -lglfw -lGL
./MyGame
```

## Работа с системами

### Физическая симуляция

Пример создания физических объектов:

```cpp
void CreatePhysicsObject() {
    Entity box = registry.CreateEntity();
    
    // Transform
    TransformComponent transform;
    transform.position = Vector2(400, 100);
    transform.size = Vector2(50, 50);
    registry.AddComponent(box, transform);
    
    // Physics
    PhysicsComponent physics;
    physics.type = PhysicsBodyType::Dynamic;
    physics.mass = 1.0f;
    physics.restitution = 0.3f;  // Упругость
    registry.AddComponent(box, physics);
    
    // Collider
    auto collider = ColliderComponent::CreateBox(Vector2(50, 50));
    registry.AddComponent(box, collider);
}
```

Применение силы:

```cpp
auto* physics = registry.GetComponent<PhysicsComponent>(box);
if (physics) {
    physics->ApplyForce(Vector2(100, 0));  // Толчок вправо
}
```

### Анимация

Создание анимационного клипа:

```cpp
// Создаём клип
auto clip = std::make_shared<AnimationClip>("walk");
clip->SetLooping(true);

// Добавляем кадры (для sprite sheet 4x4)
float frameWidth = 1.0f / 4.0f;
float frameHeight = 1.0f / 4.0f;

for (int i = 0; i < 4; i++) {
    float u = i * frameWidth;
    clip->AddFrame(
        Float2(u, 0),                          // uvMin
        Float2(u + frameWidth, frameHeight),   // uvMax
        0.1f                                   // duration (100ms)
    );
}

// Добавляем к entity
AnimationComponent anim;
anim.SetClip(clip);
anim.Play();
registry.AddComponent(player, anim);
```

Управление воспроизведением:

```cpp
auto* anim = registry.GetComponent<AnimationComponent>(player);
if (anim) {
    anim->Play();
    anim->Pause();
    anim->Stop();
    anim->playbackSpeed = 2.0f;  // Ускорить в 2 раза
}
```

### Аудио

Загрузка и воспроизведение звуков:

```cpp
AudioSystem audio;
audio.Init();

// SFX
audio.LoadSFX("jump", "assets/sounds/jump.wav", 3);  // 3 голоса
audio.LoadSFX("shoot", "assets/sounds/shoot.wav", 5);

// Воспроизведение
audio.PlaySFX("jump", 1.0f);  // Громкость 100%

// Музыка
audio.LoadBGM("menu", "assets/music/menu.mp3");
audio.PlayBGM("menu", 0.7f, 2.0f);  // Громкость 70%, fade in 2 сек

// Управление громкостью
audio.SetMasterVolume(0.8f);
audio.SetSFXVolume(1.0f);
audio.SetBGMVolume(0.6f);
```

3D позиционирование звука:

```cpp
// Позиция слушателя (обычно камера/игрок)
audio.SetListenerPosition(playerX, playerY, 0);

// Звук будет тише/громче в зависимости от расстояния
audio.PlaySFX("explosion");
```

### События

Создание и использование событий:

```cpp
// Определение события
struct EnemyDefeatedEvent {
    Entity enemy;
    int score;
    Vector2 position;
};

// Подписка
auto& bus = EventBus::Get();
uint64_t handler = bus.Subscribe<EnemyDefeatedEvent>(
    [this](const EnemyDefeatedEvent& e) {
        totalScore += e.score;
        SpawnScorePopup(e.position, e.score);
    },
    0,    // Группа
    100   // Приоритет (больше = раньше)
);

// Публикация
EnemyDefeatedEvent event;
event.enemy = enemyEntity;
event.score = 100;
event.position = enemyPos;
bus.Publish(event);

// Отложенная публикация
bus.Enqueue(event, EventPriority::High);
// ...позже...
bus.Flush();  // Обрабатываем все отложенные события
```

Группы и категории:

```cpp
// Подписка с группой (например, для UI)
uint64_t uiHandler = bus.SubscribeScoped<ButtonClickEvent>(
    [](const ButtonClickEvent& e) { /* ... */ },
    1  // Группа UI
);

// Отписать всю группу одним вызовом
bus.UnsubscribeGroup(1);

// Фильтрация по категориям
bus.EnableCategory(EventCategory::Input);
bus.DisableCategory(EventCategory::Debug);
```

### Ввод

Обработка клавиатуры:

```cpp
auto& input = InputManager::Get();

// Проверка состояния
if (input.IsKeyDown(GLFW_KEY_SPACE)) {
    // Прыжок (срабатывает каждый кадр пока нажата)
    Jump();
}

if (input.IsKeyPressed(GLFW_KEY_SPACE)) {
    // Срабатывает только в момент нажатия
    PlayJumpSound();
}

if (input.IsKeyReleased(GLFW_KEY_SPACE)) {
    // Срабатывает при отпускании
}
```

Мышь:

```cpp
// Позиция
Vector2 mousePos = input.GetMousePosition();
Vector2 mouseDelta = input.GetMouseDelta();

// Кнопки
if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    // Клик
    Vector2 worldPos = camera.ScreenToWorld(mousePos);
    ShootAt(worldPos);
}

// Колесо
float wheel = input.GetMouseWheelDelta();
if (wheel != 0) {
    camera.SetZoom(camera.GetZoom() + wheel * 0.1f);
}
```

Action mapping:

```cpp
// Настройка маппинга
input.MapAction("jump", GLFW_KEY_SPACE);
input.MapAction("shoot", GLFW_KEY_X);
input.MapAction("menu", GLFW_KEY_ESCAPE);

// Использование
if (input.IsActionTriggered("jump")) {
    Jump();
}

if (input.IsActionActive("shoot")) {
    Shoot();
}
```

## Рендеринг

### Камера

Настройка камеры:

```cpp
Camera2D camera(800, 600);

// Позиция (следование за игроком)
auto* transform = registry.GetComponent<TransformComponent>(player);
if (transform) {
    camera.SetPosition(transform->position);
}

// Зум
camera.SetZoom(2.0f);  // 2x увеличение

// Поворот
camera.SetRotation(45.0f);

// Применение
auto& ctx = RenderContext::Instance();
ctx.SetCamera(camera);
```

Преобразование координат:

```cpp
// Экранные -> мировые
Vector2 mouseWorld = camera.ScreenToWorld(input.GetMousePosition());

// Мировые -> экранные
Vector2 enemyScreen = camera.WorldToScreen(enemyPosition);
```

### Слои рендеринга

Управление порядком отрисовки:

```cpp
// Фон
SpriteComponent background;
background.texturePath = "background.png";
background.layer = -100;
registry.AddComponent(bgEntity, background);

// Игрок
SpriteComponent player;
player.texturePath = "player.png";
player.layer = 0;
registry.AddComponent(playerEntity, player);

// UI (всегда сверху)
SpriteComponent healthBar;
healthBar.texturePath = "healthbar.png";
healthBar.layer = 1000;
registry.AddComponent(uiEntity, healthBar);
```

Объекты с большим `layer` рисуются поверх объектов с меньшим.

### Эффекты

Tint (окраска):

```cpp
auto* sprite = registry.GetComponent<SpriteComponent>(entity);
if (sprite) {
    sprite->tint = Color(255, 0, 0, 255);  // Красный
    sprite->tint.a = 128;                  // Полупрозрачный
}
```

Отражение:

```cpp
sprite->flipX = true;   // Зеркально по горизонтали
sprite->flipY = false;
```

UV регион (sprite sheet):

```cpp
// Вырезать часть текстуры
sprite->SetUVRegion(
    512, 512,  // Размер текстуры
    0, 0,      // X, Y начальной точки
    64, 64     // Ширина, высота региона
);
```

## Управление ресурсами

### Загрузка текстур

```cpp
auto& rm = ResourceManager::Get();

// Загрузка
auto texture = rm.LoadTexture("assets/player.png");

// Использование в спрайте
SpriteComponent sprite;
sprite.texture = texture;
sprite.tint = Color::White();

// Текстура кэшируется автоматически
auto sameTexture = rm.LoadTexture("assets/player.png");  // Из кэша
```

### Управление GPU памятью

```cpp
// Проверка использования
size_t used = rm.GetGpuMemoryUsage();
size_t budget = rm.GetGpuMemoryBudget();

float usage = (float)used / budget * 100.0f;
SAGE_INFO("Resources", "GPU memory: {:.1f}%", usage);

// Ручная очистка
rm.UnloadResource("assets/old_texture.png");
rm.ClearCache();  // Очистить весь кэш
```

### Headless режим

Для тестов без GPU:

```cpp
auto& rm = ResourceManager::Get();
rm.SetGpuLoadingEnabled(false);

// Текстуры загружаются, но не отправляются в GPU
auto tex = rm.LoadTexture("test.png");
```

## Отладка и профилирование

### Логирование

```cpp
// Различные уровни
SAGE_TRACE("Game", "Tick #{}", tickCount);
SAGE_INFO("Game", "Level loaded: {}", levelName);
SAGE_WARNING("Physics", "High velocity detected: {}", velocity);
SAGE_ERROR("Audio", "Failed to load sound: {}", filepath);

// С форматированием
SAGE_INFO("Player", "Position: ({:.2f}, {:.2f})", pos.x, pos.y);
```

Логи сохраняются в `logs/sage_engine.log`.

### Производительность

Измерение времени:

```cpp
auto start = std::chrono::high_resolution_clock::now();

// Код для измерения
PhysicsSystem.FixedUpdate(registry, deltaTime);

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

SAGE_INFO("Perf", "Physics update: {} μs", duration.count());
```

FPS счётчик:

```cpp
void OnUpdate(float deltaTime) override {
    static float fpsTimer = 0;
    static int frameCount = 0;
    
    frameCount++;
    fpsTimer += deltaTime;
    
    if (fpsTimer >= 1.0f) {
        float fps = frameCount / fpsTimer;
        SAGE_INFO("Perf", "FPS: {:.1f}", fps);
        
        fpsTimer = 0;
        frameCount = 0;
    }
}
```

### Отладка физики

Визуализация коллайдеров:

```cpp
// В OnRender()
registry.ForEach<TransformComponent, ColliderComponent>(
    [](Entity e, TransformComponent& t, ColliderComponent& c) {
        if (c.shape == ColliderShape::Box) {
            DrawDebugRect(t.position, c.boxSize, Color::Green());
        } else if (c.shape == ColliderShape::Circle) {
            DrawDebugCircle(t.position, c.radius, Color::Green());
        }
    }
);
```

Raycast визуализация:

```cpp
PhysicsSystem::RaycastHit hit;
bool hasHit = physics.Raycast(origin, direction, 1000.0f, hit);

DrawDebugLine(origin, origin + direction * 1000.0f, Color::Yellow());

if (hasHit) {
    DrawDebugPoint(hit.point, 5.0f, Color::Red());
}
```

## Оптимизация

### Entity pooling

Переиспользование entity вместо создания новых:

```cpp
class BulletPool {
    std::vector<Entity> inactive;
    
public:
    Entity Acquire(Registry& reg) {
        if (!inactive.empty()) {
            Entity e = inactive.back();
            inactive.pop_back();
            return e;
        }
        
        // Создаём новый
        Entity e = reg.CreateEntity();
        // Инициализация компонентов...
        return e;
    }
    
    void Release(Entity e) {
        inactive.push_back(e);
    }
};
```

### Батчинг спрайтов

Группировка по текстуре для минимизации draw calls:

```cpp
// Используйте одну текстуру для нескольких объектов
auto spriteSheet = rm.LoadTexture("assets/spritesheet.png");

for (auto& enemyDef : enemyDefinitions) {
    SpriteComponent sprite;
    sprite.texture = spriteSheet;  // Та же текстура
    sprite.SetUVRegion(/* координаты в sprite sheet */);
    registry.AddComponent(enemy, sprite);
}

// Движок автоматически батчит все спрайты с одной текстурой
```

### Spatial partitioning

Для большого количества объектов:

```cpp
// Используйте физические queries вместо перебора всех entity
std::vector<Entity> nearbyEnemies = 
    physics.QueryAABB(playerPos - Vector2(200, 200),
                      playerPos + Vector2(200, 200));

// Обрабатываем только близкие объекты
for (Entity e : nearbyEnemies) {
    // AI, взаимодействие и т.д.
}
```

### Object sleeping

Деактивация неактивных объектов:

```cpp
auto* physics = registry.GetComponent<PhysicsComponent>(entity);
if (physics) {
    // Объекты с низкой скоростью "засыпают"
    if (!physics->isAwake) {
        // Пропустить обновление AI, анимаций и т.д.
        return;
    }
}
```

## Конфигурация

### engine_config.json

```json
{
  "window": {
    "width": 1920,
    "height": 1080,
    "title": "My Game",
    "fullscreen": false,
    "vsync": true,
    "resizable": true
  },
  
  "graphics": {
    "maxTextureSize": 4096,
    "anisotropicFiltering": 8,
    "msaa": 4,
    "gpuMemoryBudgetMB": 512
  },
  
  "physics": {
    "gravity": [0, 980],
    "velocityIterations": 8,
    "positionIterations": 3,
    "timeStep": 0.016667,
    "enableSleeping": true
  },
  
  "audio": {
    "masterVolume": 0.8,
    "sfxVolume": 1.0,
    "bgmVolume": 0.7,
    "maxSFXVoices": 32,
    "streamThresholdKB": 512
  },
  
  "resources": {
    "assetPath": "assets/",
    "cacheSizeMB": 256,
    "enableCompression": true
  }
}
```

Загрузка конфигурации:

```cpp
#include <nlohmann/json.hpp>
#include <fstream>

std::ifstream file("engine_config.json");
nlohmann::json config;
file >> config;

int width = config["window"]["width"];
int height = config["window"]["height"];
bool vsync = config["window"]["vsync"];

Application app("My Game", width, height);
// ...
```

## Обработка ошибок

### Проверка компонентов

Всегда проверяйте результат GetComponent:

```cpp
auto* transform = registry.GetComponent<TransformComponent>(entity);
if (!transform) {
    SAGE_WARNING("Game", "Entity missing transform");
    return;  // Безопасный выход
}

// Теперь безопасно использовать
transform->position += velocity * dt;
```

### Проверка валидности Entity

```cpp
if (!registry.IsValid(entity)) {
    SAGE_WARNING("Game", "Invalid entity {}", entity);
    return;
}

// Entity существует, можно работать
```

### Обработка загрузки ресурсов

```cpp
auto& rm = ResourceManager::Get();

// Попытка загрузить текстуру
auto texture = rm.LoadTexture("assets/player.png");
if (!texture) {
    SAGE_ERROR("Game", "Failed to load player texture");
    
    // Вариант 1: Использовать placeholder
    texture = rm.LoadTexture("assets/error.png");
    
    // Вариант 2: Создать простую текстуру
    // texture = CreateFallbackTexture();
    
    if (!texture) {
        // Критическая ошибка - невозможно продолжить
        SAGE_ERROR("Game", "Cannot create fallback texture");
        return false;
    }
}
```

### Try-catch для критических операций

```cpp
try {
    auto config = LoadGameConfig("config.json");
    InitializeGame(config);
} catch (const std::exception& e) {
    SAGE_ERROR("Game", "Initialization failed: {}", e.what());
    // Использовать дефолтную конфигурацию
    InitializeGame(GetDefaultConfig());
}
```

### Graceful degradation

```cpp
class Game : public Application {
    bool hasAudio = false;
    bool hasPhysics = false;
    
    void OnInit() override {
        // Инициализация с проверками
        hasAudio = audio.Init();
        if (!hasAudio) {
            SAGE_WARNING("Game", "Running without audio");
        }
        
        hasPhysics = physics.Init();
        if (!hasPhysics) {
            SAGE_ERROR("Game", "Physics required!");
            Quit();
            return;
        }
    }
    
    void OnUpdate(float dt) override {
        if (hasPhysics) {
            physics.Update(registry, dt);
        }
        
        if (hasAudio && shouldPlaySound) {
            audio.PlaySFX("jump");
        }
    }
};
```

---

## Частые проблемы

### Текстуры не загружаются

**Проблема:** Спрайты не отображаются.

**Решение:**
1. Проверьте путь к файлу:
```cpp
auto tex = rm.LoadTexture("assets/player.png");
if (!tex) {
    SAGE_ERROR("Game", "Failed to load texture");
}
```

2. Убедитесь, что файл существует и доступен.

3. Проверьте формат (поддерживаются PNG, JPG, TGA).

### Физические объекты не двигаются

**Проблема:** Dynamic body не реагирует на силы.

**Решение:**
1. Проверьте тип тела:
```cpp
physics.type = PhysicsBodyType::Dynamic;  // Не Static!
```

2. Установите ненулевую массу:
```cpp
physics.SetMass(1.0f);
```

3. Убедитесь, что PhysicsSystem обновляется:
```cpp
void OnUpdate(float dt) override {
    physicsSystem.FixedUpdate(registry, dt);
}
```

### Низкая производительность

**Проблема:** FPS падает при большом количестве объектов.

**Решение:**
1. Используйте батчинг - одна текстура для многих спрайтов.

2. Включите frustum culling:
```cpp
// Объекты за пределами экрана не рендерятся (автоматически)
```

3. Ограничьте количество активных физических тел:
```cpp
// Деактивируйте далёкие объекты
if (Distance(entity, player) > 1000.0f) {
    registry.RemoveComponent<PhysicsComponent>(entity);
}
```

4. Профилируйте узкие места:
```cpp
// Измеряйте время систем
SAGE_INFO("Perf", "Physics: {} ms", physicsTime);
```

### Утечки памяти

**Проблема:** Память постоянно растёт.

**Решение:**
1. Удаляйте неиспользуемые entity:
```cpp
registry.DestroyEntity(bullet);
```

2. Очищайте кэш ресурсов:
```cpp
rm.ClearCache();
```

3. Отписывайтесь от событий:
```cpp
bus.Unsubscribe<MyEvent>(handlerId);
```

### Аудио не воспроизводится

**Проблема:** Звуки не слышны.

**Решение:**
1. Проверьте инициализацию:
```cpp
if (!audio.Init()) {
    SAGE_ERROR("Audio", "Failed to initialize");
}
```

2. Проверьте громкость:
```cpp
audio.SetMasterVolume(1.0f);
audio.SetSFXVolume(1.0f);
```

3. Проверьте формат файла (поддерживаются WAV, MP3, OGG).

4. Для длинных файлов используйте streaming:
```cpp
audio.LoadBGM("music", "music.mp3");  // Streaming автоматически
```

## Примеры проектов

### Платформер

```cpp
class Platformer : public Application {
    Registry registry;
    PhysicsSystem physics;
    Entity player;
    
public:
    Platformer() : Application("Platformer", 1280, 720) {}
    
protected:
    void OnInit() override {
        // Настройка физики
        physics.SetGravity(Vector2(0, 980));
        
        // Игрок
        player = CreatePlayer(Vector2(100, 100));
        
        // Платформы
        CreatePlatform(Vector2(400, 500), Vector2(400, 40));
        CreatePlatform(Vector2(200, 350), Vector2(200, 40));
    }
    
    void OnUpdate(float dt) override {
        HandleInput(dt);
        physics.FixedUpdate(registry, dt);
        UpdateCamera();
    }
    
private:
    Entity CreatePlayer(const Vector2& pos) {
        Entity e = registry.CreateEntity();
        
        TransformComponent transform;
        transform.position = pos;
        transform.size = Vector2(32, 48);
        registry.AddComponent(e, transform);
        
        PhysicsComponent phys;
        phys.type = PhysicsBodyType::Dynamic;
        phys.SetMass(1.0f);
        phys.fixedRotation = true;
        registry.AddComponent(e, phys);
        
        auto collider = ColliderComponent::CreateBox(Vector2(32, 48));
        registry.AddComponent(e, collider);
        
        SpriteComponent sprite;
        sprite.texturePath = "player.png";
        registry.AddComponent(e, sprite);
        
        return e;
    }
    
    Entity CreatePlatform(const Vector2& pos, const Vector2& size) {
        Entity e = registry.CreateEntity();
        
        TransformComponent transform;
        transform.position = pos;
        transform.size = size;
        registry.AddComponent(e, transform);
        
        PhysicsComponent phys;
        phys.type = PhysicsBodyType::Static;
        registry.AddComponent(e, phys);
        
        auto collider = ColliderComponent::CreateBox(size);
        registry.AddComponent(e, collider);
        
        return e;
    }
    
    void HandleInput(float dt) {
        auto& input = InputManager::Get();
        auto* phys = registry.GetComponent<PhysicsComponent>(player);
        
        float moveSpeed = 300.0f;
        
        if (input.IsKeyDown(GLFW_KEY_A))
            phys->velocity.x = -moveSpeed;
        else if (input.IsKeyDown(GLFW_KEY_D))
            phys->velocity.x = moveSpeed;
        else
            phys->velocity.x = 0;
        
        if (input.IsKeyPressed(GLFW_KEY_SPACE)) {
            // Прыжок (только если на земле)
            if (IsGrounded(player)) {
                phys->ApplyImpulse(Vector2(0, -400));
            }
        }
    }
    
    bool IsGrounded(Entity e) {
        auto* transform = registry.GetComponent<TransformComponent>(e);
        Vector2 rayOrigin = transform->position + Vector2(0, transform->size.y / 2);
        
        PhysicsSystem::RaycastHit hit;
        return physics.Raycast(rayOrigin, Vector2(0, 1), 5.0f, hit);
    }
    
    void UpdateCamera() {
        auto& ctx = RenderContext::Instance();
        auto& camera = ctx.GetCamera();
        
        auto* transform = registry.GetComponent<TransformComponent>(player);
        camera.SetPosition(transform->position);
    }
};
```

### Top-down shooter

```cpp
class TopDownShooter : public Application {
    Registry registry;
    PhysicsSystem physics;
    Entity player;
    AudioSystem audio;
    std::vector<Entity> bullets;
    float shootCooldown = 0;
    
public:
    TopDownShooter() : Application("Shooter", 1280, 720) {}
    
protected:
    void OnInit() override {
        audio.Init();
        audio.LoadSFX("shoot", "shoot.wav", 5);
        audio.LoadSFX("hit", "hit.wav", 10);
        
        player = CreatePlayer(Vector2(640, 360));
        
        // Враги
        for (int i = 0; i < 10; i++) {
            Vector2 pos(Math::Random::Global().NextRange(100, 1180),
                       Math::Random::Global().NextRange(100, 620));
            CreateEnemy(pos);
        }
    }
    
    void OnUpdate(float dt) override {
        HandleInput(dt);
        UpdateBullets(dt);
        CheckCollisions();
        physics.FixedUpdate(registry, dt);
        audio.Update(dt);
        
        shootCooldown -= dt;
    }
    
private:
    void HandleInput(float dt) {
        auto& input = InputManager::Get();
        auto* transform = registry.GetComponent<TransformComponent>(player);
        
        // Движение WASD
        Vector2 move(0, 0);
        if (input.IsKeyDown(GLFW_KEY_W)) move.y -= 1;
        if (input.IsKeyDown(GLFW_KEY_S)) move.y += 1;
        if (input.IsKeyDown(GLFW_KEY_A)) move.x -= 1;
        if (input.IsKeyDown(GLFW_KEY_D)) move.x += 1;
        
        if (move.LengthSquared() > 0) {
            move = move.Normalized();
            transform->position += move * 200.0f * dt;
        }
        
        // Направление к мыши
        auto& ctx = RenderContext::Instance();
        Vector2 mouseWorld = ctx.GetCamera().ScreenToWorld(
            input.GetMousePosition()
        );
        Vector2 direction = (mouseWorld - transform->position).Normalized();
        transform->rotation = std::atan2(direction.y, direction.x) * 180.0f / 3.14159f;
        
        // Стрельба
        if (input.IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && shootCooldown <= 0) {
            Shoot(transform->position, direction);
            shootCooldown = 0.1f;
        }
    }
    
    void Shoot(const Vector2& pos, const Vector2& dir) {
        Entity bullet = registry.CreateEntity();
        
        TransformComponent transform;
        transform.position = pos;
        transform.size = Vector2(8, 8);
        registry.AddComponent(bullet, transform);
        
        PhysicsComponent phys;
        phys.type = PhysicsBodyType::Dynamic;
        phys.velocity = dir * 800.0f;
        phys.gravityScale = 0;
        registry.AddComponent(bullet, phys);
        
        auto collider = ColliderComponent::CreateCircle(4.0f);
        collider.isSensor = true;
        registry.AddComponent(bullet, collider);
        
        bullets.push_back(bullet);
        audio.PlaySFX("shoot", 0.5f);
    }
    
    void UpdateBullets(float dt) {
        for (auto it = bullets.begin(); it != bullets.end();) {
            auto* transform = registry.GetComponent<TransformComponent>(*it);
            
            // Удаляем пули за пределами экрана
            if (transform->position.x < -100 || transform->position.x > 1380 ||
                transform->position.y < -100 || transform->position.y > 820) {
                registry.DestroyEntity(*it);
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

## Заключение

Это руководство покрывает основные аспекты работы с SAGE Engine. Для более детальной информации см.:

- **API_REFERENCE.md** - полное описание всех классов и методов
- **ARCHITECTURE.md** - архитектура и внутреннее устройство
- **Examples/** - примеры проектов в директории Examples/

Дополнительная помощь:
- GitHub Issues: https://github.com/AGamesStudios/SAGE-Engine/issues
- Wiki: https://github.com/AGamesStudios/SAGE-Engine/wiki
