# SAGE Engine Examples & Tutorials# SAGE Engine - Примеры кода



Complete guide to using SAGE Engine with practical examples.## Пример 1: Базовое приложение



## Table of Contents```cpp

#include <SAGE.h>

- [Getting Started](#getting-started)

- [Basic Examples](#basic-examples)class MyFirstGame : public SAGE::Application {

  - [Hello Window](#hello-window)public:

  - [Drawing Shapes](#drawing-shapes)    MyFirstGame() : Application("My First Game") {}

  - [Input Handling](#input-handling)    

- [Intermediate Examples](#intermediate-examples)    void OnInit() override {

  - [Simple Game](#simple-game)        SAGE_INFO("Game initialized!");

  - [Physics Demo](#physics-demo)        SAGE::Renderer::Init();

  - [Particle Effects](#particle-effects)    }

- [Advanced Examples](#advanced-examples)    

  - [UI System](#ui-system)    void OnUpdate(float deltaTime) override {

  - [Asset Management](#asset-management)        // Игровая логика обновляется каждый кадр

  - [Scene Management](#scene-management)    }

- [Tips & Best Practices](#tips--best-practices)    

- [Troubleshooting](#troubleshooting)    void OnRender() override {

        // Очистка экрана голубым цветом

---        SAGE::Renderer::Clear(0.2f, 0.3f, 0.8f, 1.0f);

    }

## Getting Started    

    void OnShutdown() override {

### Prerequisites        SAGE::Renderer::Shutdown();

        SAGE_INFO("Game shutdown");

Before running examples, ensure you have:    }

- SAGE Engine installed (see [INSTALL.md](INSTALL.md))};

- C++20 compatible compiler

- CMake 3.20 or higherSAGE::Application* SAGE::CreateApplication() {

- OpenGL 3.3+ support    return new MyFirstGame();

}

### Running Examples

int main() {

All examples are located in the `Examples/` directory. Each example has its own `CMakeLists.txt` and can be built independently:    auto app = SAGE::CreateApplication();

    app->Run();

```bash    delete app;

cd Examples/SimpleGame    return 0;

mkdir build && cd build}

cmake .. -DCMAKE_PREFIX_PATH="path/to/sage/install"```

cmake --build .

```---



---## Пример 2: Рисование квадрата



## Basic Examples```cpp

#include <SAGE.h>

### Hello Window

class QuadExample : public SAGE::Application {

Create a basic SAGE application with a colored window.public:

    QuadExample() : Application("Quad Example") {}

```cpp    

#include <SAGE.h>    void OnInit() override {

        SAGE::Renderer::Init();

using namespace SAGE;    }

    

class HelloWindow : public Application {    void OnRender() override {

public:        SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);

    HelloWindow() : Application("Hello Window") {}        SAGE::Renderer::BeginScene();

    

    void OnInit() override {        SAGE::Renderer::SetMaterial(SAGE::MaterialLibrary::GetDefaultId());

        SAGE_INFO("Hello Window initialized!");

        Renderer::Init();        // Рисуем красный квадрат

    }        SAGE::QuadDesc redQuad;

            redQuad.position = { 100.0f, 100.0f };

    void OnRender() override {        redQuad.size = { 200.0f, 200.0f };

        // Clear with cornflower blue        redQuad.color = SAGE::Color::Red();

        Renderer::Clear(0.39f, 0.58f, 0.93f, 1.0f);        SAGE::Renderer::DrawQuad(redQuad);

    }

            // Рисуем зелёный квадрат

    void OnShutdown() override {        SAGE::QuadDesc greenQuad;

        Renderer::Shutdown();        greenQuad.position = { 350.0f, 150.0f };

    }        greenQuad.size = { 150.0f, 150.0f };

};        greenQuad.color = SAGE::Color::Green();

        SAGE::Renderer::DrawQuad(greenQuad);

SAGE::Application* SAGE::CreateApplication() {

    return new HelloWindow();        SAGE::Renderer::EndScene();

}    }

    

int main() {    void OnShutdown() override {

    auto* app = SAGE::CreateApplication();        SAGE::Renderer::Shutdown();

    app->Run();    }

    delete app;};

    return 0;

}SAGE::Application* SAGE::CreateApplication() {

```    return new QuadExample();

}

**What you'll learn:**```

- Creating a SAGE Application

- Application lifecycle (OnInit, OnRender, OnShutdown)---

- Basic renderer usage

## Пример 3: Обработка ввода

---

```cpp

### Drawing Shapes#include <SAGE.h>



Draw basic shapes using the Renderer API.class InputExample : public SAGE::Application {

private:

```cpp    SAGE::Vector2 playerPosition;

#include <SAGE.h>    float moveSpeed = 200.0f;

    

using namespace SAGE;public:

    InputExample() 

class DrawingShapes : public Application {        : Application("Input Example"),

public:          playerPosition(400.0f, 300.0f) {}

    DrawingShapes() : Application("Drawing Shapes") {}    

        void OnInit() override {

    void OnInit() override {        SAGE::Renderer::Init();

        Renderer::Init();    }

    }    

        void OnUpdate(float deltaTime) override {

    void OnRender() override {        // Движение WASD

        Renderer::Clear(0.1f, 0.1f, 0.1f, 1.0f);        if (SAGE::Input::IsKeyPressed(SAGE_KEY_W))

        Renderer::BeginScene();            playerPosition.y += moveSpeed * deltaTime;

                if (SAGE::Input::IsKeyPressed(SAGE_KEY_S))

        // Red square            playerPosition.y -= moveSpeed * deltaTime;

        QuadDesc redSquare;        if (SAGE::Input::IsKeyPressed(SAGE_KEY_A))

        redSquare.position = {200, 300};            playerPosition.x -= moveSpeed * deltaTime;

        redSquare.size = {100, 100};        if (SAGE::Input::IsKeyPressed(SAGE_KEY_D))

        redSquare.color = Color::Red();            playerPosition.x += moveSpeed * deltaTime;

        Renderer::DrawQuad(redSquare);        

                // Выход по ESC

        // Green circle (approximated with quad)        if (SAGE::Input::IsKeyPressed(SAGE_KEY_ESCAPE)) {

        QuadDesc greenCircle;            SAGE_INFO("Escape pressed - exiting!");

        greenCircle.position = {400, 300};        }

        greenCircle.size = {80, 80};        

        greenCircle.color = Color::Green();        // Проверка кликов мыши

        Renderer::DrawQuad(greenCircle);        if (SAGE::Input::IsMouseButtonPressed(SAGE_MOUSE_BUTTON_LEFT)) {

                    SAGE::Vector2 mousePos = SAGE::Input::GetMousePosition();

        // Blue rectangle            SAGE_INFO("Mouse clicked at: ({0}, {1})", mousePos.x, mousePos.y);

        QuadDesc blueRect;        }

        blueRect.position = {600, 300};    }

        blueRect.size = {150, 75};    

        blueRect.color = Color::Blue();    void OnRender() override {

        Renderer::DrawQuad(blueRect);        SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);

                SAGE::Renderer::BeginScene();

        Renderer::EndScene();

    }        SAGE::Renderer::SetMaterial(SAGE::MaterialLibrary::GetDefaultId());

    

    void OnShutdown() override {        // Рисуем игрока

        Renderer::Shutdown();        SAGE::QuadDesc playerQuad;

    }        playerQuad.position = { playerPosition.x, playerPosition.y };

};        playerQuad.size = { 50.0f, 50.0f };

        playerQuad.color = SAGE::Color::Cyan();

SAGE::Application* SAGE::CreateApplication() {        SAGE::Renderer::DrawQuad(playerQuad);

    return new DrawingShapes();

}        SAGE::Renderer::EndScene();

    }

int main() {    

    auto* app = SAGE::CreateApplication();    void OnShutdown() override {

    app->Run();        SAGE::Renderer::Shutdown();

    delete app;    }

    return 0;};

}

```SAGE::Application* SAGE::CreateApplication() {

    return new InputExample();

**What you'll learn:**}

- QuadDesc structure```

- Drawing multiple shapes

- Color system---

- Scene rendering flow

## Пример 4: Использование Vector2

---

```cpp

### Input Handling#include <SAGE.h>



Handle keyboard and mouse input.void VectorExamples() {

    // Создание векторов

```cpp    SAGE::Vector2 pos(100.0f, 200.0f);

#include <SAGE.h>    SAGE::Vector2 velocity(50.0f, -30.0f);

    

using namespace SAGE;    // Арифметические операции

    SAGE::Vector2 newPos = pos + velocity;

class InputDemo : public Application {    SAGE::Vector2 difference = pos - velocity;

public:    SAGE::Vector2 scaled = velocity * 2.0f;

    InputDemo() : Application("Input Handling") {}    

        // Длина вектора

    void OnInit() override {    float length = velocity.Length();

        Renderer::Init();    float lengthSquared = velocity.LengthSquared();

        m_Position = {400, 300};    

        m_Size = {50, 50};    // Нормализация

    }    SAGE::Vector2 direction = velocity.Normalized();

        

    void OnUpdate(float deltaTime) override {    // Скалярное произведение

        const float speed = 200.0f;    float dot = SAGE::Vector2::Dot(pos, velocity);

            

        // WASD movement    // Константные векторы

        if (Input::IsKeyPressed(SAGE_KEY_W)) m_Position.y -= speed * deltaTime;    SAGE::Vector2 zero = SAGE::Vector2::Zero();

        if (Input::IsKeyPressed(SAGE_KEY_S)) m_Position.y += speed * deltaTime;    SAGE::Vector2 up = SAGE::Vector2::Up();

        if (Input::IsKeyPressed(SAGE_KEY_A)) m_Position.x -= speed * deltaTime;    SAGE::Vector2 right = SAGE::Vector2::Right();

        if (Input::IsKeyPressed(SAGE_KEY_D)) m_Position.x += speed * deltaTime;}

        ```

        // Mouse click to teleport

        if (Input::IsMouseButtonJustPressed(SAGE_MOUSE_BUTTON_LEFT)) {---

            m_Position = Input::GetMousePosition();

        }## Пример 5: Спрайты и текстуры

        

        // Space to grow, Shift to shrink```cpp

        if (Input::IsKeyPressed(SAGE_KEY_SPACE)) {#include <SAGE.h>

            m_Size.x += 50.0f * deltaTime;

            m_Size.y += 50.0f * deltaTime;class SpriteExample : public SAGE::Application {

        }private:

        if (Input::IsKeyPressed(SAGE_KEY_LEFT_SHIFT)) {    SAGE::Ref<SAGE::Texture> texture;

            m_Size.x = std::max(10.0f, m_Size.x - 50.0f * deltaTime);    SAGE::Scope<SAGE::Sprite> sprite;

            m_Size.y = std::max(10.0f, m_Size.y - 50.0f * deltaTime);    

        }public:

            SpriteExample() : Application("Sprite Example") {}

        // ESC to quit    

        if (Input::IsKeyPressed(SAGE_KEY_ESCAPE)) {    void OnInit() override {

            Close();        SAGE::Renderer::Init();

        }        

    }        // Создание текстуры (пока без загрузки файлов)

            texture = SAGE::CreateRef<SAGE::Texture>(256, 256);

    void OnRender() override {        

        Renderer::Clear(0.2f, 0.2f, 0.2f, 1.0f);        // Создание спрайта

        Renderer::BeginScene();        sprite = SAGE::CreateScope<SAGE::Sprite>(texture);

                sprite->SetPosition(SAGE::Vector2(200.0f, 200.0f));

        QuadDesc quad;        sprite->SetSize(SAGE::Vector2(128.0f, 128.0f));

        quad.position = m_Position;        sprite->SetColor(1.0f, 0.5f, 0.0f, 1.0f); // Оранжевый

        quad.size = m_Size;    }

        quad.color = Color::Cyan();    

        Renderer::DrawQuad(quad);    void OnRender() override {

                SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);

        Renderer::EndScene();        SAGE::Renderer::BeginScene();

    }        

            sprite->Draw();

    void OnShutdown() override {        

        Renderer::Shutdown();        SAGE::Renderer::EndScene();

    }    }

        

private:    void OnShutdown() override {

    Vector2 m_Position;        SAGE::Renderer::Shutdown();

    Vector2 m_Size;    }

};};



SAGE::Application* SAGE::CreateApplication() {SAGE::Application* SAGE::CreateApplication() {

    return new InputDemo();    return new SpriteExample();

}}

```

int main() {

    auto* app = SAGE::CreateApplication();---

    app->Run();

    delete app;## Пример 6: Простая игра - Pong

    return 0;

}```cpp

```#include <SAGE.h>



**What you'll learn:**class PongGame : public SAGE::Application {

- Keyboard input (IsKeyPressed, IsKeyJustPressed)private:

- Mouse input (position, buttons)    SAGE::Vector2 paddleLeft;

- Delta time for frame-independent movement    SAGE::Vector2 paddleRight;

- Application::Close() method    SAGE::Vector2 ball;

    SAGE::Vector2 ballVelocity;

---    

    const float paddleSpeed = 400.0f;

## Intermediate Examples    const float paddleWidth = 20.0f;

    const float paddleHeight = 100.0f;

### Simple Game    const float ballSize = 15.0f;

    

**Location:** `Examples/SimpleGame/`public:

    PongGame() 

A complete simple game with player movement.        : Application("Pong Game"),

          paddleLeft(50.0f, 300.0f),

**Features:**          paddleRight(1230.0f, 300.0f),

- WASD movement          ball(640.0f, 360.0f),

- Boundary checking          ballVelocity(200.0f, 150.0f) {}

- ESC to quit    

- Colored player and ground    void OnInit() override {

        SAGE::Renderer::Init();

See [Examples/SimpleGame/main.cpp](Examples/SimpleGame/main.cpp) for full source code.    }

    

---    void OnUpdate(float deltaTime) override {

        // Управление левой ракеткой (W/S)

## Tips & Best Practices        if (SAGE::Input::IsKeyPressed(SAGE_KEY_W))

            paddleLeft.y += paddleSpeed * deltaTime;

### Performance        if (SAGE::Input::IsKeyPressed(SAGE_KEY_S))

            paddleLeft.y -= paddleSpeed * deltaTime;

1. **Batch Rendering**: Group draw calls to minimize state changes        

2. **Asset Preloading**: Load assets during loading screens        // Управление правой ракеткой (стрелки)

3. **Object Pooling**: Reuse GameObjects instead of creating/destroying        if (SAGE::Input::IsKeyPressed(SAGE_KEY_UP))

4. **Profiler**: Use the built-in profiler to identify bottlenecks            paddleRight.y += paddleSpeed * deltaTime;

        if (SAGE::Input::IsKeyPressed(SAGE_KEY_DOWN))

### Code Organization            paddleRight.y -= paddleSpeed * deltaTime;

        

```        // Движение мяча

MyGame/        ball += ballVelocity * deltaTime;

├── src/        

│   ├── Game.cpp             # Main application        // Отскок от верхней и нижней границы

│   ├── Player.cpp           # Player logic        if (ball.y > 720.0f || ball.y < 0.0f)

│   ├── Enemy.cpp            # Enemy logic            ballVelocity.y = -ballVelocity.y;

│   ├── Level.cpp            # Level management        

│   └── UI/                  # UI components        // Проверка столкновения с ракетками (упрощённая)

├── assets/        if (ball.x < paddleLeft.x + paddleWidth &&

│   ├── textures/            ball.y > paddleLeft.y && ball.y < paddleLeft.y + paddleHeight)

│   ├── sounds/            ballVelocity.x = -ballVelocity.x;

│   └── shaders/        

└── CMakeLists.txt        if (ball.x > paddleRight.x &&

```            ball.y > paddleRight.y && ball.y < paddleRight.y + paddleHeight)

            ballVelocity.x = -ballVelocity.x;

### Common Patterns        

        // Сброс мяча если вышел за границы

**Singleton Systems:**        if (ball.x < 0.0f || ball.x > 1280.0f) {

```cpp            ball = SAGE::Vector2(640.0f, 360.0f);

auto& physics = SystemManager::GetPhysicsSystem();            ballVelocity = SAGE::Vector2(200.0f, 150.0f);

auto& assets = SystemManager::GetAssetManager();        }

auto& profiler = SystemManager::GetProfiler();    }

```    

    void OnRender() override {

**Component Pattern:**        SAGE::Renderer::Clear(0.0f, 0.0f, 0.0f);

```cpp        SAGE::Renderer::BeginScene();

auto* obj = CreateGameObject("Entity");

auto* collider = obj->AddComponent<BoxCollider>();        SAGE::Renderer::SetMaterial(SAGE::MaterialLibrary::GetDefaultId());

auto* sprite = obj->AddComponent<Sprite>();

```        SAGE::QuadDesc quad;

        quad.size = { paddleWidth, paddleHeight };

---        quad.color = SAGE::Color::White();



## Troubleshooting        // Левая ракетка

        quad.position = { paddleLeft.x, paddleLeft.y };

### Build Issues        SAGE::Renderer::DrawQuad(quad);



**Error: Cannot find SAGE package**        // Правая ракетка

```bash        quad.position = { paddleRight.x, paddleRight.y };

# Set CMAKE_PREFIX_PATH to installation directory        SAGE::Renderer::DrawQuad(quad);

cmake .. -DCMAKE_PREFIX_PATH="path/to/sage/install"

```        // Мяч

        quad.position = { ball.x, ball.y };

**Error: OpenGL headers not found**        quad.size = { ballSize, ballSize };

```bash        quad.color = SAGE::Color::Yellow();

# Ensure graphics drivers are up to date        SAGE::Renderer::DrawQuad(quad);

# Install OpenGL development libraries

```        SAGE::Renderer::EndScene();

    }

### Runtime Issues    

    void OnShutdown() override {

**Black Screen**        SAGE::Renderer::Shutdown();

- Check if `Renderer::Init()` is called in `OnInit()`    }

- Verify `Renderer::BeginScene()` and `Renderer::EndScene()` are called};

- Ensure clear color is set

SAGE::Application* SAGE::CreateApplication() {

**No Input Response**    return new PongGame();

- Verify window has focus}

- Check key codes match (use SAGE_KEY_* constants)```



------



## Additional Resources## Советы по использованию



- [Installation Guide](INSTALL.md)### 1. Организация кода

- [Quick Start](QUICKSTART.md)Рекомендуется разделять игровую логику на отдельные классы и файлы:

- [GitHub Repository](https://github.com/AGamesStudios/SAGE-Engine)- `Game.h/cpp` - основной класс игры

- `Player.h/cpp` - логика игрока

---- `Enemy.h/cpp` - логика врагов

- и т.д.

<div align="center">

### 2. Управление ресурсами

**Happy Coding with SAGE Engine! 🎮**Используйте умные указатели для управления памятью:

```cpp

[⬆ Back to top](#sage-engine-examples--tutorials)SAGE::Ref<Texture> texture = SAGE::CreateRef<Texture>("path/to/texture.png");

SAGE::Scope<Sprite> sprite = SAGE::CreateScope<Sprite>(texture);

</div>```


### 3. Производительность
- Минимизируйте количество вызовов DrawQuad
- Используйте батчинг для множества объектов (будет в версии 1.1.0)
- Избегайте создания новых объектов в OnUpdate и OnRender

### 4. Отладка
Используйте макросы логирования:
```cpp
SAGE_INFO("Player position: ({0}, {1})", player.x, player.y);
SAGE_WARNING("Low health: {0}", health);
SAGE_ERROR("Failed to load texture!");
```

---

Больше примеров будет добавлено по мере развития движка!
