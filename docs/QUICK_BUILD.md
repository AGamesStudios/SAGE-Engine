# SAGE Engine - Quick Build Guide

## 🚀 Быстрая компиляция игр (одна команда!)

### Windows PowerShell:
```powershell
.\build-game.ps1
```

Или для конкретной игры:
```powershell
.\build-game.ps1 TestGame
.\build-game.ps1 FlappyBird
.\build-game.ps1 SpaceShooter
```

### Windows CMD:
```cmd
build-game.bat
```

Или:
```cmd
build-game.bat TestGame
build-game.bat FlappyBird Release
```

### Что делает скрипт:
1. ✅ Автоматически настраивает CMake (первый раз)
2. ✅ Компилирует движок SAGE_Engine
3. ✅ Компилирует вашу игру
4. ✅ Предлагает сразу запустить

## 📦 Создание новой игры

### Шаг 1: Создайте файл игры
Создайте файл `Examples/MyGame.cpp`:

```cpp
#include <SAGE/SAGE.h>

using namespace SAGE;

class MyGameScene : public Scene {
public:
    MyGameScene() : Scene("MyGameScene") {}
    
    void OnEnter(const TransitionContext&) override {
        SAGE_INFO("My Game Started!");
    }
    
    void OnUpdate(float deltaTime) override {
        // Ваша логика игры
    }
    
    void OnRender() override {
        Renderer::Clear(Color{0.2f, 0.3f, 0.5f, 1.0f});
        // Ваш рендеринг
    }
    
    void OnExit() override {
        SAGE_INFO("Game closed");
    }
};

class MyGame : public Game {
public:
    MyGame() : Game({
        .window = {
            .title = "My Game",
            .width = 800,
            .height = 600,
            .vsync = true
        }
    }) {}
    
    void OnGameInit() override {
        Matrix3 projection = Matrix3::Ortho(0.0f, 800.0f, 600.0f, 0.0f);
        Renderer::SetProjectionMatrix(projection);
        
        SceneManager::Get().RegisterScene<MyGameScene>("MyGameScene");
        SceneManager::Get().PushScene("MyGameScene");
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

### Шаг 2: Добавьте в CMakeLists.txt

Откройте `Examples/CMakeLists.txt` и добавьте:

```cmake
# My Game
add_executable(MyGame Examples/MyGame.cpp)
target_link_libraries(MyGame PRIVATE SAGE_Engine)
set_target_properties(MyGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)
```

### Шаг 3: Скомпилируйте и запустите!

```powershell
.\build-game.ps1 MyGame
```

Готово! 🎮

## 🎨 Полезные функции

### Рендеринг:
```cpp
// Очистка экрана цветом
Renderer::Clear(Color{0.2f, 0.3f, 0.5f, 1.0f});

// Рисование квада
Renderer::DrawQuad(position, size, color);

// Рисование частиц
Renderer::DrawParticle(position, size, color, rotation);

// Текст
TextRenderer::DrawText("Hello", position, color);
TextRenderer::DrawTextAligned("Center", position, TextAlign::Center, color);
```

### Управление:
```cpp
// Проверка клавиш (удержание)
if (Input::IsKeyDown(KeyCode::A)) { /* код */ }
if (Input::IsKeyDown(KeyCode::Space)) { /* прыжок */ }

// Проверка нажатия (один раз)
if (Input::IsKeyPressed(KeyCode::Escape)) { /* пауза */ }
```

### Частицы:
```cpp
ParticleEmitter particles;
auto config = ParticleEmitter::CreateExplosionEmitter();
config.position = {400, 300};
particles.SetConfig(config);
particles.Start();

// В OnUpdate:
particles.Update(deltaTime);

// В OnRender:
for (const auto& p : particles.GetParticles()) {
    if (p.active) {
        Renderer::DrawParticle(p.position, p.size, p.color, p.rotation);
    }
}
```

## ⚡ Примеры готовых игр:

- **TestGame** - платформер с физикой
- **FlappyBird** - аркада
- **SpaceShooter** - космический шутер (если создан)

Запуск:
```powershell
.\build-game.ps1 TestGame
.\build-game.ps1 FlappyBird
```

## 🔧 Дополнительные опции

### Debug сборка:
```powershell
.\build-game.ps1 TestGame Debug
```

### Только сборка (без запуска):
В скрипте нажмите `n` когда спросит про запуск.

### Пересборка с нуля:
```powershell
Remove-Item -Recurse -Force build
.\build-game.ps1
```

---

**Теперь создание игр на SAGE Engine - это просто одна команда!** 🚀
