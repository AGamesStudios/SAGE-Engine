# 🎮 SAGE Engine - Руководство по разработке игр

## 📋 Содержание
1. [Быстрый старт](#быстрый-старт)
2. [Создание простой игры](#создание-простой-игры)
3. [Примеры игр](#примеры-игр)
4. [Архитектура игры](#архитектура-игры)
5. [Best Practices](#best-practices)

---

## 🚀 Быстрый старт

### Минимальная игра (100 строк)

```cpp
#include <SAGE/SAGE.h>
using namespace SAGE;

class SimpleGame : public Game {
    std::shared_ptr<Texture> m_PlayerTexture;
    Vector2 m_PlayerPos{400, 300};
    float m_Speed = 200.0f;

public:
    SimpleGame() : Game({.window = {.title = "My Game", .width = 800, .height = 600}}) {}

    void OnGameInit() override {
        m_PlayerTexture = Texture::Create("assets/player.png");
    }

    void OnGameUpdate(float deltaTime) override {
        // Управление
        if (Input::IsKeyPressed(Key::W)) m_PlayerPos.y -= m_Speed * deltaTime;
        if (Input::IsKeyPressed(Key::S)) m_PlayerPos.y += m_Speed * deltaTime;
        if (Input::IsKeyPressed(Key::A)) m_PlayerPos.x -= m_Speed * deltaTime;
        if (Input::IsKeyPressed(Key::D)) m_PlayerPos.x += m_Speed * deltaTime;
    }

    void OnGameRender() override {
        SpriteRenderer::DrawSprite(m_PlayerTexture, m_PlayerPos, {64, 64});
    }
};

int main() {
    SimpleGame game;
    game.Run();
    return 0;
}
```

---

## 🎯 Создание простой игры

### Структура проекта

```
MyGame/
├── src/
│   ├── main.cpp              # Точка входа
│   ├── Game.cpp/h            # Главный класс игры
│   ├── Player.cpp/h          # Игрок
│   ├── Enemy.cpp/h           # Враги
│   └── GameScene.cpp/h       # Игровая сцена
├── assets/
│   ├── textures/             # Текстуры
│   ├── sounds/               # Звуки
│   └── fonts/                # Шрифты
└── CMakeLists.txt            # Конфигурация сборки
```

### 1. Создание игрока

```cpp
// Player.h
#pragma once
#include <SAGE/SAGE.h>

class Player {
public:
    Player(SAGE::Vector2 position);
    
    void Update(float deltaTime);
    void Render();
    
    SAGE::Vector2 GetPosition() const { return m_Position; }
    SAGE::Rect GetBounds() const { return {m_Position.x, m_Position.y, 64, 64}; }
    
    void TakeDamage(int damage);
    bool IsAlive() const { return m_Health > 0; }

private:
    SAGE::Vector2 m_Position;
    SAGE::Vector2 m_Velocity;
    std::shared_ptr<SAGE::Texture> m_Texture;
    
    int m_Health = 100;
    float m_Speed = 250.0f;
    bool m_IsFacingRight = true;
};

// Player.cpp
#include "Player.h"
using namespace SAGE;

Player::Player(Vector2 position) : m_Position(position) {
    m_Texture = Texture::Create("assets/textures/player.png");
}

void Player::Update(float deltaTime) {
    m_Velocity = {0, 0};
    
    // Движение
    if (Input::IsKeyPressed(Key::W)) m_Velocity.y = -m_Speed;
    if (Input::IsKeyPressed(Key::S)) m_Velocity.y = m_Speed;
    if (Input::IsKeyPressed(Key::A)) {
        m_Velocity.x = -m_Speed;
        m_IsFacingRight = false;
    }
    if (Input::IsKeyPressed(Key::D)) {
        m_Velocity.x = m_Speed;
        m_IsFacingRight = true;
    }
    
    // Применяем скорость
    m_Position = m_Position + m_Velocity * deltaTime;
    
    // Ограничения по экрану
    m_Position.x = std::clamp(m_Position.x, 0.0f, 800.0f - 64.0f);
    m_Position.y = std::clamp(m_Position.y, 0.0f, 600.0f - 64.0f);
}

void Player::Render() {
    if (!IsAlive()) return;
    
    SpriteRenderer::DrawSprite(
        m_Texture, 
        m_Position, 
        {64, 64},
        0.0f,
        m_IsFacingRight ? 1.0f : -1.0f  // Отражение по горизонтали
    );
}

void Player::TakeDamage(int damage) {
    m_Health -= damage;
    if (m_Health < 0) m_Health = 0;
}
```

### 2. Система врагов

```cpp
// Enemy.h
#pragma once
#include <SAGE/SAGE.h>

class Enemy {
public:
    Enemy(SAGE::Vector2 position, SAGE::Vector2 target);
    
    void Update(float deltaTime);
    void Render();
    
    SAGE::Rect GetBounds() const { return {m_Position.x, m_Position.y, 48, 48}; }
    bool IsAlive() const { return m_Health > 0; }
    
    void TakeDamage(int damage) { m_Health -= damage; }

private:
    SAGE::Vector2 m_Position;
    SAGE::Vector2 m_Target;
    std::shared_ptr<SAGE::Texture> m_Texture;
    
    int m_Health = 50;
    float m_Speed = 100.0f;
};

// Enemy.cpp
#include "Enemy.h"
using namespace SAGE;

Enemy::Enemy(Vector2 position, Vector2 target) 
    : m_Position(position), m_Target(target) {
    m_Texture = Texture::Create("assets/textures/enemy.png");
}

void Enemy::Update(float deltaTime) {
    // Движение к цели
    Vector2 direction = m_Target - m_Position;
    float distance = direction.Length();
    
    if (distance > 5.0f) {
        direction = direction.Normalized();
        m_Position = m_Position + direction * m_Speed * deltaTime;
    }
}

void Enemy::Render() {
    if (!IsAlive()) return;
    SpriteRenderer::DrawSprite(m_Texture, m_Position, {48, 48});
}
```

### 3. Игровая сцена

```cpp
// GameScene.h
#pragma once
#include <SAGE/SAGE.h>
#include "Player.h"
#include "Enemy.h"

class GameScene : public SAGE::Scene {
public:
    void OnSceneEnter() override;
    void OnSceneUpdate(float deltaTime) override;
    void OnSceneRender() override;

private:
    void SpawnEnemy();
    void CheckCollisions();
    void UpdateUI();

    std::unique_ptr<Player> m_Player;
    std::vector<std::unique_ptr<Enemy>> m_Enemies;
    
    SAGE::QuadTree<Enemy*> m_QuadTree;
    float m_SpawnTimer = 0.0f;
    int m_Score = 0;
};

// GameScene.cpp
#include "GameScene.h"
using namespace SAGE;

void GameScene::OnSceneEnter() {
    m_Player = std::make_unique<Player>(Vector2{400, 300});
    m_QuadTree = QuadTree<Enemy*>(Rect{0, 0, 800, 600}, 10, 5);
    
    SAGE_INFO("Game Scene started!");
}

void GameScene::OnSceneUpdate(float deltaTime) {
    if (!m_Player->IsAlive()) {
        SAGE_WARN("Game Over! Score: {}", m_Score);
        return;
    }
    
    // Обновление игрока
    m_Player->Update(deltaTime);
    
    // Спавн врагов
    m_SpawnTimer += deltaTime;
    if (m_SpawnTimer > 2.0f) {
        SpawnEnemy();
        m_SpawnTimer = 0.0f;
    }
    
    // Обновление QuadTree
    m_QuadTree.Clear();
    for (auto& enemy : m_Enemies) {
        if (enemy->IsAlive()) {
            m_QuadTree.Insert(enemy->GetBounds(), enemy.get());
        }
    }
    
    // Обновление врагов
    for (auto& enemy : m_Enemies) {
        if (enemy->IsAlive()) {
            enemy->Update(deltaTime);
        }
    }
    
    // Проверка коллизий
    CheckCollisions();
    
    // Удаление мертвых врагов
    m_Enemies.erase(
        std::remove_if(m_Enemies.begin(), m_Enemies.end(),
            [](const auto& e) { return !e->IsAlive(); }),
        m_Enemies.end()
    );
}

void GameScene::OnSceneRender() {
    // Фон
    SpriteRenderer::DrawRect({0, 0}, {800, 600}, Color{0.1f, 0.1f, 0.15f, 1.0f});
    
    // Враги
    for (auto& enemy : m_Enemies) {
        enemy->Render();
    }
    
    // Игрок
    m_Player->Render();
    
    // UI
    UpdateUI();
}

void GameScene::SpawnEnemy() {
    float x = (rand() % 800);
    float y = (rand() % 600);
    m_Enemies.push_back(
        std::make_unique<Enemy>(Vector2{x, y}, m_Player->GetPosition())
    );
}

void GameScene::CheckCollisions() {
    // Проверка коллизии игрока с врагами
    auto nearbyEnemies = m_QuadTree.Query(m_Player->GetBounds());
    
    for (auto* enemy : nearbyEnemies) {
        if (enemy->GetBounds().Intersects(m_Player->GetBounds())) {
            m_Player->TakeDamage(10);
            enemy->TakeDamage(100); // Убиваем врага
            m_Score += 10;
        }
    }
}

void GameScene::UpdateUI() {
    // Отображение счета
    // TODO: Добавить текст когда будет шрифтовая система
}
```

### 4. Главный файл

```cpp
// main.cpp
#include <SAGE/SAGE.h>
#include "GameScene.h"

using namespace SAGE;

class MyGame : public Game {
public:
    MyGame() : Game({
        .window = {
            .title = "SAGE Game Demo",
            .width = 800,
            .height = 600,
            .vsync = true
        }
    }) {}

    void OnGameInit() override {
        // Создаем и добавляем игровую сцену
        auto gameScene = std::make_shared<GameScene>();
        SceneManager::PushScene(gameScene);
        
        SAGE_INFO("Game initialized!");
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

---

## 📚 Примеры игр

### 1. Space Shooter (Космический шутер)

См. `Examples/SpaceShooter/`

**Возможности:**
- Управление космическим кораблем
- Стрельба по врагам
- Система волн врагов
- Счет и жизни
- Эффекты частиц при взрывах

### 2. Platformer (Платформер)

См. `Examples/Platformer/`

**Возможности:**
- Физика прыжков с Box2D
- Коллизии с платформами
- Сбор монет
- Анимация персонажа
- Tilemap уровни

### 3. Tower Defense (Защита башни)

См. `Examples/TowerDefense/`

**Возможности:**
- Размещение башен
- Волны врагов с pathfinding
- Система улучшений
- Экономика (деньги/ресурсы)
- Несколько типов башен

---

## 🏗️ Архитектура игры

### Рекомендуемая структура

```
Game
├── Scenes (Сцены)
│   ├── MenuScene         # Главное меню
│   ├── GameScene         # Игровой процесс
│   ├── PauseScene        # Пауза
│   └── GameOverScene     # Конец игры
│
├── Entities (Сущности)
│   ├── Player           # Игрок
│   ├── Enemy            # Враги
│   ├── Bullet           # Пули
│   └── Pickup           # Подбираемые предметы
│
├── Systems (Системы)
│   ├── CollisionSystem  # Обработка коллизий
│   ├── SpawnSystem      # Спавн объектов
│   ├── AISystem         # Искусственный интеллект
│   └── UISystem         # Интерфейс
│
└── Managers (Менеджеры)
    ├── GameState        # Состояние игры
    ├── ScoreManager     # Система очков
    └── SaveManager      # Сохранения
```

### Жизненный цикл игры

```cpp
Game::Run()
  → OnGameInit()           // 1 раз при запуске
  → while (running) {
      OnGameUpdate(dt)     // Каждый кадр - логика
      OnGameRender()       // Каждый кадр - отрисовка
    }
  → OnGameShutdown()       // 1 раз при выходе
```

---

## ✅ Best Practices

### 1. Управление ресурсами

```cpp
// ✅ ХОРОШО - автоматическая загрузка и кэширование
class GameAssets {
public:
    static void LoadAll() {
        ResourceManager::Load<Texture>("player", "assets/player.png");
        ResourceManager::Load<Texture>("enemy", "assets/enemy.png");
        ResourceManager::Load<Sound>("shoot", "assets/shoot.wav");
    }
    
    static std::shared_ptr<Texture> GetPlayerTexture() {
        return ResourceManager::Get<Texture>("player");
    }
};

// ❌ ПЛОХО - загрузка в каждом кадре
void Render() {
    auto tex = Texture::Create("assets/player.png"); // Медленно!
}
```

### 2. Оптимизация коллизий

```cpp
// ✅ ХОРОШО - используем QuadTree для пространственного разделения
QuadTree<Enemy*> m_QuadTree(Rect{0, 0, 1920, 1080}, 10, 5);

void Update() {
    m_QuadTree.Clear();
    for (auto& enemy : m_Enemies) {
        m_QuadTree.Insert(enemy->GetBounds(), enemy.get());
    }
    
    // Только близкие объекты
    auto nearby = m_QuadTree.Query(player->GetBounds());
}

// ❌ ПЛОХО - проверка всех со всеми O(n²)
for (auto& a : objects) {
    for (auto& b : objects) {
        if (a != b && a.Intersects(b)) { }
    }
}
```

### 3. Профилирование

```cpp
void Update(float deltaTime) {
    SAGE_PROFILE_FUNCTION(); // Автоматическое измерение
    
    {
        SAGE_PROFILE_SCOPE("Physics");
        UpdatePhysics(deltaTime);
    }
    
    {
        SAGE_PROFILE_SCOPE("AI");
        UpdateAI(deltaTime);
    }
}

// Просмотр результатов
auto results = Profiler::Get().GetResults();
for (auto& r : results) {
    SAGE_INFO("{}: {:.2f}ms", r.name, r.averageMs);
}
```

### 4. Использование таймеров

```cpp
// ✅ ХОРОШО - используем Timer для задержек
class PowerUp {
    void Activate() {
        m_IsActive = true;
        
        // Автоматически деактивируем через 5 секунд
        Timer::SetTimeout([this]() {
            m_IsActive = false;
        }, 5.0f);
    }
};

// ❌ ПЛОХО - ручной подсчет времени
float m_PowerUpTimer = 5.0f;
void Update(float dt) {
    m_PowerUpTimer -= dt;
    if (m_PowerUpTimer <= 0) {
        m_IsActive = false;
    }
}
```

### 5. Эффекты частиц

```cpp
// Взрыв при уничтожении врага
void Enemy::OnDestroy() {
    auto explosion = std::make_shared<ParticleEmitter>(200);
    explosion->SetConfig(ParticleEmitter::CreateExplosionEmitter());
    explosion->SetPosition(m_Position);
    explosion->Start();
    explosion->Burst(50);
    
    // Добавляем в систему частиц сцены
    m_Scene->AddParticleEmitter(explosion);
}
```

---

## 🎯 Готовые системы движка

### Доступные компоненты:

- **SpriteRenderer** - Рендеринг спрайтов
- **ParticleEmitter** - Система частиц
- **Camera2D** - Камера с эффектами
- **Tilemap** - Сетка тайлов
- **Animator** - Покадровая анимация
- **QuadTree** - Пространственное разделение
- **Profiler** - Производительность
- **Timer** - Задержки и интервалы
- **Input** - Клавиатура/мышь/геймпад
- **Audio** - Звуки и музыка
- **SceneManager** - Управление сценами
- **ResourceManager** - Загрузка ресурсов

### Полный пример интеграции:

```cpp
class FullFeaturedGame : public Game {
    Camera2D m_Camera;
    ParticleEmitter m_Rain;
    Tilemap m_Map;
    
public:
    FullFeaturedGame() : Game({.window = {.title = "Full Game"}}) {}
    
    void OnGameInit() override {
        // Камера
        m_Camera.SetBounds({0, 0, 3200, 2400});
        m_Camera.SetPosition({400, 300});
        
        // Tilemap
        m_Map.LoadFromFile("assets/level1.tmx");
        
        // Частицы дождя
        m_Rain.SetConfig(ParticleEmitter::CreateRainEmitter());
        m_Rain.Start();
        
        // Физика
        Physics::Initialize({0, 9.8f}); // Гравитация
    }
    
    void OnGameUpdate(float dt) override {
        SAGE_PROFILE_FUNCTION();
        
        m_Rain.Update(dt);
        Physics::Step(dt);
        
        // Камера следует за игроком
        m_Camera.Follow(m_Player->GetPosition(), dt);
    }
    
    void OnGameRender() override {
        m_Camera.Begin();
        
        m_Map.Render();
        m_Player->Render();
        m_Rain.Render();
        
        m_Camera.End();
    }
};
```

---

## 📝 Следующие шаги

1. **Изучите примеры** в папке `Examples/`
2. **Прочитайте API документацию** в `docs/API.md`
3. **Создайте свою игру** используя шаблоны выше
4. **Задавайте вопросы** в [Issues](https://github.com/AGamesStudios/SAGE-Engine/issues)

**Удачи в разработке! 🚀**
