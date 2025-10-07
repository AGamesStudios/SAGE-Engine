# SAGE Engine - Early Alpha Release

**English** | [Русский](#sage-engine---ранняя-альфа-версия)

---

## About SAGE Engine

SAGE Engine is a 2D game engine built with C++20 and OpenGL 3.3+. Currently in early alpha development.

**Version:** Early Alpha (2025-01-07)  
**License:** MIT  
**Platform:** Windows (tested), Linux/macOS (experimental)

---

## What Works Now

- Application window with GLFW
- 2D batch rendering (OpenGL 3.3+)
- Keyboard and mouse input
- AABB collision detection
- Texture loading and rendering
- Multi-channel audio playback
- Logging system with printf formatting
- Math utilities (Vector2, Rect, Transform)
- CMake build system
- Standalone EXE builds (Windows, 1.2 MB)

---

## Demo: Pong Game

Simple Pong game to test the engine:
- Two paddles: Left (W/S keys), Right (Arrow keys)
- Ball bounces off walls and paddles
- Score tracking
- Single 1.2 MB executable, no external DLLs needed

---

## Known Issues

- Logger UTF-8 console output has encoding issues on some systems
- Renderer Init() may be called twice (warning shown, not critical)
- Collision detection fails with fast-moving objects
- No 3D rendering
- No visual editor
- Documentation incomplete
- Test coverage partial
- Linux/macOS builds not thoroughly tested

---

## Limitations

- 2D only (no 3D support)
- No ECS architecture yet
- No sprite animation system
- No tilemap support
- No scene serialization
- No networking
- No scripting integration
- Small community

---

## Who Is It For

**Good for:**
- Learning C++20 game engine architecture
- Rapid prototyping 2D games
- Game jams (quick standalone builds)
- Understanding how game engines work internally
- Educational projects

**Not for:**
- 3D games
- Large commercial projects
- Visual designers (no editor)
- Mobile platforms (not tested)
- Production-ready games (alpha quality)

---

## Real Differences from Other Engines

### vs Unity/Godot
- Smaller: 25,000 lines of code (Unity/Godot have millions)
- Simpler: Can read entire codebase in a week
- Standalone: 1.2 MB EXE with no DLLs (Unity needs 50+ MB)
- Transparent: Every line of code is visible
- Limited: Only 2D, no editor, alpha quality

### vs SDL2/SFML
- Modern: C++20 (SDL2 is C89, SFML is C++11)
- Standalone: No DLLs needed (SDL2/SFML require DLLs)
- Higher-level: Built-in batch renderer, scene system
- Less mature: SDL2/SFML are stable, SAGE is alpha

---

## Download

**Repository:** https://github.com/AGamesStudios/SAGE-Engine  
**Latest Release:** Early Alpha (2025-01-07)

**Requirements:**
- Windows 10+ (primary), Linux/macOS (experimental)
- C++20 compiler (MSVC 2019+, GCC 9+, Clang 10+)
- CMake 3.20+
- OpenGL 3.3+

**Build:**
```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
```

---

## Future Plans

- Fix critical bugs (collision, UTF-8 encoding)
- Add sprite animation system
- Implement tilemap support
- Scene serialization
- Improve documentation
- Test on Linux/macOS

---

## Contact

- **Issues:** https://github.com/AGamesStudios/SAGE-Engine/issues
- **Discussions:** https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**Author:** AGamesStudios  
**Date:** October 2025  
**Status:** Early Alpha - Experimental Software

---

# SAGE Engine - Ранняя Альфа Версия

[English](#sage-engine---early-alpha-release) | **Русский**

---

## О SAGE Engine

SAGE Engine — это 2D игровой движок на C++20 и OpenGL 3.3+. Сейчас находится в ранней альфа-разработке.

**Версия:** Ранняя Альфа (2025-01-07)  
**Лицензия:** MIT  
**Платформа:** Windows (протестировано), Linux/macOS (экспериментально)

---

## Что работает сейчас

- Игровое окно на GLFW
- 2D батч-рендеринг (OpenGL 3.3+)
- Управление клавиатурой и мышью
- AABB определение столкновений
- Загрузка и отрисовка текстур
- Многоканальное воспроизведение звука
- Система логирования с printf форматированием
- Математические утилиты (Vector2, Rect, Transform)
- Сборка через CMake
- Standalone EXE файлы (Windows, 1.2 МБ)

---

## Демо: Игра Pong

Простая игра Pong для теста движка:
- Две ракетки: Левая (клавиши W/S), Правая (стрелки)
- Мяч отскакивает от стен и ракеток
- Подсчёт очков
- Один исполняемый файл 1.2 МБ, без внешних DLL

---

## Известные проблемы

- UTF-8 вывод в консоль работает с ошибками на некоторых системах
- Renderer Init() может вызываться дважды (показывается предупреждение, не критично)
- Определение столкновений не работает с быстрыми объектами
- Нет 3D рендеринга
- Нет визуального редактора
- Документация неполная
- Частичное покрытие тестами
- Сборки для Linux/macOS не тщательно протестированы

---

## Ограничения

- Только 2D (нет поддержки 3D)
- Пока нет ECS архитектуры
- Нет системы анимации спрайтов
- Нет поддержки тайлмапов
- Нет сериализации сцен
- Нет сетевого взаимодействия
- Нет интеграции скриптов
- Небольшое сообщество

---

## Для кого это

**Подходит для:**
- Изучения архитектуры игрового движка на C++20
- Быстрого прототипирования 2D игр
- Геймджемов (быстрая сборка standalone)
- Понимания внутреннего устройства игровых движков
- Образовательных проектов

**Не подходит для:**
- 3D игр
- Крупных коммерческих проектов
- Визуальных дизайнеров (нет редактора)
- Мобильных платформ (не тестировалось)
- Production-ready игр (качество альфа)

---

## Реальные отличия от других движков

### vs Unity/Godot
- Меньше: 25,000 строк кода (Unity/Godot имеют миллионы)
- Проще: Можно прочитать весь код за неделю
- Standalone: 1.2 МБ EXE без DLL (Unity требует 50+ МБ)
- Прозрачно: Каждая строка кода видна
- Ограничено: Только 2D, нет редактора, альфа качество

### vs SDL2/SFML
- Современный: C++20 (SDL2 это C89, SFML это C++11)
- Standalone: Не нужны DLL (SDL2/SFML требуют DLL)
- Высокоуровневый: Встроенный батч-рендерер, система сцен
- Менее зрелый: SDL2/SFML стабильны, SAGE в альфе

---

## Скачать

**Репозиторий:** https://github.com/AGamesStudios/SAGE-Engine  
**Последний релиз:** Ранняя Альфа (2025-01-07)

**Требования:**
- Windows 10+ (основная), Linux/macOS (экспериментально)
- C++20 компилятор (MSVC 2019+, GCC 9+, Clang 10+)
- CMake 3.20+
- OpenGL 3.3+

**Сборка:**
```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
```

---

## Планы на будущее

- Исправить критические баги (столкновения, UTF-8 кодировка)
- Добавить систему анимации спрайтов
- Реализовать поддержку тайлмапов
- Сериализация сцен
- Улучшить документацию
- Протестировать на Linux/macOS

---

## Контакты

- **Проблемы:** https://github.com/AGamesStudios/SAGE-Engine/issues
- **Обсуждения:** https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**Автор:** AGamesStudios  
**Дата:** Октябрь 2025  
**Статус:** Ранняя Альфа - Экспериментальное ПО