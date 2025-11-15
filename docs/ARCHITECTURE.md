# SAGE Engine - Архитектура

**Версия:** Alpha  
**Стандарт:** C++17  
**Архитектура:** Entity Component System (ECS)  
**Платформы:** Windows, Linux, macOS

---

## Обзор

SAGE Engine - это современный 2D игровой движок на C++17 с архитектурой Entity Component System (ECS). Движок предназначен для создания высокопроизводительных 2D игр с поддержкой физики, анимации, аудио и скриптинга.

## Основные компоненты

### 1. Entity Component System (ECS)

ECS - это паттерн проектирования, разделяющий данные (Components) и поведение (Systems).

**Ключевые элементы:**

- **Entity** - уникальный идентификатор игрового объекта (uint64_t с версионированием)
- **Component** - данные без логики (POD структуры)
- **System** - логика обработки компонентов
- **Registry** - центральное хранилище всех entity и компонентов

**Архитектурные решения:**

```
Entity (64 бита):
  [31:0]  - ID сущности
  [63:32] - Версия (защита от ABA проблемы при переиспользовании ID)
```

**Реестр компонентов:**
- Каждый тип компонента хранится в отдельном пуле (ComponentPool)
- Компоненты индексируются по ID сущности
- Быстрый доступ O(1) через direct indexing
- Кэш-дружественное расположение данных

### 2. Системы рендеринга

**Архитектура рендеринга:**

```
Application
    |
    v
RenderContext (Singleton)
    |
    v
RenderBackend (OpenGL/DirectX/Vulkan)
    |
    v
SceneRenderer
    |
    +-- BatchRenderer (спрайты)
    +-- CommandBuffer (draw calls)
    +-- StateManager (OpenGL states)
```

**Компоненты рендеринга:**
- **Texture** - текстурные данные с GPU handle
- **Shader** - GLSL программы
- **Material** - набор параметров для шейдера
- **SpriteBatchSoA** - батчинг спрайтов (Structure of Arrays)

**Оптимизации:**
- Автоматический батчинг спрайтов по текстуре
- State sorting для минимизации state changes
- Frustum culling для 2D
- GPU memory tracking

### 3. Физический движок

**Абстракция физики:**

```
IPhysicsBackend (Interface)
    |
    +-- Box2DBackend (Box2D v3.x)
    +-- CustomPhysics (опционально)
```

**PhysicsSystem:**
- Легковесная обёртка над физическим бэкендом
- Автоматическая синхронизация Transform <-> Physics
- Создание физических тел по требованию
- Поддержка различных типов коллайдеров

**Компоненты:**
- **PhysicsComponent** - масса, трение, restitution, тип тела
- **ColliderComponent** - форма коллайдера (Box, Circle, Polygon)
- **TransformComponent** - позиция, поворот, размер

**Жизненный цикл физического тела:**
1. Добавление PhysicsComponent к entity
2. PhysicsSystem обнаруживает новый компонент
3. Создание физического тела в бэкенде
4. Каждый кадр: синхронизация Transform <-> Body

### 4. Аудио система

**Архитектура:**

```
AudioSystem (класс)
    |
    +-- miniaudio v0.11.23 (backend)
    +-- SFX Management (короткие звуки)
    +-- BGM Management (фоновая музыка)
    +-- 3D Positioning (spatial audio)
```

**Особенности:**
- Streaming для больших файлов
- Множественные голоса для SFX
- Fade in/out для музыки
- Volume control (master, category, individual)
- Listener positioning для 3D звука

### 5. Менеджер ресурсов

**ResourceManager:**

```
ResourceManager (Singleton)
    |
    +-- LRU Cache (текстуры, аудио, шрифты)
    +-- GPU Memory Tracking
    +-- Async Loading (опционально)
    +-- Headless Mode (для тестов)
```

**Стратегия кэширования:**
- LRU (Least Recently Used) eviction
- Бюджет GPU памяти
- Reference counting через shared_ptr
- Автоматическая выгрузка при превышении бюджета

**Типы ресурсов:**
- Texture (GPU текстуры)
- Font (шрифты с атласами)
- Audio (звуковые буферы)
- Shader (скомпилированные программы)

### 6. Система событий

**EventBus:**

```
EventBus (Singleton)
    |
    +-- Immediate Dispatch
    +-- Queued Dispatch
    +-- Priority Queue
    +-- Category Filtering
    +-- Weak Listeners (auto-cleanup)
```

**Особенности:**
- Type-safe события через шаблоны
- Подписка с приоритетами
- Группировка обработчиков
- Background worker для async обработки
- Event coalescing (объединение дубликатов)

**Производительность:**
- 100,000 событий < 1 микросекунда/событие
- Lock-free для immediate dispatch
- Batch processing для queued events

### 7. Система анимации

**AnimationSystem:**

```
AnimationClip
    |
    +-- Frames (UV coordinates, duration)
    +-- Loop mode
    +-- Playback control
```

**AnimationComponent:**
- Ссылка на AnimationClip
- Текущий кадр
- Время накопления
- Флаги воспроизведения (playing, looping)

**Типы анимаций:**
- Sprite sheet animation (UV mapping)
- Frame-by-frame animation
- Interpolation (опционально)

### 8. Система ввода

**InputManager:**

```
InputManager (Singleton)
    |
    +-- Keyboard State
    +-- Mouse State
    +-- Gamepad State (до 4 геймпадов)
    +-- Action Mapping
    +-- Input Buffer
```

**Абстракция:**
- **IWindow** - интерфейс для GLFW/SDL
- Опрос состояния каждый кадр
- История для комбо-вводов
- Deadzone для стиков

### 9. Скриптинговая система LogCon

**LogCon:**

```
LogConCompiler
    |
    +-- Lexer (токенизация)
    +-- Parser (AST построение)
    +-- Language Detection (русский/английский)
```

**Особенности:**
- Двуязычный синтаксис (RU/EN)
- Декларативное описание сущностей
- Компиляция в entity blueprint
- Интеграция с ECS

## Потоки данных

### Игровой цикл

```
1. Input Polling (InputManager)
2. Event Processing (EventBus)
3. System Updates:
   - PhysicsSystem::FixedUpdate (фиксированный timestep)
   - AnimationSystem::Update
   - ScriptSystem::Update
4. Rendering:
   - Camera Update
   - Frustum Culling
   - Batch Building
   - Draw Call Submission
5. Audio Update (AudioSystem)
6. Frame Timing
```

### Создание entity

```
1. Registry::CreateEntity() -> Entity ID
2. Registry::AddComponent(entity, component)
3. Component сохраняется в ComponentPool
4. Systems автоматически обнаруживают новый компонент
5. При наличии PhysicsComponent - создаётся физическое тело
```

### Рендеринг entity

```
1. ForEach<TransformComponent, SpriteComponent>
2. Frustum culling по Transform
3. Batch sorting по текстуре/shader
4. SpriteBatchSoA::Add(transform, sprite)
5. BatchRenderer::Flush() -> GPU draw calls
```

## Управление памятью

### Entity и компоненты
- Entity ID переиспользуются с инкрементом версии
- Компоненты удаляются swap-and-pop
- Нет фрагментации в ComponentPool

### GPU ресурсы
- ResourceManager отслеживает использование VRAM
- LRU eviction при превышении бюджета
- Shared ownership через Ref<T> (shared_ptr)

### Аудио буферы
- Streaming для файлов > порога
- Буферы освобождаются при завершении воспроизведения
- Переиспользование голосов для SFX

## Многопоточность

**Текущее состояние:**
- Основной поток: рендеринг, физика, логика
- EventBus: опциональный background worker
- ResourceManager: async loading (опционально)

**Синхронизация:**
- Lock-free очереди для событий
- Mutex для resource cache
- Atomic counters для reference counting

## Конфигурация

### engine_config.json
```json
{
  "window": {
    "width": 1280,
    "height": 720,
    "title": "SAGE Engine",
    "vsync": true
  },
  "physics": {
    "gravity": [0, 980],
    "iterations": 8
  },
  "audio": {
    "masterVolume": 1.0,
    "sfxVolume": 0.8,
    "bgmVolume": 0.7
  }
}
```

## Расширяемость

### Добавление нового компонента

1. Создать структуру компонента:
```cpp
struct MyComponent {
    float value;
    std::string data;
};
```

2. Зарегистрировать в Registry (автоматически через шаблоны)

3. Создать систему обработки:
```cpp
class MySystem : public System {
    void Update(Registry& registry, float dt) override {
        registry.ForEach<MyComponent>([](Entity e, MyComponent& c) {
            // Логика
        });
    }
};
```

### Добавление нового физического бэкенда

1. Реализовать IPhysicsBackend
2. Переопределить методы Create/Update/Step
3. Зарегистрировать в PhysicsSystem

## Производительность

### Бенчмарки (Release build):

- 1000 entity с физикой: ~55 мс симуляция
- 100,000 событий: ~95 мс обработка
- 500 entity создание: ~90 мс
- Батчинг 1000 спрайтов: < 5 мс

### Профилирование:

- Встроенный Profiler с иерархическими метриками
- GPU timing через OpenGL queries
- Memory tracking в ResourceManager

## Зависимости

### Обязательные:
- GLFW 3.x (окна, ввод)
- GLAD (OpenGL загрузка)
- Box2D 3.x (физика)
- miniaudio 0.11.23 (аудио)
- stb_image (загрузка изображений)
- nlohmann/json (конфигурация)

### Опциональные:
- ImGui (редактор/debug UI)
- tinyxml2 (XML парсинг)
- sol2 (Lua скриптинг)

## Ограничения

### Текущие:
- Только 2D рендеринг (нет 3D)
- Только OpenGL backend (DirectX/Vulkan в планах)
- Однопоточный игровой цикл
- Максимум 65535 одновременных entity

### Планируемые улучшения:
- Multithreaded system updates
- Render graph для advanced effects
- Network synchronization
- Editor integration
