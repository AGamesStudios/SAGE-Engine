# Быстрый старт SAGE Engine

## Автоматическая установка (рекомендуется)

Запустите PowerShell скрипт для автоматической загрузки зависимостей:

```powershell
.\setup.ps1
```

Затем вручную скачайте GLAD (см. инструкции в выводе скрипта).

## Ручная установка

См. подробную инструкцию в файле [SETUP.md](SETUP.md)

## Быстрая сборка и запуск

```powershell
# Сборка
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Запуск примера
.\Sandbox\Release\Sandbox.exe
```

## Возможности движка

### Текущая версия (v1.0.0)

✅ **Реализовано:**
- Система окон (GLFW)
- Контекст OpenGL 3.3+
- Базовый рендерер
- Система ввода (клавиатура, мышь)
- Математические утилиты (Vector2)
- Система логирования
- Шейдеры
- Текстуры (базовая поддержка)
- Спрайты

🔨 **В разработке:**
- Батчинг рендеринга
- Система сцен и сущностей (ECS)
- Физический движок 2D
- Аудио система
- Поддержка загрузки изображений (stb_image)
- Система анимации
- Particle system

## Пример использования

### Базовый пример игры

```cpp
#include <SAGE.h>

class MyGame : public SAGE::Application {
public:
    MyGame() : Application("My Game") {}
    
    void OnInit() override {
        SAGE::Renderer::Init();
    }
    
    void OnUpdate(float deltaTime) override {
        if (SAGE::Input::IsKeyPressed(SAGE_KEY_W)) {
            // Движение вверх
        }
    }
    
    void OnRender() override {
        SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);
        SAGE::Renderer::BeginScene();
        
        // Рисование
        SAGE::Renderer::SetMaterial(SAGE::MaterialLibrary::GetDefaultId());

        SAGE::QuadDesc quad;
        quad.position = { 100.0f, 100.0f };
        quad.size = { 50.0f, 50.0f };
        quad.color = SAGE::Color::Red();
        SAGE::Renderer::DrawQuad(quad);
        
        SAGE::Renderer::EndScene();
    }
    
    void OnShutdown() override {
        SAGE::Renderer::Shutdown();
    }
};

SAGE::Application* SAGE::CreateApplication() {
    return new MyGame();
}

int main() {
    auto app = SAGE::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
```

---

## Новые возможности (v1.1.0-alpha)

### AssetManager - Управление ресурсами

AssetManager централизованно управляет всеми ресурсами (текстуры, звуки, шейдеры, шрифты) с автоматическим отслеживанием памяти и поддержкой асинхронной загрузки.

#### Базовая загрузка ресурсов

```cpp
#include <SAGE.h>

void LoadGameAssets() {
    // Инициализация AssetManager
    SAGE::AssetManager::Init();
    SAGE::AssetManager::SetAssetDirectory("Assets");

    // Синхронная загрузка текстуры
    auto playerTexture = SAGE::AssetManager::LoadTexture("player.png");
    if (playerTexture) {
        SAGE_INFO("Текстура загружена: {}x{}", playerTexture->GetWidth(), playerTexture->GetHeight());
    }

    // Загрузка звука
    auto jumpSound = SAGE::AssetManager::LoadSound("jump.wav");
    
    // Загрузка шейдера
    auto customShader = SAGE::AssetManager::LoadShader("custom_shader.glsl");

    // Проверка наличия ресурса
    if (SAGE::AssetManager::HasTexture("player.png")) {
        SAGE_INFO("Текстура уже загружена");
    }

    // Получение уже загруженного ресурса
    auto texture = SAGE::AssetManager::GetTexture("player.png");

    // Выгрузка ресурса
    SAGE::AssetManager::UnloadTexture("player.png");

    // Статистика
    size_t textureCount = SAGE::AssetManager::GetAssetCount(SAGE::AssetType::Texture);
    size_t memoryUsed = SAGE::AssetManager::GetTotalMemoryUsage();
    SAGE_INFO("Загружено текстур: {}, Использовано памяти: {} MB", 
              textureCount, memoryUsed / (1024 * 1024));
}
```

#### Асинхронная загрузка (для больших ресурсов)

```cpp
class GameLoadingScreen : public SAGE::Application {
private:
    bool m_AssetsLoaded = false;
    int m_LoadedCount = 0;
    const int m_TotalAssets = 5;

public:
    void OnInit() override {
        SAGE::AssetManager::Init();
        SAGE::AssetManager::SetAssetDirectory("Assets");

        // Асинхронная загрузка с callback
        SAGE::AssetManager::LoadTextureAsync("background.png", 
            [this](SAGE::Ref<SAGE::Texture> texture) {
                if (texture) {
                    SAGE_INFO("Background загружен асинхронно");
                    m_LoadedCount++;
                }
            });

        SAGE::AssetManager::LoadTextureAsync("tileset.png",
            [this](SAGE::Ref<SAGE::Texture> texture) {
                m_LoadedCount++;
            });

        SAGE::AssetManager::LoadSoundAsync("music.mp3",
            [this](SAGE::Ref<SAGE::Sound> sound) {
                m_LoadedCount++;
            });

        SAGE::AssetManager::LoadTextureAsync("player.png",
            [this](SAGE::Ref<SAGE::Texture> texture) {
                m_LoadedCount++;
            });

        SAGE::AssetManager::LoadTextureAsync("enemies.png",
            [this](SAGE::Ref<SAGE::Texture> texture) {
                m_LoadedCount++;
                // Все ресурсы загружены
                if (m_LoadedCount >= m_TotalAssets) {
                    m_AssetsLoaded = true;
                    SAGE_INFO("Все ресурсы загружены!");
                }
            });
    }

    void OnUpdate(float deltaTime) override {
        // КРИТИЧНО: вызывайте ProcessAsyncLoads() каждый кадр
        // для завершения асинхронных загрузок в главном потоке (OpenGL контекст)
        SAGE::AssetManager::ProcessAsyncLoads();

        if (m_AssetsLoaded) {
            // Переход к игре
            // ...
        }
    }

    void OnRender() override {
        SAGE::Renderer::Clear(0.0f, 0.0f, 0.0f);
        
        // Экран загрузки
        if (!m_AssetsLoaded) {
            float progress = (float)m_LoadedCount / m_TotalAssets;
            // Рисуем прогресс-бар
            SAGE::Renderer::BeginScene();
            
            SAGE::QuadDesc progressBar;
            progressBar.position = { 100.0f, 300.0f };
            progressBar.size = { 600.0f * progress, 30.0f };
            progressBar.color = SAGE::Color::Green();
            SAGE::Renderer::DrawQuad(progressBar);
            
            SAGE::Renderer::EndScene();
        }
    }

    void OnShutdown() override {
        SAGE::AssetManager::Shutdown();
    }
};
```

---

### PhysicsSystem - Физика и коллизии

PhysicsSystem предоставляет 2D физику с поддержкой AABB и круговых коллайдеров, а также точный raycast для игровой механики.

#### Настройка коллизий

```cpp
#include <SAGE.h>

class PhysicsDemo : public SAGE::Application {
private:
    SAGE::Ref<SAGE::GameObject> m_Player;
    SAGE::Ref<SAGE::GameObject> m_Enemy;
    SAGE::Ref<SAGE::GameObject> m_Wall;

public:
    void OnInit() override {
        SAGE::PhysicsSystem::Init();
        SAGE::PhysicsSystem::SetGravity({ 0.0f, -9.8f });

        // Создаём игровые объекты
        m_Player = SAGE::CreateRef<SAGE::GameObject>();
        m_Player->position = { 100.0f, 100.0f };
        m_Player->tag = "Player";

        m_Enemy = SAGE::CreateRef<SAGE::GameObject>();
        m_Enemy->position = { 300.0f, 100.0f };
        m_Enemy->tag = "Enemy";

        m_Wall = SAGE::CreateRef<SAGE::GameObject>();
        m_Wall->position = { 200.0f, 50.0f };
        m_Wall->tag = "Wall";

        // Регистрируем AABB коллайдеры
        SAGE::PhysicsSystem::RegisterAABB(m_Player, { 50.0f, 50.0f });
        SAGE::PhysicsSystem::RegisterCircle(m_Enemy, 25.0f);
        SAGE::PhysicsSystem::RegisterAABB(m_Wall, { 20.0f, 100.0f });
    }

    void OnUpdate(float deltaTime) override {
        // Движение игрока
        if (SAGE::Input::IsKeyPressed(SAGE_KEY_D)) {
            m_Player->position.x += 200.0f * deltaTime;
        }
        if (SAGE::Input::IsKeyPressed(SAGE_KEY_A)) {
            m_Player->position.x -= 200.0f * deltaTime;
        }

        // Проверка коллизий
        auto collision = SAGE::PhysicsSystem::CheckCollision(m_Player, m_Enemy);
        if (collision.collided) {
            SAGE_INFO("Столкновение! Penetration: {:.2f}, Normal: ({:.2f}, {:.2f})",
                      collision.penetrationDepth, collision.normal.x, collision.normal.y);
            
            // Отталкиваем игрока
            m_Player->position -= collision.normal * collision.penetrationDepth;
        }

        // Проверка со стеной
        auto wallCollision = SAGE::PhysicsSystem::CheckCollision(m_Player, m_Wall);
        if (wallCollision.collided) {
            m_Player->position -= wallCollision.normal * wallCollision.penetrationDepth;
        }
    }

    void OnShutdown() override {
        SAGE::PhysicsSystem::Shutdown();
    }
};
```

#### Raycast для стрельбы и видимости

```cpp
class RaycastDemo : public SAGE::Application {
private:
    SAGE::Ref<SAGE::GameObject> m_Player;
    std::vector<SAGE::Ref<SAGE::GameObject>> m_Enemies;

public:
    void OnInit() override {
        SAGE::PhysicsSystem::Init();

        m_Player = SAGE::CreateRef<SAGE::GameObject>();
        m_Player->position = { 100.0f, 300.0f };
        SAGE::PhysicsSystem::RegisterAABB(m_Player, { 32.0f, 32.0f });

        // Создаём врагов
        for (int i = 0; i < 5; i++) {
            auto enemy = SAGE::CreateRef<SAGE::GameObject>();
            enemy->position = { 200.0f + i * 100.0f, 300.0f };
            enemy->tag = "Enemy";
            SAGE::PhysicsSystem::RegisterCircle(enemy, 20.0f);
            m_Enemies.push_back(enemy);
        }
    }

    void OnUpdate(float deltaTime) override {
        // Raycast при клике мыши (стрельба)
        if (SAGE::Input::IsMouseButtonPressed(SAGE_MOUSE_BUTTON_LEFT)) {
            SAGE::Vector2 mousePos = SAGE::Input::GetMousePosition();
            SAGE::Vector2 direction = (mousePos - m_Player->position).Normalized();

            // Один луч - первое попадание
            auto hit = SAGE::PhysicsSystem::Raycast(m_Player->position, direction, 1000.0f);
            if (hit.hit) {
                SAGE_INFO("Попадание в {} на расстоянии {:.2f}", 
                          hit.object->tag, hit.distance);
                
                // Визуализация попадания
                SAGE::Renderer::BeginScene();
                SAGE::Renderer::DrawLine(m_Player->position, hit.point, SAGE::Color::Red());
                SAGE::Renderer::EndScene();
            }
        }

        // Raycast через все объекты (пробивание)
        if (SAGE::Input::IsKeyPressed(SAGE_KEY_SPACE)) {
            SAGE::Vector2 direction = { 1.0f, 0.0f }; // вправо
            
            auto hits = SAGE::PhysicsSystem::RaycastAll(m_Player->position, direction, 1000.0f);
            SAGE_INFO("Луч пробил {} объектов", hits.size());
            
            for (const auto& hit : hits) {
                SAGE_INFO("  - {} на расстоянии {:.2f}", hit.object->tag, hit.distance);
            }
        }
    }

    void OnShutdown() override {
        SAGE::PhysicsSystem::Shutdown();
    }
};
```

---

### Profiler - Мониторинг производительности

Profiler отслеживает CPU и GPU метрики для оптимизации игры.

#### Базовое профилирование

```cpp
#include <SAGE.h>

class ProfiledGame : public SAGE::Application {
public:
    void OnInit() override {
        SAGE::Profiler::Init();
        SAGE::Profiler::SetTargetFPS(60.0f);
        
        // Включить GPU профилирование (опционально)
        SAGE::Profiler::EnableGPUProfiling(true);
        
        SAGE::Renderer::Init();
    }

    void OnUpdate(float deltaTime) override {
        SAGE::Profiler::BeginFrame();

        // Ваша игровая логика
        UpdateGameLogic(deltaTime);

        SAGE::Profiler::EndFrame();

        // Вывод статистики раз в секунду
        static float statTimer = 0.0f;
        statTimer += deltaTime;
        if (statTimer >= 1.0f) {
            SAGE::Profiler::PrintStats();
            statTimer = 0.0f;
        }
    }

    void OnRender() override {
        // GPU профилирование начинается автоматически при BeginScene
        SAGE::Profiler::BeginGPUFrame();
        
        SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);
        SAGE::Renderer::BeginScene();
        
        // Рисование
        for (int i = 0; i < 1000; i++) {
            SAGE::QuadDesc quad;
            quad.position = { (float)(rand() % 800), (float)(rand() % 600) };
            quad.size = { 10.0f, 10.0f };
            quad.color = SAGE::Color::Random();
            SAGE::Renderer::DrawQuad(quad);
        }
        
        SAGE::Renderer::EndScene();
        
        SAGE::Profiler::EndGPUFrame();
    }

    void OnShutdown() override {
        SAGE::Profiler::PrintStats();
        SAGE::Profiler::Shutdown();
        SAGE::Renderer::Shutdown();
    }
};
```

#### Пользовательские таймеры и метрики

```cpp
void GameplayLoop(float deltaTime) {
    // Замер конкретного участка кода
    SAGE::Profiler::StartTimer("AI Update");
    UpdateAI();
    SAGE::Profiler::StopTimer("AI Update");

    SAGE::Profiler::StartTimer("Physics Update");
    UpdatePhysics(deltaTime);
    SAGE::Profiler::StopTimer("Physics Update");

    // Автоматический замер с помощью ScopedTimer
    {
        SAGE::ScopedTimer timer("Render Preparation");
        PrepareRenderData();
        // Время автоматически записывается при выходе из scope
    }

    // Запись кастомных метрик
    SAGE::Profiler::RecordMetric("Active Enemies", (float)enemyCount);
    SAGE::Profiler::RecordMetric("Particle Count", (float)particleSystem.GetCount());

    // Вывод таймеров
    SAGE::Profiler::PrintTimers();
    
    // Получение конкретных значений
    float aiTime = SAGE::Profiler::GetTimerDuration("AI Update");
    float fps = SAGE::Profiler::GetFPS();
    float frameTime = SAGE::Profiler::GetFrameTime();
    float gpuTime = SAGE::Profiler::GetGPUTime();
    
    SAGE_INFO("AI: {:.2f}ms, FPS: {:.1f}, Frame: {:.2f}ms, GPU: {:.2f}ms",
              aiTime, fps, frameTime, gpuTime);
}
```

---

### Полный пример игры с новыми системами

```cpp
#include <SAGE.h>

class CompleteGame : public SAGE::Application {
private:
    SAGE::Ref<SAGE::GameObject> m_Player;
    std::vector<SAGE::Ref<SAGE::GameObject>> m_Enemies;
    SAGE::Ref<SAGE::Texture> m_PlayerTexture;
    bool m_AssetsLoaded = false;

public:
    CompleteGame() : Application("Complete Game Demo") {}

    void OnInit() override {
        // Инициализация всех систем
        SAGE::AssetManager::Init();
        SAGE::AssetManager::SetAssetDirectory("Assets");
        SAGE::PhysicsSystem::Init();
        SAGE::Profiler::Init();
        SAGE::Profiler::SetTargetFPS(60.0f);
        SAGE::Profiler::EnableGPUProfiling(true);
        SAGE::Renderer::Init();

        // Асинхронная загрузка ресурсов
        SAGE::AssetManager::LoadTextureAsync("player.png",
            [this](SAGE::Ref<SAGE::Texture> texture) {
                m_PlayerTexture = texture;
                m_AssetsLoaded = true;
                
                // Создаём игровые объекты после загрузки
                CreateGameObjects();
            });
    }

    void CreateGameObjects() {
        // Создаём игрока
        m_Player = SAGE::CreateRef<SAGE::GameObject>();
        m_Player->position = { 400.0f, 300.0f };
        m_Player->tag = "Player";
        SAGE::PhysicsSystem::RegisterAABB(m_Player, { 32.0f, 32.0f });

        // Создаём врагов
        for (int i = 0; i < 5; i++) {
            auto enemy = SAGE::CreateRef<SAGE::GameObject>();
            enemy->position = { 100.0f + i * 150.0f, 200.0f };
            enemy->tag = "Enemy";
            SAGE::PhysicsSystem::RegisterCircle(enemy, 20.0f);
            m_Enemies.push_back(enemy);
        }
    }

    void OnUpdate(float deltaTime) override {
        SAGE::Profiler::BeginFrame();
        SAGE::AssetManager::ProcessAsyncLoads(); // Обработка асинхронных загрузок

        if (!m_AssetsLoaded) {
            SAGE::Profiler::EndFrame();
            return; // Ждём загрузки
        }

        // Движение игрока
        {
            SAGE::ScopedTimer timer("Player Movement");
            SAGE::Vector2 movement = { 0.0f, 0.0f };
            if (SAGE::Input::IsKeyPressed(SAGE_KEY_W)) movement.y += 1.0f;
            if (SAGE::Input::IsKeyPressed(SAGE_KEY_S)) movement.y -= 1.0f;
            if (SAGE::Input::IsKeyPressed(SAGE_KEY_A)) movement.x -= 1.0f;
            if (SAGE::Input::IsKeyPressed(SAGE_KEY_D)) movement.x += 1.0f;
            
            if (movement.LengthSquared() > 0.0f) {
                movement = movement.Normalized();
                m_Player->position += movement * 200.0f * deltaTime;
            }
        }

        // Raycast при клике
        if (SAGE::Input::IsMouseButtonPressed(SAGE_MOUSE_BUTTON_LEFT)) {
            SAGE::Vector2 mousePos = SAGE::Input::GetMousePosition();
            SAGE::Vector2 direction = (mousePos - m_Player->position).Normalized();
            
            auto hit = SAGE::PhysicsSystem::Raycast(m_Player->position, direction, 1000.0f);
            if (hit.hit && hit.object->tag == "Enemy") {
                SAGE_INFO("Попал во врага!");
                // Удаляем врага
                SAGE::PhysicsSystem::UnregisterObject(hit.object);
                m_Enemies.erase(std::remove(m_Enemies.begin(), m_Enemies.end(), hit.object), m_Enemies.end());
            }
        }

        // Проверка коллизий с врагами
        {
            SAGE::ScopedTimer timer("Collision Detection");
            for (const auto& enemy : m_Enemies) {
                auto collision = SAGE::PhysicsSystem::CheckCollision(m_Player, enemy);
                if (collision.collided) {
                    m_Player->position -= collision.normal * collision.penetrationDepth;
                    SAGE_WARNING("Столкновение с врагом!");
                }
            }
        }

        // Кастомные метрики
        SAGE::Profiler::RecordMetric("Enemies Alive", (float)m_Enemies.size());

        SAGE::Profiler::EndFrame();
    }

    void OnRender() override {
        SAGE::Profiler::BeginGPUFrame();
        
        SAGE::Renderer::Clear(0.1f, 0.1f, 0.15f);
        SAGE::Renderer::BeginScene();

        if (m_AssetsLoaded) {
            // Рисуем игрока
            SAGE::SpriteDesc playerSprite;
            playerSprite.position = m_Player->position;
            playerSprite.size = { 32.0f, 32.0f };
            playerSprite.texture = m_PlayerTexture;
            SAGE::Renderer::DrawSprite(playerSprite);

            // Рисуем врагов
            for (const auto& enemy : m_Enemies) {
                SAGE::CircleDesc circle;
                circle.position = enemy->position;
                circle.radius = 20.0f;
                circle.color = SAGE::Color::Red();
                SAGE::Renderer::DrawCircle(circle);
            }
        } else {
            // Экран загрузки
            SAGE::QuadDesc loadingText;
            loadingText.position = { 350.0f, 300.0f };
            loadingText.size = { 100.0f, 20.0f };
            loadingText.color = SAGE::Color::White();
            SAGE::Renderer::DrawQuad(loadingText);
        }

        SAGE::Renderer::EndScene();
        SAGE::Profiler::EndGPUFrame();

        // Вывод статистики раз в секунду
        static float statTimer = 0.0f;
        statTimer += SAGE::Profiler::GetDeltaTime();
        if (statTimer >= 1.0f) {
            SAGE::Profiler::PrintStats();
            statTimer = 0.0f;
        }
    }

    void OnShutdown() override {
        SAGE::Profiler::PrintTimers();
        SAGE::Profiler::Shutdown();
        SAGE::PhysicsSystem::Shutdown();
        SAGE::AssetManager::Shutdown();
        SAGE::Renderer::Shutdown();
    }
};

SAGE::Application* SAGE::CreateApplication() {
    return new CompleteGame();
}

int main() {
    auto app = SAGE::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
```

## Управление

В примере Sandbox:
- **ESC** - Вывод сообщения в консоль
- **W** - Вывод сообщения в консоль
- **Закрыть окно** - Выход из приложения

## Документация

Более подробная документация появится в будущих версиях.

## Лицензия

MIT License - свободное использование и модификация.
