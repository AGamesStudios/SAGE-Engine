# Entity-Component System (актуальная версия)

**Дата:** 2025-01-19  
**Статус:** Актуально, старая Entity/EntityManager удалена  
**Основной заголовок:** `Engine/include/SAGE/Core/ECS.h`

## Кратко
- `Entity`: 32-битный хендл (index + version).
- `Registry`: создаёт/удаляет сущности, добавляет/читает/удаляет компоненты, перебирает сущности по набору компонентов через `ForEach`.
- `ComponentPool<T>`: спарс-сет (dense + sparse) для O(1) доступа к данным без «пустых» слотов.
- `ISystem` + `SystemScheduler`: системы с методом `Tick(registry, dt)`, планировщик вызывает их по порядку.

## Основные файлы
- `Engine/include/SAGE/Core/ECS.h` — весь API ECS (Registry, Entity, ComponentPool, ISystem, SystemScheduler).
- `Examples/EntitySystemDemo.cpp` — пример применения ECS (физика, коллизии, здоровье, камера).
- `Tests/ECSTests.cpp` — юнит-тесты на Registry/SystemScheduler.
- `docs/ECS.md` — краткий гайд и пример кода.

## Быстрый старт
```cpp
#include <SAGE/Core/ECS.h>
using namespace SAGE::ECS;

struct Transform { float x{}, y{}; };
struct Velocity  { float vx{}, vy{}; };

Registry world;
Entity e = world.CreateEntity();
world.Add<Transform>(e, Transform{0, 0});
world.Add<Velocity>(e, Velocity{10, 0});

world.ForEach<Transform, Velocity>([](Entity, Transform& t, Velocity& v){
    t.x += v.vx * 0.016f;
});

class MoveSystem : public ISystem {
public: void Tick(Registry& r, float dt) override {
    r.ForEach<Transform, Velocity>([dt](Entity, Transform& t, Velocity& v){
        t.x += v.vx * dt;
        t.y += v.vy * dt;
    });
}};
SystemScheduler sched;
sched.AddSystem<MoveSystem>();
sched.UpdateAll(world, 0.016f);
```

## API ядра
- Сущности: `CreateEntity()`, `DestroyEntity(e)`, `IsAlive(e)`, `AliveCount()`.
- Компоненты: `Add<T>(e, args...)`, `Get<T>(e)`, `Remove<T>(e)`, `Has<T>(e)`.
- Итерации: `ForEach<Comp...>(fn)` — перебор только сущностей с указанными компонентами; выбирается наименее заполненный пул для минимизации проверок.
- Системы: унаследоваться от `ISystem`, добавить через `SystemScheduler::AddSystem`, обновлять `UpdateAll(registry, dt)`.

## Производительность
- Компоненты хранятся плотно (dense), индексируются sparse-таблицей — O(1) доступ, без пустых записей.
- `ForEach` стартует с наименьшего пула компонентов, уменьшая число проверок при больших наборах сущностей.
- Идентификаторы сущностей версионируются — безопасная работа со старыми хендлами после удаления.

## Тесты
- Юнит-тесты: `Tests/ECSTests.cpp` (создание/удаление, Add/Get/Remove, ForEach, порядок систем).
- Запуск (после конфигурации CMake):
  ```powershell
  cmake --build build --config Debug
  .\build\bin\Debug\SAGE_Tests.exe
  ```

## Миграция со старой системы
- Удалены: `Entity.h`, `EntityManager.h`, их реализации и ссылки.
- Используйте `ECS::Registry` вместо `EntityManager`, `ForEach` вместо ручных FindEntitiesWithComponent.
- Пример миграции: создать Registry в сцене/игре, добавить компоненты через `Add<T>`, в цикле обновления вызывать `ForEach` и/или `SystemScheduler::UpdateAll`.

## Известные ограничения
- Нет автоматической резолюции коллизий (только детект) — коллизии обрабатываются на прикладном уровне.
- Нет сериализации/снапшотов по умолчанию.
- Планировщик систем последовательный; параллель не включена.

## Дальнейшие шаги
- Добавить сериализацию компонентов/сцен.
- Включить пространственные структуры (QuadTree) для фильтрации `ForEach` в коллизиях.
- Добавить опциональный параллельный рантайм систем (разделение по данным/компонентам).
