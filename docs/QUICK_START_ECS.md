# Quick Start: ECSGame (SAGE Engine)

Базовый порядок систем (кадр): `InputState -> PlayerInput -> Movement -> Collision -> Stats -> Animation -> Particles -> Audio -> CameraFollow -> SpriteRender -> HudRender`.

## Минимальный пример
```cpp
#include <SAGE/SAGE.h>
using namespace SAGE;
using namespace SAGE::ECS;

class MyGame : public ECSGame {
public:
    MyGame() : ECSGame({.window = {.title = "My ECS Game", .width = 1280, .height = 720}}) {}

    void OnECSCreate() override {
        auto tex = Texture::Create("assets/player.png");
        if (!tex) tex = Texture::CreateWhiteTexture();

        // игрок
        Entity p = GetRegistry().CreateEntity();
        auto& t = GetRegistry().Add<TransformComponent>(p);
        t.position = {200,300}; t.scale = {48,48};
        auto& s = GetRegistry().Add<SpriteComponent>(p);
        s.sprite.SetTexture(tex);
        GetRegistry().Add<PlayerTag>(p);
        GetRegistry().Add<VelocityComponent>(p);
        GetRegistry().Add<PlayerMovementComponent>(p);
        GetRegistry().Add<HealthComponent>(p);
        GetRegistry().Add<ColliderComponent>(p).size = {48,48};

        SetCameraTarget(p, 5.0f);
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

## Подключение ввода
- Клавиатура/мышь читаются в `InputStateSystem` → `InputComponent`.
- `PlayerInputSystem` использует `InputComponent` (если есть) или прямой опрос клавиш.
- Обновление ввода происходит после `PollEvents()` и до пользовательской логики/систем.

## Коллизии
- Добавьте `ColliderComponent` (size/offset) и `TransformComponent`.
- `CollisionSystem` помечает `colliding = true` при пересечении AABB. Реакцию (урон/отскок) реализуйте в своей логике (`OnECSUpdate`).

## Частицы
- Добавьте `ParticleEmitterComponent` на сущность. Настройте `config` и `emissionRate`, `playing`.
- `ParticleSystemSystem` обновит и отрисует частицы (DrawParticle), рендерится после анимации.

## HUD/UI
- `HudRenderSystem` рисует HP/EN первого `PlayerTag` и флаг паузы (`m_Paused` в `ECSGame`). Поддержка TextRenderer обязательна.

## Пауза/фокус окна
- В `ECSGame` фокус окна ставит паузу (`OnFocusChanged`). В паузе логика/системы не обновляются, но рендер можно использовать для UI.

## Рендер
- `SpriteRenderSystem` сортирует по слою и текстуре (батчинг уменьшает glBind/glDraw). Прозрачные спрайты (transparent=true) рисуются после непрозрачных.
- Режимы рендера: `Renderer::SetRenderMode(RenderMode::Wireframe/Solid)`, `Renderer::EnableBlending(true)`, `Renderer::SetBlendFunc(...)`.

## Порядок кадра (Application)
1. `PollEvents()`
2. `Input::Update()`
3. `OnECSUpdate(dt)` — ваша логика
4. `SystemScheduler.UpdateAll(...)` — системы в указанном порядке
5. UI/гизма (`OnECSRender`)
6. `SwapBuffers()`

## Сборка/запуск
```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
.\build\bin\Debug\SAGE_Tests.exe   # если включены тесты
```

## Wireframe для отладки
```cpp
Renderer::SetRenderMode(RenderMode::Wireframe); // включить
Renderer::SetRenderMode(RenderMode::Solid);     // вернуть
```

## Мини-сцены/демо
- `Examples/ECSBasicDemo.cpp`: игрок + враги, коллизии, сбор предмета, HUD. Требует текстуры (`assets/player.png`, `enemy.png`, `item.png`) или подставит белую.

## Рекомендации по тестам
- Покрыть: CollisionSystem (AABB), ParticleSystemSystem (умирание частиц), InputStateSystem (флаги клавиш), SpriteRenderSystem (сортировка прозрачных/непрозрачных), HudRenderSystem (вывод HP/PAUSED).
