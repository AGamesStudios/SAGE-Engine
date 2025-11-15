# SAGE Engine - Справочник компонентов

**Версия:** Alpha  
**Namespace:** SAGE::ECS  
**Header:** `<SAGE/ECS/Components.h>`

---

## Содержание

- [Обзор](#обзор)
- [Основные компоненты](#основные-компоненты)
  - [TransformComponent](#transformcomponent)
  - [SpriteComponent](#spritecomponent)
  - [PhysicsComponent](#physicscomponent)
  - [ColliderComponent](#collidercomponent)
  - [AnimationComponent](#animationcomponent)
- [Пользовательские компоненты](#пользовательские-компоненты)
- [Общие паттерны](#общие-паттерны)
- [Производительность](#производительность)

---

## Обзор

Компоненты в SAGE Engine - это POD (Plain Old Data) структуры, содержащие только данные без логики. Они добавляются к Entity через Registry.

**Важно:** Все компоненты определены в namespace `SAGE::ECS`. Для удобства можно использовать `using namespace SAGE::ECS;`

## Основные компоненты

### TransformComponent

**Описание:** Позиция, поворот и размер объекта в мировых координатах.

**Определение:**

```cpp
struct TransformComponent {
    Vector2 position;    // Мировая позиция (центр объекта)
    Vector2 scale;       // Множитель размера (1.0 = 100%)
    Vector2 size;        // Базовый размер объекта
    Vector2 pivot;       // Точка вращения (0.5, 0.5 = центр)
    float rotation;      // Угол в градусах (против часовой стрелки)
};
```

**Поля:**

- `position` - Координаты центра объекта в мировом пространстве
- `scale` - Множитель размера (scale.x = 2.0 удвоит ширину)
- `size` - Базовый размер объекта в пикселях
- `pivot` - Точка вращения в локальных координатах (0,0 - левый верхний, 1,1 - правый нижний)
- `rotation` - Угол поворота в градусах (0 = направо, 90 = вверх)

**Использование:**

```cpp
TransformComponent transform;
transform.position = Vector2(400, 300);  // Центр объекта
transform.size = Vector2(64, 64);        // 64x64 пикселей
transform.scale = Vector2(1.5f, 1.5f);   // Увеличить в 1.5 раза
transform.pivot = Vector2(0.5f, 0.5f);   // Вращение вокруг центра
transform.rotation = 45.0f;              // Повернуть на 45 градусов

registry.AddComponent(entity, transform);
```

**Примечания:**

- Итоговый размер = `size * scale`
- Рендеринг использует `position` как центр спрайта
- Физика синхронизируется с Transform автоматически

---

### SpriteComponent

**Описание:** Визуальное представление объекта через текстуру.

**Определение:**

```cpp
struct SpriteComponent {
    std::string texturePath;
    Ref<Texture> texture;
    Ref<Material> material;
    
    Color tint;          // Цвет окраски (RGBA)
    bool visible;
    bool flipX;
    bool flipY;
    
    int layer;           // Слой рендеринга (больше = поверх)
    
    Float2 uvMin;        // UV координаты начала (для sprite sheet)
    Float2 uvMax;        // UV координаты конца
    Float2 pivot;
};
```

**Поля:**

- `texturePath` - Путь к файлу текстуры (загружается через ResourceManager)
- `texture` - Ссылка на загруженную текстуру
- `material` - Опциональный материал с шейдером
- `tint` - Цвет тинта (255,255,255,255 = без изменений, alpha влияет на прозрачность)
- `visible` - Флаг видимости (false = не рендерится)
- `flipX/flipY` - Отражение по осям
- `layer` - Порядок отрисовки (объекты с большим layer рисуются поверх)
- `uvMin/uvMax` - UV координаты для sprite sheet (0,0 до 1,1)
- `pivot` - Точка привязки текстуры

**Использование:**

```cpp
SpriteComponent sprite;
sprite.texturePath = "assets/player.png";
sprite.tint = Color::White();
sprite.visible = true;
sprite.layer = 0;
sprite.flipX = false;

// Для sprite sheet (вырезать часть текстуры)
sprite.SetUVRegion(
    512, 512,  // Размер текстуры
    64, 0,     // Начало (x, y)
    64, 64     // Размер региона (w, h)
);

registry.AddComponent(entity, sprite);
```

**Слои:**

- Фон: -1000 до -100
- Игровые объекты: -100 до 100
- UI: 100 до 1000

**Примечания:**

- Текстуры кэшируются автоматически
- UV координаты: (0,0) = левый верхний угол, (1,1) = правый нижний
- Батчинг спрайтов с одинаковой текстурой автоматический

---

### PhysicsComponent

**Описание:** Физические свойства объекта для симуляции.

**Определение:**

```cpp
enum class PhysicsBodyType {
    Static,    // Неподвижный (стены, платформы)
    Dynamic,   // Полная физика (игрок, враги)
    Kinematic  // Управляется кодом, но влияет на другие
};

struct PhysicsComponent {
    PhysicsBodyType type;
    
    float mass;                // Масса в кг
    float inverseMass;         // 1/масса (кэш)
    float linearDamping;       // Затухание линейной скорости (0-1)
    float angularDamping;      // Затухание угловой скорости (0-1)
    float gravityScale;        // Множитель гравитации (1.0 = обычная)
    
    float restitution;         // Упругость (0 = не отскакивает, 1 = полностью)
    float staticFriction;      // Трение покоя
    float dynamicFriction;     // Трение движения
    
    Vector2 velocity;          // Линейная скорость (м/с)
    Vector2 force;             // Накопленная сила
    float angularVelocity;     // Угловая скорость (рад/с)
    float torque;              // Крутящий момент
    
    bool fixedRotation;        // Запретить вращение
    bool isBullet;             // Continuous collision detection
    bool isAwake;              // Активен ли объект
    bool isSensor;             // Триггер без физического столкновения
};
```

**Поля:**

- `type` - Тип физического тела
- `mass` - Масса объекта (автоматически вычисляет inverseMass)
- `linearDamping` - Сопротивление воздуха для скорости (0.1 = медленное затухание)
- `angularDamping` - Сопротивление для вращения
- `gravityScale` - Множитель гравитации (0 = невесомость, 2 = удвоенная)
- `restitution` - Упругость при столкновениях (0.5 = теряет половину энергии)
- `staticFriction` - Трение покоя (больше = труднее начать движение)
- `dynamicFriction` - Трение движения (больше = быстрее замедляется)
- `velocity` - Текущая скорость
- `force` - Накопленная сила (очищается после каждого шага)
- `angularVelocity` - Скорость вращения
- `torque` - Крутящий момент
- `fixedRotation` - true = объект не вращается (для персонажей)
- `isBullet` - true = CCD для быстрых объектов (пули)
- `isAwake` - false = объект "спит" (не симулируется)
- `isSensor` - true = обнаруживает столкновения, но не сталкивается физически

**Использование:**

```cpp
PhysicsComponent physics;
physics.type = PhysicsBodyType::Dynamic;
physics.SetMass(1.0f);
physics.restitution = 0.3f;     // Немного упругий
physics.staticFriction = 0.5f;
physics.dynamicFriction = 0.3f;
physics.gravityScale = 1.0f;
physics.fixedRotation = true;   // Для персонажа
physics.isBullet = false;

registry.AddComponent(entity, physics);

// Применение силы
auto* phys = registry.GetComponent<PhysicsComponent>(entity);
phys->ApplyForce(Vector2(100, 0));      // Постоянная сила
phys->ApplyImpulse(Vector2(0, -500));   // Моментальный импульс
```

**Типы тел:**

- `Static` - Для стен, платформ, статичных препятствий
- `Dynamic` - Для объектов с полной физикой (игрок, враги, ящики)
- `Kinematic` - Для движущихся платформ, управляемых скриптом

**Примечания:**

- Требует ColliderComponent для формы
- Синхронизируется с TransformComponent автоматически
- Сила очищается после каждого шага физики

---

### ColliderComponent

**Описание:** Форма для обнаружения столкновений.

**Определение:**

```cpp
enum class ColliderShape {
    Box,      // Прямоугольник
    Circle,   // Круг
    Polygon   // Произвольный многоугольник
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
    
    float density;       // Плотность (для вычисления массы)
};
```

**Поля:**

- `shape` - Тип формы коллайдера
- `offset` - Смещение центра коллайдера относительно Transform
- `boxSize` - Размер прямоугольника (для Box)
- `radius` - Радиус (для Circle)
- `vertices` - Вершины многоугольника (для Polygon, против часовой стрелки)
- `density` - Плотность материала (масса = плотность * площадь)

**Использование:**

```cpp
// Box коллайдер
auto boxCollider = ColliderComponent::CreateBox(Vector2(50, 50));
boxCollider.offset = Vector2(0, 0);  // Центрирован
boxCollider.density = 1.0f;
registry.AddComponent(entity, boxCollider);

// Circle коллайдер
auto circleCollider = ColliderComponent::CreateCircle(25.0f);
circleCollider.offset = Vector2(0, 0);
circleCollider.density = 1.0f;
registry.AddComponent(entity, circleCollider);

// Polygon коллайдер (треугольник)
std::vector<Vector2> vertices = {
    Vector2(0, -20),
    Vector2(-20, 20),
    Vector2(20, 20)
};
auto polyCollider = ColliderComponent::CreatePolygon(vertices);
registry.AddComponent(entity, polyCollider);
```

**Рекомендации:**

- Box - самый быстрый, используйте по умолчанию
- Circle - для круглых объектов (мячи, враги)
- Polygon - только для сложных форм (макс. 8 вершин)

**Примечания:**

- Требует PhysicsComponent для физической симуляции
- Форма влияет на производительность: Box > Circle > Polygon
- Вершины Polygon должны быть выпуклыми и против часовой стрелки

---

### AnimationComponent

**Описание:** Анимация спрайта через последовательность кадров.

**Определение:**

```cpp
struct AnimationComponent {
    Ref<AnimationClip> clip;
    
    size_t currentFrame;
    float timeAccumulator;
    bool isPlaying;
    bool isLooping;
    float playbackSpeed;
};
```

**Поля:**

- `clip` - Ссылка на клип анимации (последовательность кадров)
- `currentFrame` - Индекс текущего кадра
- `timeAccumulator` - Накопленное время для смены кадра
- `isPlaying` - Флаг воспроизведения
- `isLooping` - Зацикливать ли анимацию
- `playbackSpeed` - Множитель скорости (2.0 = в 2 раза быстрее)

**Использование:**

```cpp
// Создание клипа
auto walkClip = std::make_shared<AnimationClip>("walk");
walkClip->SetLooping(true);

// Добавление кадров (для sprite sheet 4x4)
for (int i = 0; i < 4; i++) {
    float u = i * 0.25f;
    walkClip->AddFrame(
        Float2(u, 0),           // uvMin
        Float2(u + 0.25f, 0.25f), // uvMax
        0.1f                    // duration (100 мс)
    );
}

// Добавление к entity
AnimationComponent anim;
anim.SetClip(walkClip);
anim.playbackSpeed = 1.0f;
anim.Play();
registry.AddComponent(entity, anim);

// Управление
auto* animComp = registry.GetComponent<AnimationComponent>(entity);
animComp->Play();
animComp->Pause();
animComp->Stop();
animComp->Reset();
```

**Примечания:**

- Автоматически обновляет UV координаты SpriteComponent
- Требует SpriteComponent на том же entity
- AnimationSystem обрабатывает обновление кадров

---

## Пользовательские компоненты

### Создание компонента

```cpp
// Определение
struct HealthComponent {
    int currentHealth;
    int maxHealth;
    bool isDead;
    
    HealthComponent() 
        : currentHealth(100)
        , maxHealth(100)
        , isDead(false) {}
    
    HealthComponent(int max) 
        : currentHealth(max)
        , maxHealth(max)
        , isDead(false) {}
    
    void TakeDamage(int damage) {
        currentHealth -= damage;
        if (currentHealth <= 0) {
            currentHealth = 0;
            isDead = true;
        }
    }
    
    void Heal(int amount) {
        currentHealth += amount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
        isDead = false;
    }
    
    float GetHealthPercent() const {
        return (float)currentHealth / maxHealth;
    }
};
```

### Использование

```cpp
// Добавление
HealthComponent health(100);
registry.AddComponent(player, health);

// Получение и модификация
auto* healthComp = registry.GetComponent<HealthComponent>(player);
if (healthComp) {
    healthComp->TakeDamage(25);
    
    if (healthComp->isDead) {
        // Обработка смерти
    }
}

// Итерация
registry.ForEach<HealthComponent, TransformComponent>(
    [](Entity e, HealthComponent& health, TransformComponent& trans) {
        if (health.isDead) {
            // Спавн частиц смерти на позиции trans.position
        }
    }
);
```

### Рекомендации

**Хорошие компоненты:**

```cpp
// POD структура, только данные
struct VelocityComponent {
    Vector2 velocity;
    float maxSpeed;
};

// Простая логика для удобства
struct TimerComponent {
    float duration;
    float elapsed;
    bool IsFinished() const { return elapsed >= duration; }
};
```

**Плохие компоненты:**

```cpp
// Избегайте тяжелой логики в компонентах
struct BadComponent {
    void Update(float dt) {  // НЕТ! Логика должна быть в System
        // ...
    }
    
    std::vector<Entity> children;  // НЕТ! Храните в отдельной структуре
    Registry* registry;            // НЕТ! Компоненты не должны знать о Registry
};
```

**Принципы:**

1. Компоненты содержат только данные
2. Логика находится в Systems
3. Минимум зависимостей между компонентами
4. POD типы когда возможно (для кэш-дружественности)
5. Простые методы-хелперы допустимы (GetHealthPercent, IsFinished)

## Общие паттерны

### Компоненты-флаги

```cpp
struct PlayerTag {};
struct EnemyTag {};
struct BulletTag {};

// Использование
registry.AddComponent(player, PlayerTag{});

// Фильтрация
registry.ForEach<PlayerTag, TransformComponent>(
    [](Entity e, PlayerTag&, TransformComponent& t) {
        // Только игроки
    }
);
```

### Компоненты состояния

```cpp
struct CharacterState {
    enum class State {
        Idle,
        Walking,
        Jumping,
        Falling,
        Attacking
    };
    
    State current;
    State previous;
    float stateTime;  // Время в текущем состоянии
};
```

### Компоненты таймеров

```cpp
struct CooldownComponent {
    float shootCooldown;
    float dashCooldown;
    float specialCooldown;
};

struct LifetimeComponent {
    float lifetime;
    float elapsed;
};
```

## Производительность

### Размер компонентов

Рекомендуется держать компоненты небольшими для кэш-дружественности:

- Идеально: <= 64 байта
- Хорошо: <= 128 байт
- Приемлемо: <= 256 байт
- Избегайте: > 256 байт

### Частота доступа

Группируйте часто используемые данные в одном компоненте:

```cpp
// Хорошо - часто используемые данные вместе
struct MovementComponent {
    Vector2 velocity;
    Vector2 acceleration;
    float maxSpeed;
    float friction;
};

// Плохо - разбросано по нескольким компонентам
struct VelocityComponent { Vector2 velocity; };
struct AccelerationComponent { Vector2 acceleration; };
struct MaxSpeedComponent { float maxSpeed; };
```

### Опциональные данные

Используйте std::optional для редко используемых полей:

```cpp
struct AIComponent {
    Vector2 target;
    float detectionRadius;
    std::optional<std::vector<Vector2>> pathfinding;  // Только если нужен pathfinding
};
```
