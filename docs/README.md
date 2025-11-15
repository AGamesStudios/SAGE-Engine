# SAGE Engine - Техническая документация

**Версия движка:** Alpha  
**Версия документации:** Alpha  
**Дата обновления:** Ноябрь 2025  
**Совместимость:** C++17, CMake 3.15+

---

## Содержание

### Основная документация

1. **ARCHITECTURE.md** - Архитектура движка
   - Обзор ECS системы
   - Системы рендеринга
   - Физический движок
   - Аудио система
   - Менеджер ресурсов
   - Система событий
   - Потоки данных и производительность

2. **API_REFERENCE.md** - Справочник API
   - Core API (Registry, Entity, System)
   - Graphics API (RenderContext, Camera2D, Texture, Shader)
   - Physics API (PhysicsSystem, настройки)
   - Audio API (AudioSystem)
   - Resource Management API
   - Event System API
   - Input API
   - Math API (Vector2, Matrix4, Random)
   - Utility API (Color, Logger)
   - Application Framework

3. **USER_GUIDE.md** - Руководство пользователя
   - Быстрый старт
   - Работа с системами
   - Рендеринг
   - Управление ресурсами
   - Отладка и профилирование
   - Оптимизация
   - Примеры проектов

### Дополнительные руководства

4. **BUILD_GUIDE.md** - Руководство по сборке
   - Системные требования
   - Установка инструментов
   - Конфигурация CMake
   - Опции сборки
   - Решение проблем

5. **COMPONENT_REFERENCE.md** - Справочник компонентов
   - TransformComponent - позиция, вращение, размер
   - SpriteComponent - визуальное представление
   - PhysicsComponent - физические свойства
   - ColliderComponent - формы столкновений
   - AnimationComponent - анимация спрайтов
   - Создание пользовательских компонентов

6. **SYSTEM_REFERENCE.md** - Справочник систем
   - PhysicsSystem - физическая симуляция
   - AnimationSystem - обновление анимаций
   - RenderSystem - рендеринг графики
   - AudioSystem - воспроизведение звука
   - InputManager - управление вводом
   - ResourceManager - управление ресурсами
   - EventBus - система событий

7. **MATH_API_REFERENCE.md** - Математические API
   - Vector2/Vector3/Vector4 - векторная математика
   - Matrix4 - матричные трансформации
   - Random - генерация случайных чисел
   - Color - работа с цветом
   - Time - управление временем

8. **PERFORMANCE_GUIDE.md** - Оптимизация производительности
   - Профилирование
   - ECS оптимизация
   - Рендеринг оптимизация
   - Физика оптимизация
   - Управление памятью
   - Бенчмарки

## Быстрая навигация

### Начало работы

Если вы новичок в SAGE Engine:
1. Прочитайте раздел "Быстрый старт" в **USER_GUIDE.md**
2. Изучите **BUILD_GUIDE.md** для настройки среды разработки
3. Просмотрите примеры в директории **Examples/**
4. Используйте **API_REFERENCE.md** как справочник

### Разработка игр

Для создания игры:
1. **USER_GUIDE.md** - основные паттерны и практики
2. **COMPONENT_REFERENCE.md** - доступные компоненты
3. **SYSTEM_REFERENCE.md** - встроенные системы
4. **API_REFERENCE.md** - детали API

### Архитектура и внутренности

Для понимания устройства движка:
1. **ARCHITECTURE.md** - общая архитектура
2. **PERFORMANCE_GUIDE.md** - оптимизация
3. Исходный код в директории **Engine/**

### Сборка и конфигурация

Для настройки проекта:
1. **BUILD_GUIDE.md** - инструкции по сборке
2. **engine_config.json** - пример конфигурации
3. **CMakeLists.txt** - настройки CMake

## Ключевые концепции

### Entity Component System (ECS)

ECS - это архитектурный паттерн, разделяющий данные и логику:

```
Entity (ID) + Components (данные) -> Systems (логика)
```

**Подробнее:** ARCHITECTURE.md, разделы 1-2

### Игровой цикл

```
Input -> Events -> System Updates -> Physics -> Rendering -> Audio
```

**Подробнее:** ARCHITECTURE.md, раздел "Потоки данных"

### Компоненты

Основные компоненты для игровых объектов:

- **TransformComponent** - позиция, поворот, размер
- **SpriteComponent** - визуальное представление
- **PhysicsComponent** - физические свойства
- **ColliderComponent** - форма столкновения
- **AnimationComponent** - анимация спрайта

**Подробнее:** API_REFERENCE.md, раздел "Компоненты"

### Системы

Встроенные системы обработки:

- **PhysicsSystem** - физическая симуляция
- **AnimationSystem** - обновление анимаций
- **RenderSystem** - отрисовка спрайтов
- **AudioSystem** - воспроизведение звука

**Подробнее:** API_REFERENCE.md, раздел "System"

## Примеры кода

### Создание entity

```cpp
Registry registry;
Entity player = registry.CreateEntity();

TransformComponent transform;
transform.position = Vector2(100, 200);
registry.AddComponent(player, transform);

SpriteComponent sprite;
sprite.texturePath = "player.png";
registry.AddComponent(player, sprite);
```

### Физическая симуляция

```cpp
PhysicsComponent physics;
physics.type = PhysicsBodyType::Dynamic;
physics.SetMass(1.0f);
registry.AddComponent(player, physics);

auto collider = ColliderComponent::CreateBox(Vector2(32, 48));
registry.AddComponent(player, collider);

PhysicsSystem physicsSystem;
physicsSystem.FixedUpdate(registry, deltaTime);
```

### События

```cpp
struct PlayerDiedEvent {
    Entity player;
    int score;
};

auto& bus = EventBus::Get();
bus.Subscribe<PlayerDiedEvent>([](const PlayerDiedEvent& e) {
    // Обработка
});

PlayerDiedEvent event;
event.player = playerEntity;
event.score = 1000;
bus.Publish(event);
```

### Анимация

```cpp
auto clip = std::make_shared<AnimationClip>("walk");
clip->AddFrame(Float2(0, 0), Float2(0.25f, 0.25f), 0.1f);
clip->AddFrame(Float2(0.25f, 0), Float2(0.5f, 0.25f), 0.1f);

AnimationComponent anim;
anim.SetClip(clip);
anim.Play();
registry.AddComponent(player, anim);
```

## Версии документации

Текущая версия: 1.0 (ноябрь 2025)

Документация соответствует SAGE Engine v1.0.

## Структура документации

```
docs/
├── README.md                    # Главная страница (этот файл)
├── ARCHITECTURE.md              # Архитектура движка
├── API_REFERENCE.md             # Справочник API
├── USER_GUIDE.md                # Руководство пользователя
├── BUILD_GUIDE.md               # Руководство по сборке
├── COMPONENT_REFERENCE.md       # Справочник компонентов
├── SYSTEM_REFERENCE.md          # Справочник систем
├── MATH_API_REFERENCE.md        # Математические API
└── PERFORMANCE_GUIDE.md         # Оптимизация производительности
```

## Полнота документации

Документация покрывает:

- 100% публичного API
- Все основные компоненты (Transform, Sprite, Physics, Collider, Animation)
- Все системы (Physics, Animation, Render, Audio, Input, Events)
- Математические типы (Vector2/3/4, Matrix4, Random, Color)
- Утилиты (Filesystem, String, Time)
- Примеры использования для каждого API
- Паттерны и лучшие практики
- Оптимизация и профилирование
- Решение частых проблем

## Вклад в документацию

Для улучшения документации:

1. Создайте Issue с предложением
2. Отправьте Pull Request с исправлениями
3. Используйте тот же формат и стиль

Требования к документации:
- Ясность и конкретность
- Примеры кода для сложных концепций
- Без emoji
- Поддержка русского языка
- Технические детали и спецификации

## Лицензия

SAGE Engine распространяется под лицензией MIT.

См. файл **LICENSE** для деталей.

## Контакты

- GitHub: https://github.com/AGamesStudios/SAGE-Engine
- Issues: https://github.com/AGamesStudios/SAGE-Engine/issues
- Wiki: https://github.com/AGamesStudios/SAGE-Engine/wiki
