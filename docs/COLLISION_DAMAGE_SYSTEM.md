# Система обработки урона при столкновении (DamageSystem)

## Обзор

SAGE Engine включает полнофункциональную систему обработки столкновений и урона:

1. **CollisionSystem** - обнаруживает AABB столкновения и устанавливает флаг `colliding`
2. **PhysicsSystem** - обрабатывает Box2D физику с Contact Listener
3. **DamageSystem** - наносит урон сущностям при столкновении

## Компоненты

### DamageOnCollisionComponent

```cpp
struct DamageOnCollisionComponent {
    int damageAmount = 10;           // Размер урона
    float cooldown = 0.5f;           // Минимальное время между ударами (сек)
    float timeSinceLastDamage = 0.0f; // Внутренний таймер
    bool damageOnce = false;         // Если true - урон только один раз
    bool hasDealtDamage = false;     // Для режима damageOnce
};
```

### HealthComponent

```cpp
struct HealthComponent {
    int maxHealth = 100;
    int currentHealth = 100;
    bool IsDead() const { return currentHealth <= 0; }
};
```

### ColliderComponent

```cpp
struct ColliderComponent {
    Vector2 size{32.0f, 32.0f};
    Vector2 offset{0.0f, 0.0f};
    bool isTrigger = false;
    bool colliding = false;  // <-- Флаг установленный CollisionSystem
};
```

## Использование

### Пример 1: Враг наносит урон игроку

```cpp
// Создание врага
Entity enemy = registry.Create();

// Физика и коллайдер
auto& rb = registry.AddComponent<RigidBodyComponent>(enemy);
rb.type = BodyType::Dynamic;

auto& physCol = registry.AddComponent<PhysicsColliderComponent>(enemy);
physCol.shape = ColliderShape::Box;
physCol.size = {50.0f, 50.0f};

auto& col = registry.AddComponent<ColliderComponent>(enemy);
col.size = {50.0f, 50.0f};

// Здоровье
auto& health = registry.AddComponent<HealthComponent>(enemy);
health.maxHealth = 50;
health.currentHealth = 50;

// УРОН: враг наносит 10 урона каждые 1 секунду при контакте
auto& dmg = registry.AddComponent<DamageOnCollisionComponent>(enemy);
dmg.damageAmount = 10;
dmg.cooldown = 1.0f;
dmg.damageOnce = false;
```

### Пример 2: Пуля наносит урон один раз

```cpp
// Создание пули
Entity bullet = registry.Create();

// Коллайдер
auto& col = registry.AddComponent<ColliderComponent>(bullet);
col.size = {10.0f, 10.0f};
col.isTrigger = true;

// УРОН: пуля наносит 50 урона при первом столкновении
auto& dmg = registry.AddComponent<DamageOnCollisionComponent>(bullet);
dmg.damageAmount = 50;
dmg.cooldown = 0.1f;
dmg.damageOnce = true;  // <-- Урон только один раз!
dmg.hasDealtDamage = false;
```

### Пример 3: Огненный предмет с прерыванием

```cpp
// Создание огня (наносит урон с перерывом)
Entity fire = registry.Create();

auto& col = registry.AddComponent<ColliderComponent>(fire);
col.size = {30.0f, 30.0f};
col.isTrigger = true;

// УРОН: огонь наносит 5 урона каждые 0.3 сек
auto& dmg = registry.AddComponent<DamageOnCollisionComponent>(fire);
dmg.damageAmount = 5;
dmg.cooldown = 0.3f;
dmg.damageOnce = false;
dmg.timeSinceLastDamage = 0.0f;
```

## Поток выполнения

### Каждый frame:

1. **CollisionSystem::Tick()**
   - Сбрасывает флаги `colliding` для всех коллайдеров
   - Проверяет AABB пересечения между всеми коллайдерами
   - Устанавливает `colliding = true` для пересекающихся коллайдеров

2. **PhysicsSystem::Tick()**
   - Шагает Box2D физику
   - Contact Listener вызывает `BeginContact`/`EndContact`
   - Также устанавливает флаги `colliding`
   - Синхронизирует трансформы с Box2D телами

3. **DamageSystem::Tick()**
   - Для каждого компонента с `DamageOnCollisionComponent`:
     - Обновляет `timeSinceLastDamage`
     - Если коллайдер пересекается И прошел cooldown:
       - Ищет жертв в области коллайдера
       - Вычитает `damageAmount` из их `HealthComponent`
     - Если `damageOnce = true`, отмечает `hasDealtDamage`

## Wireframe режим рендеринга

### Использование:

```cpp
// Включить wireframe режим
Renderer::SetRenderMode(RenderMode::Wireframe);

// Полный рендер в режиме каркаса
// Все примитивы будут нарисованы как линии

// Вернуться в нормальный режим
Renderer::SetRenderMode(RenderMode::Solid);

// Проверить текущий режим
if (Renderer::GetRenderMode() == RenderMode::Wireframe) {
    // ... в режиме каркаса
}
```

### Пример использования в игре:

```cpp
class DebugDemoGame : public ECSGame {
    bool m_WireframeMode = false;
    
    void OnSceneUpdate(Scene& scene, float deltaTime) override {
        // Переключение режима по T
        if (Input::IsKeyPressed(Key::T)) {
            m_WireframeMode = !m_WireframeMode;
            RenderMode mode = m_WireframeMode ? RenderMode::Wireframe : RenderMode::Solid;
            Renderer::SetRenderMode(mode);
        }
        
        // ... другой код
    }
};
```

## Батчинг по текстурам

Система рендеринга **уже оптимизирована** с батчингом по текстурам:

```cpp
// В SpriteRenderSystem::Tick():
auto sorter = [](const DrawItem& a, const DrawItem& b) {
    if (a.layer != b.layer) return a.layer < b.layer;
    return a.texture < b.texture;  // <-- Сортировка по текстуре!
};
std::stable_sort(opaque.begin(), opaque.end(), sorter);
std::stable_sort(transparent.begin(), transparent.end(), sorter);
```

Это означает, что спрайты автоматически группируются по текстуре, что минимизирует смену текстур и улучшает производительность.

## Архитектура систем

### SystemScheduler

Все системы работают через `SystemScheduler`, который вызывает их в правильном порядке:

```cpp
scene.GetScheduler().AddSystem(std::make_unique<AnimationSystem>());
scene.GetScheduler().AddSystem(std::make_unique<CollisionSystem>());
scene.GetScheduler().AddSystem(std::make_unique<PhysicsSystem>());
scene.GetScheduler().AddSystem(std::make_unique<DamageSystem>());
scene.GetScheduler().AddSystem(std::make_unique<SpriteRenderSystem>());
```

### Order of Execution:

1. Input Systems
2. Physics Systems (PhysicsSystem, CollisionSystem)
3. Gameplay Systems (DamageSystem, MovementSystem)
4. Animation Systems
5. Render Systems (SpriteRenderSystem, HudRenderSystem)

## Примеры запуска

### Базовая демонстрация столкновений:

```bash
cmake --build build --config Release --target CollisionDamageDemo
.\build\bin\Release\CollisionDamageDemo.exe
```

**Управление в демо:**
- W/A/S/D - движение игрока
- Столкновение с врагами наносит урон обоим
- Враги наносят игроку 10 урона каждую секунду
- Когда HP = 0, сущность должна удалиться (при реализации)

### EntitySystemDemo:

```bash
cmake --build build --config Release --target EntitySystemDemo
.\build\bin\Release\EntitySystemDemo.exe
```

## Best Practices

1. **Cooldown для врагов:** Устанавливайте cooldown >= 1.0 сек
2. **Пули:** Используйте `damageOnce = true` для одноразовых снарядов
3. **Яд/Огонь:** Используйте `damageOnce = false` и низкий cooldown (0.2-0.5 сек)
4. **Отладка:** Включайте wireframe для визуализации коллайдеров

## Дальнейшие улучшения

Возможные расширения:

- [ ] Типы урона (физический, магический, огонь, лед)
- [ ] Система статус-эффектов (яд, оглушение, горение)
- [ ] Точки урона и броня
- [ ] Разные типы коллайдеров (круг, полигон)
- [ ] Raycast запросы для проверки видимости
- [ ] Смерть и удаление сущностей
- [ ] Звуки удара и урона

## Файлы

- **Header:** `Engine/include/SAGE/Core/ECSComponents.h` - DamageOnCollisionComponent
- **Header:** `Engine/include/SAGE/Core/ECSSystems.h` - DamageSystem
- **Implementation:** `Engine/src/Core/ECSSystems.cpp` - DamageSystem::Tick()
- **Example:** `Examples/CollisionDamageDemo.cpp` - Полный пример использования

## Контроль качества

Код использует:
- ✅ RAII для управления ресурсами
- ✅ Безопасные проверки null указателей
- ✅ Валидация компонентов перед использованием
- ✅ Типобезопасные операции через Registry
- ✅ Логирование ошибок через Logger
