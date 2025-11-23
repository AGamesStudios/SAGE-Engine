# 🚀 Быстрый Старт с SAGE Engine

Добро пожаловать! Это руководство поможет вам создать вашу первую игру на SAGE Engine за 5 минут.

## Шаг 1: Создание Проекта

Создайте новый `.cpp` файл (например, `MyGame.cpp`) в папке `Examples` или в вашем собственном проекте.

Минимальный шаблон игры выглядит так:

```cpp
#include <SAGE/SAGE.h>

using namespace SAGE;

class MyGame : public Game {
public:
    // Настройка окна при запуске
    MyGame() : Game({
        .window = {
            .title = "My First SAGE Game",
            .width = 1280,
            .height = 720
        }
    }) {}

    // Инициализация ресурсов
    void OnGameInit() override {
        // Загружаем текстуру игрока
        m_PlayerTexture = Texture::Create("assets/player.png");
        
        // Создаем сущность игрока
        m_Player = SceneManager::Get().GetCurrentScene()->CreateEntity("Player");
        
        // Добавляем компоненты
        m_Player.AddComponent<SpriteComponent>(m_PlayerTexture);
        m_Player.AddComponent<TransformComponent>(Vector2(0, 0));
    }

    // Логика игры (вызывается каждый кадр)
    void OnUpdate(double deltaTime) override {
        auto& transform = m_Player.GetComponent<TransformComponent>();
        float speed = 200.0f * (float)deltaTime;

        // Управление
        if (Input::IsKeyDown(KeyCode::W)) transform.position.y -= speed;
        if (Input::IsKeyDown(KeyCode::S)) transform.position.y += speed;
        if (Input::IsKeyDown(KeyCode::A)) transform.position.x -= speed;
        if (Input::IsKeyDown(KeyCode::D)) transform.position.x += speed;
    }

private:
    std::shared_ptr<Texture> m_PlayerTexture;
    Entity m_Player;
};

// Точка входа
SAGE_GAME_ENTRY(MyGame)
```

## Шаг 2: Сборка

Если вы добавили файл в папку `Examples`, вам нужно пересобрать проект.

1. Откройте терминал в корне движка.
2. Запустите сборку:
   ```powershell
   cmake --build build/msvc --config Debug
   ```

## Шаг 3: Запуск

Ваш исполняемый файл появится в `build/msvc/bin/Debug/`.

```powershell
./build/msvc/bin/Debug/MyGame.exe
```

## Что дальше?

*   Изучите [Руководство разработчика](docs/GAME_DEVELOPMENT_GUIDE.md) для понимания ECS.
*   Посмотрите примеры в папке `Examples/` (например, `CatAnimationDemo.cpp` или `PhysicsDemo.cpp`).
