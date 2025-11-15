# SAGE Engine - Руководство по оптимизации производительности

## Обзор

Это руководство содержит рекомендации по оптимизации производительности игр на SAGE Engine.

## Профилирование

### Встроенный профайлер

**Примечание:** Profiler доступен только в Debug и RelWithDebInfo сборках. В Release макросы профилирования отключены.

```cpp
#include <SAGE/Core/Profiler.h>

// Профилирование функции
void ExpensiveFunction() {
    SAGE_PROFILE_FUNCTION();
    // Код функции
}

// Профилирование блока
void Update(float dt) {
    {
        SAGE_PROFILE_SCOPE("Physics Update");
        physicsSystem.FixedUpdate(registry, dt);
    }
    
    {
        SAGE_PROFILE_SCOPE("Animation Update");
        animationSystem.Update(registry, dt);
    }
}

// Получение результатов
auto results = Profiler::GetResults();
for (const auto& [name, stats] : results) {
    SAGE_INFO("Profiler", "{}: {:.3f} ms (avg), {} calls",
              name, stats.averageTime, stats.callCount);
}
```

### Измерение FPS

```cpp
class Game : public Application {
    float fpsTimer = 0;
    int frameCount = 0;
    
    void OnUpdate(float dt) override {
        frameCount++;
        fpsTimer += dt;
        
        if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            float ms = (fpsTimer / frameCount) * 1000.0f;
            
            SAGE_INFO("Perf", "FPS: {:.1f} ({:.2f} ms/frame)", fps, ms);
            
            frameCount = 0;
            fpsTimer = 0;
        }
    }
};
```

### GPU профилирование

**Примечание:** GPU profiling требует OpenGL 3.3+ и расширения GL_ARB_timer_query.

```cpp
#include <SAGE/Graphics/GPUProfiler.h>

void Render() {
    GPU_PROFILE_SCOPE("Scene Rendering");
    
    {
        GPU_PROFILE_SCOPE("Opaque Pass");
        RenderOpaque();
    }
    
    {
        GPU_PROFILE_SCOPE("Transparent Pass");
        RenderTransparent();
    }
}

// Результаты
auto gpuStats = GPUProfiler::GetResults();
for (const auto& [name, time] : gpuStats) {
    SAGE_INFO("GPU", "{}: {:.2f} ms", name, time);
}
```

---

## ECS оптимизация

### Использование компонентов

**Хорошо: Группировка часто используемых данных**

```cpp
// Один компонент для связанных данных
struct MovementComponent {
    Vector2 velocity;
    Vector2 acceleration;
    float maxSpeed;
    float friction;
};
```

**Плохо: Разбросанные данные**

```cpp
// Плохо - требует множество cache misses
struct VelocityComponent { Vector2 velocity; };
struct AccelerationComponent { Vector2 acceleration; };
struct MaxSpeedComponent { float maxSpeed; };
struct FrictionComponent { float friction; };
```

### Размер компонентов

Держите компоненты небольшими для кэш-дружественности:

```cpp
// Хорошо - 32 байта
struct TransformComponent {
    Vector2 position;   // 8 байт
    Vector2 size;       // 8 байт
    Vector2 scale;      // 8 байт
    float rotation;     // 4 байта
    // padding 4 байта
};

// Плохо - 256+ байт
struct BadComponent {
    std::vector<Vector2> path;        // Динамический массив
    std::map<std::string, int> data;  // Тяжёлая структура
    std::string name;                 // Динамическая строка
};
```

Рекомендации:
- Идеально: <= 64 байт
- Хорошо: <= 128 байт
- Приемлемо: <= 256 байт

### Итерация по компонентам

**Хорошо: Минимальная итерация**

```cpp
// Обрабатываем только то, что нужно
registry.ForEach<TransformComponent, VelocityComponent>(
    [dt](Entity e, TransformComponent& t, VelocityComponent& v) {
        t.position += v.velocity * dt;
    }
);
```

**Плохо: Избыточные проверки**

```cpp
// Плохо - итерация по всем entity
auto allEntities = registry.GetAllEntities();
for (Entity e : allEntities) {
    auto* transform = registry.GetComponent<TransformComponent>(e);
    auto* velocity = registry.GetComponent<VelocityComponent>(e);
    if (transform && velocity) {  // Много избыточных проверок
        transform->position += velocity->velocity * dt;
    }
}
```

### Entity pooling

Переиспользование entity вместо создания новых:

```cpp
class EntityPool {
    std::vector<Entity> active;
    std::vector<Entity> inactive;
    Registry& registry;
    
public:
    Entity Acquire() {
        if (!inactive.empty()) {
            Entity e = inactive.back();
            inactive.pop_back();
            active.push_back(e);
            
            // Сброс компонентов к дефолтным значениям
            Reset(e);
            return e;
        }
        
        Entity e = CreateNew();
        active.push_back(e);
        return e;
    }
    
    void Release(Entity e) {
        auto it = std::find(active.begin(), active.end(), e);
        if (it != active.end()) {
            active.erase(it);
            inactive.push_back(e);
        }
    }
    
private:
    Entity CreateNew() {
        Entity e = registry.CreateEntity();
        // Инициализация компонентов
        return e;
    }
    
    void Reset(Entity e) {
        // Сброс к начальным значениям
        auto* transform = registry.GetComponent<TransformComponent>(e);
        if (transform) {
            transform->position = Vector2::Zero();
            transform->rotation = 0;
        }
    }
};
```

Использование:

```cpp
EntityPool bulletPool;

// Выстрел
Entity bullet = bulletPool.Acquire();
auto* transform = registry.GetComponent<TransformComponent>(bullet);
transform->position = gunPos;

// Удаление пули
bulletPool.Release(bullet);  // Не уничтожается, а возвращается в пул
```

---

## Рендеринг оптимизация

### Батчинг спрайтов

**Хорошо: Одна текстура для множества объектов**

```cpp
// Используем sprite sheet
auto spriteSheet = rm.LoadTexture("spritesheet.png");

for (int i = 0; i < 1000; i++) {
    Entity e = registry.CreateEntity();
    
    SpriteComponent sprite;
    sprite.texture = spriteSheet;  // Та же текстура
    sprite.SetUVRegion(512, 512, (i % 8) * 64, (i / 8) * 64, 64, 64);
    
    registry.AddComponent(e, sprite);
}
// Результат: 1 draw call вместо 1000
```

**Плохо: Разные текстуры**

```cpp
// Плохо - каждый спрайт своя текстура
for (int i = 0; i < 1000; i++) {
    Entity e = registry.CreateEntity();
    
    SpriteComponent sprite;
    sprite.texturePath = "sprite_" + std::to_string(i) + ".png";
    
    registry.AddComponent(e, sprite);
}
// Результат: 1000 draw calls
```

### Texture Atlas

Создание текстурного атласа:

```cpp
// Инструмент для создания атласа (можно использовать TexturePacker)
// Или вручную:
struct AtlasRegion {
    std::string name;
    Float2 uvMin, uvMax;
};

class TextureAtlas {
    Ref<Texture> texture;
    std::map<std::string, AtlasRegion> regions;
    
public:
    void AddRegion(const std::string& name, 
                   int x, int y, int w, int h) {
        int texW = texture->GetWidth();
        int texH = texture->GetHeight();
        
        regions[name] = {
            name,
            Float2(x / (float)texW, y / (float)texH),
            Float2((x + w) / (float)texW, (y + h) / (float)texH)
        };
    }
    
    AtlasRegion GetRegion(const std::string& name) const {
        return regions.at(name);
    }
};

// Использование
TextureAtlas atlas;
atlas.Load("game_atlas.png");
atlas.AddRegion("player", 0, 0, 64, 64);
atlas.AddRegion("enemy", 64, 0, 64, 64);

SpriteComponent sprite;
sprite.texture = atlas.GetTexture();
auto region = atlas.GetRegion("player");
sprite.uvMin = region.uvMin;
sprite.uvMax = region.uvMax;
```

### Frustum Culling

Автоматическое отсечение объектов вне экрана:

```cpp
RenderSystem renderSystem;
renderSystem.EnableFrustumCulling(true);  // Включено по умолчанию

// Ручная проверка (если нужно для логики)
bool IsVisible(const Vector2& position, const Vector2& size, 
               const Camera2D& camera) {
    // AABB камеры
    Vector2 camPos = camera.GetPosition();
    float camW = camera.GetWidth() / camera.GetZoom();
    float camH = camera.GetHeight() / camera.GetZoom();
    
    Vector2 camMin = camPos - Vector2(camW, camH) * 0.5f;
    Vector2 camMax = camPos + Vector2(camW, camH) * 0.5f;
    
    // AABB объекта
    Vector2 objMin = position - size * 0.5f;
    Vector2 objMax = position + size * 0.5f;
    
    // AABB пересечение
    return !(objMax.x < camMin.x || objMin.x > camMax.x ||
             objMax.y < camMin.y || objMin.y > camMax.y);
}
```

### Слои рендеринга

Оптимизация через правильное использование слоёв:

```cpp
// Статичный фон (низкий слой, рисуется первым)
SpriteComponent background;
background.layer = -1000;

// Игровые объекты (средний слой)
SpriteComponent gameObject;
gameObject.layer = 0;

// UI (высокий слой, рисуется последним)
SpriteComponent ui;
ui.layer = 1000;

// Статичные объекты одного слоя батчуются вместе
```

### Occlusion Culling

Для игр с большими уровнями:

```cpp
class OcclusionGrid {
    std::vector<std::vector<bool>> grid;
    int cellSize;
    
public:
    void MarkOccluded(const Vector2& pos, const Vector2& size) {
        int x1 = pos.x / cellSize;
        int y1 = pos.y / cellSize;
        int x2 = (pos.x + size.x) / cellSize;
        int y2 = (pos.y + size.y) / cellSize;
        
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                if (x >= 0 && x < grid[0].size() && 
                    y >= 0 && y < grid.size()) {
                    grid[y][x] = true;
                }
            }
        }
    }
    
    bool IsOccluded(const Vector2& pos) const {
        int x = pos.x / cellSize;
        int y = pos.y / cellSize;
        
        if (x >= 0 && x < grid[0].size() && 
            y >= 0 && y < grid.size()) {
            return grid[y][x];
        }
        return false;
    }
};
```

---

## Физика оптимизация

### Spatial partitioning

Используйте встроенные query вместо перебора всех entity:

```cpp
// Плохо - проверка всех entity
registry.ForEach<TransformComponent, PhysicsComponent>(
    [&](Entity e, TransformComponent& t, PhysicsComponent& p) {
        float dist = t.position.Distance(playerPos);
        if (dist < 100.0f) {
            // Обработка близких объектов
        }
    }
);

// Хорошо - query только в области
std::vector<Entity> nearby = physics.QueryAABB(
    playerPos - Vector2(100, 100),
    playerPos + Vector2(100, 100)
);

for (Entity e : nearby) {
    // Обработка только близких
}
```

### Physics sleeping

Объекты с низкой скоростью автоматически "засыпают":

```cpp
PhysicsSettings settings;
settings.enableSleeping = true;
settings.linearSleepThreshold = 0.01f;   // Порог скорости
settings.angularSleepThreshold = 0.01f;  // Порог вращения
settings.timeToSleep = 0.5f;             // Время до усыпления
physics.SetPhysicsSettings(settings);

// Проверка состояния
auto* phys = registry.GetComponent<PhysicsComponent>(entity);
if (!phys->isAwake) {
    // Пропустить обновление AI, анимации и т.д.
    return;
}
```

### LOD для физики

Упрощение физики для далёких объектов:

```cpp
void UpdatePhysics(Entity e, float distance) {
    auto* physics = registry.GetComponent<PhysicsComponent>(e);
    auto* collider = registry.GetComponent<ColliderComponent>(e);
    
    if (distance > 500.0f) {
        // Очень далеко - отключить физику
        physics->type = PhysicsBodyType::Static;
    }
    else if (distance > 200.0f) {
        // Далеко - упрощённый коллайдер
        collider->shape = ColliderShape::Circle;
        collider->radius = 10.0f;
    }
    else {
        // Близко - полная физика
        physics->type = PhysicsBodyType::Dynamic;
        collider->shape = ColliderShape::Polygon;
        // Полная геометрия
    }
}
```

### Фиксированный timestep

```cpp
class Game : public Application {
    float accumulator = 0;
    const float fixedDt = 1.0f / 60.0f;  // 60 Hz
    
    void OnUpdate(float dt) override {
        accumulator += dt;
        
        // Фиксированные шаги физики
        while (accumulator >= fixedDt) {
            physics.FixedUpdate(registry, fixedDt);
            accumulator -= fixedDt;
        }
        
        // Интерполяция для плавности
        float alpha = accumulator / fixedDt;
        InterpolateTransforms(alpha);
    }
    
    void InterpolateTransforms(float alpha) {
        registry.ForEach<TransformComponent, PhysicsComponent>(
            [alpha](Entity e, TransformComponent& t, PhysicsComponent& p) {
                t.position = Vector2::Lerp(p.previousPos, p.position, alpha);
            }
        );
    }
};
```

---

## Память оптимизация

### Object pooling

Общий паттерн для переиспользования объектов:

```cpp
template<typename T>
class ObjectPool {
    std::vector<T*> available;
    std::vector<std::unique_ptr<T>> all;
    
public:
    T* Acquire() {
        if (!available.empty()) {
            T* obj = available.back();
            available.pop_back();
            return obj;
        }
        
        all.push_back(std::make_unique<T>());
        return all.back().get();
    }
    
    void Release(T* obj) {
        available.push_back(obj);
    }
    
    void Clear() {
        available.clear();
        all.clear();
    }
};

// Использование
ObjectPool<Particle> particlePool;

void SpawnParticle(const Vector2& pos) {
    Particle* p = particlePool.Acquire();
    p->position = pos;
    p->lifetime = 2.0f;
    activeParticles.push_back(p);
}

void UpdateParticles(float dt) {
    for (auto it = activeParticles.begin(); it != activeParticles.end();) {
        Particle* p = *it;
        p->lifetime -= dt;
        
        if (p->lifetime <= 0) {
            particlePool.Release(p);
            it = activeParticles.erase(it);
        } else {
            ++it;
        }
    }
}
```

### Resource управление

```cpp
auto& rm = ResourceManager::Get();

// Установка бюджета памяти
rm.SetGpuMemoryBudget(512 * 1024 * 1024);  // 512 MB

// Выгрузка неиспользуемых ресурсов
rm.UnloadUnusedResources();

// Проверка использования
size_t usage = rm.GetGpuMemoryUsage();
size_t budget = rm.GetGpuMemoryBudget();
float percent = (float)usage / budget * 100.0f;

if (percent > 90.0f) {
    SAGE_WARNING("Memory", "GPU memory usage: {:.1f}%", percent);
    rm.UnloadUnusedResources();
}
```

### Ленивая загрузка

```cpp
class AssetManager {
    std::map<std::string, Ref<Texture>> textures;
    
public:
    Ref<Texture> GetTexture(const std::string& path) {
        // Загрузить только при первом использовании
        if (textures.find(path) == textures.end()) {
            textures[path] = ResourceManager::Get().LoadTexture(path);
        }
        return textures[path];
    }
    
    void UnloadUnused() {
        for (auto it = textures.begin(); it != textures.end();) {
            if (it->second.use_count() == 1) {  // Только в кэше
                it = textures.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

---

## Общие рекомендации

### 1. Профилируйте перед оптимизацией

```cpp
// Сначала измерьте
SAGE_PROFILE_FUNCTION();

// Потом оптимизируйте узкие места
```

### 2. Избегайте преждевременной оптимизации

Пишите чистый код сначала, оптимизируйте потом когда найдёте проблемы.

### 3. Используйте константы для настройки

```cpp
constexpr int MAX_PARTICLES = 1000;
constexpr float CULLING_DISTANCE = 1000.0f;
constexpr int PHYSICS_ITERATIONS = 8;
```

### 4. Кэшируйте частые вычисления

```cpp
// Плохо
for (Entity e : entities) {
    float dist = position.Distance(GetPlayerPosition());
}

// Хорошо
Vector2 playerPos = GetPlayerPosition();
for (Entity e : entities) {
    float dist = position.Distance(playerPos);
}
```

### 5. Используйте правильные структуры данных

```cpp
// Для быстрого поиска
std::unordered_map<std::string, Entity> entityMap;

// Для частой итерации
std::vector<Entity> entities;

// Для приоритетной очереди
std::priority_queue<Event> eventQueue;
```

---

## Бенчмарки

### Тестовая конфигурация

**Система 1 (средний ПК):**
- CPU: Intel Core i5-10400 (6 cores, 12 threads, 2.9-4.3 GHz)
- RAM: 16 GB DDR4 2666 MHz
- GPU: NVIDIA GTX 1660 (6 GB VRAM)
- OS: Windows 10 64-bit
- Компилятор: MSVC 2022 (v143)
- Конфигурация: Release, x64

**Система 2 (низкий ПК):**
- CPU: Intel Core i3-7100 (2 cores, 4 threads, 3.9 GHz)
- RAM: 8 GB DDR4 2133 MHz
- GPU: Intel HD Graphics 630
- OS: Windows 10 64-bit

**Методология:**
- Измерения проводились в Release сборке
- Каждый тест запускался 100 раз, указано среднее значение
- VSync отключен
- Без других приложений

### Результаты (средний ПК)

Типичная производительность SAGE Engine:

| Операция | Производительность |
|----------|-------------------|
| Entity создание | ~10,000 entity/ms |
| ForEach (1 компонент) | ~1,000,000 entity/ms |
| ForEach (3 компонента) | ~500,000 entity/ms |
| Физика (1000 тел) | ~55 ms/update |
| Рендеринг (1000 спрайтов, батчинг) | ~2 ms |
| Рендеринг (1000 спрайтов, без батчинга) | ~50 ms |
| EventBus (100,000 событий) | ~95 ms |
| Raycast (простая сцена) | ~0.1 ms |

Целевая производительность для 60 FPS: 16.67 ms/кадр
