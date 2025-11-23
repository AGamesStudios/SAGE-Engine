# ECS (Entity-Component-System)

Лёгкая ECS в `SAGE::ECS` построена на спарс-сет пулах и версионируемых идентификаторах.

- `Entity`: 32-битный хендл (index + version).
- `Registry`: создаёт/удаляет сущности, добавляет/читает/удаляет компоненты, перебирает сущности по подписи компонентов (`ForEach`).
- `ComponentPool<T>`: плотные массивы + sparse-индексы для O(1) доступа без «пустых» слотов.
- `ISystem` / `SystemScheduler`: логика в системах (`Tick(registry, dt)`), последовательный вызов через планировщик.

## Быстрый старт

```cpp
#include <SAGE/Core/ECS.h>
using namespace SAGE::ECS;

struct Transform { float x{}, y{}; };
struct Velocity { float vx{}, vy{}; };

class MoveSystem : public ISystem {
public:
    void Tick(Registry& reg, float dt) override {
        reg.ForEach<Transform, Velocity>([dt](Entity, Transform& t, Velocity& v) {
            t.x += v.vx * dt;
            t.y += v.vy * dt;
        });
    }
};

int main() {
    Registry world;
    SystemScheduler systems;
    systems.AddSystem<MoveSystem>();

    Entity e = world.CreateEntity();
    world.Add<Transform>(e, Transform{0, 0});
    world.Add<Velocity>(e, Velocity{10, 0});

    systems.UpdateAll(world, 0.016f);
}
```

## Основное API

- Сущности: `CreateEntity()`, `DestroyEntity(e)`, `IsAlive(e)`, `AliveCount()`.
- Компоненты: `Add<T>(e, args...)`, `Get<T>(e)`, `Remove<T>(e)`, `Has<T>(e)`.
- Итерации: `ForEach<Comp...>(fn)` перебирает только сущности с нужными компонентами; стартует с наименьшего пула для снижения проверок.
- Очистка: `Registry::Clear()`, `SystemScheduler::Clear()`.

## Производительность

- Спарс-сет экономит память и даёт O(1) доступ; нет хранения отсутствующих компонентов.
- Версионирование хендлов защищает от использования удалённых сущностей.
- Эвристика минимального пула в `ForEach` ускоряет перебор при больших объёмах данных.

## Тестирование

- Юнит-тесты: `Tests/ECSTests.cpp` (создание/уничтожение, Add/Get/Remove, ForEach, порядок систем).
- Запуск после конфигурации CMake:
  ```powershell
  cmake --build build --config Debug
  .\build\bin\Debug\SAGE_Tests.exe
  ```

## Расширяемость

- Новый компонент — обычный `struct`, без наследования.
- Новая система — унаследовать `ISystem`, реализовать `Tick`, зарегистрировать в `SystemScheduler`.
- Можно добавлять произвольные типы данных в компоненты; система обрабатывает только «подписанные» сущности.
