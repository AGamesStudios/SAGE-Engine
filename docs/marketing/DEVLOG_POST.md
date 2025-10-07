# Development Log - SAGE Engine Early Alpha

**English** | [Русский](#журнал-разработки---sage-engine-ранняя-альфа)

---

## Why I Built SAGE Engine

I wanted to understand how game engines work from the ground up. No black boxes, no magic - just C++20, OpenGL, and learning by doing.

### Goals:
- Build a simple 2D engine from scratch
- Learn modern C++20 features in practice
- Create standalone executables without DLL dependencies
- Make something useful for game jams and prototyping

---

## Development Journey

### October 2024 - Start
- Set up CMake build system
- Integrated GLFW for windowing
- Basic OpenGL 3.3+ renderer initialization

### November 2024 - Core Systems
- Implemented batch renderer for sprites
- Added keyboard and mouse input handling
- AABB collision detection
- Multi-channel audio with OpenAL

### December 2024 - Math and Resources
- Vector2, Rect, Transform classes
- Texture loading system
- Resource management
- Logger with printf formatting

### January 2025 - Testing and Fixes
- Built Pong game demo to test engine
- Fixed Logger UTF-8 console issues
- Added standalone EXE build (1.2 MB, no DLLs)
- Documentation cleanup

**Current Status:** Early Alpha - core features work, many rough edges

---

## What I Learned

### Technical Lessons
- OpenGL batch rendering is complex but powerful
- Static linking (/MT) requires careful CMake configuration
- UTF-8 console output on Windows needs special handling
- Fast-moving objects need better collision algorithms

### Design Lessons
- Start simple - don't build Unity/Godot clone
- Test with real games early (Pong was crucial)
- Documentation is hard but necessary
- Standalone builds matter for distribution

---

## Challenges

1. **Runtime Library Issues**: Spent days debugging /MD vs /MT linker errors
2. **Logger Formatting**: Printf-style formatting needed custom parser
3. **Collision Detection**: Fast objects phase through walls - needs swept AABB
4. **Cross-Platform**: Windows works well, Linux/macOS untested

---

## What's Next

### Short-term (1-2 months)
- Fix collision detection for fast objects
- Add sprite animation system
- Implement tilemap support
- Test on Linux and macOS

### Medium-term (3-6 months)
- Scene serialization
- Basic visual editor (maybe)
- Scripting integration (Lua?)
- More example games

### Long-term (6+ months)
- Entity Component System (ECS)
- Networking layer
- Mobile platform support (Android?)
- Community building

---

## Try It

**Repository:** https://github.com/AGamesStudios/SAGE-Engine  
**Latest Release:** Early Alpha (2025-01-07)

Build Pong in 3 minutes:
```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
.\build\Examples\PongTest\Release\PongTest.exe
```

---

## Feedback Welcome

I'm learning as I go. If you find bugs or have suggestions, please open an issue or discussion on GitHub.

**Contact:**
- Issues: https://github.com/AGamesStudios/SAGE-Engine/issues
- Discussions: https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**Author:** AGamesStudios  
**Date:** January 2025  
**Status:** Early Alpha - Experimental

---

# Журнал Разработки - SAGE Engine Ранняя Альфа

[English](#development-log---sage-engine-early-alpha) | **Русский**

---

## Почему я создал SAGE Engine

Я хотел понять, как работают игровые движки с нуля. Никаких черных ящиков, никакой магии - только C++20, OpenGL и обучение на практике.

### Цели:
- Построить простой 2D движок с нуля
- Изучить современные возможности C++20 на практике
- Создать standalone исполняемые файлы без зависимостей от DLL
- Сделать что-то полезное для геймджемов и прототипирования

---

## Путь Разработки

### Октябрь 2024 - Начало
- Настроил систему сборки CMake
- Интегрировал GLFW для оконного менеджмента
- Базовая инициализация рендерера OpenGL 3.3+

### Ноябрь 2024 - Основные Системы
- Реализовал батч-рендерер для спрайтов
- Добавил обработку клавиатуры и мыши
- AABB определение столкновений
- Многоканальное аудио с OpenAL

### Декабрь 2024 - Математика и Ресурсы
- Классы Vector2, Rect, Transform
- Система загрузки текстур
- Управление ресурсами
- Логгер с printf форматированием

### Январь 2025 - Тестирование и Исправления
- Создал демо-игру Pong для теста движка
- Исправил проблемы UTF-8 консоли в Logger
- Добавил сборку standalone EXE (1.2 МБ, без DLL)
- Очистка документации

**Текущий статус:** Ранняя Альфа - основные функции работают, много шероховатостей

---

## Что Я Узнал

### Технические Уроки
- Батч-рендеринг OpenGL сложный, но мощный
- Статическая линковка (/MT) требует тщательной настройки CMake
- UTF-8 вывод в консоль Windows требует специальной обработки
- Быстродвижущиеся объекты нуждаются в лучших алгоритмах столкновений

### Уроки Дизайна
- Начинай просто - не строй клон Unity/Godot
- Тестируй с реальными играми рано (Pong был критичен)
- Документация сложна, но необходима
- Standalone сборки важны для распространения

---

## Вызовы

1. **Проблемы Runtime Library**: Потратил дни на отладку ошибок линковки /MD vs /MT
2. **Форматирование Logger**: Printf-стиль форматирования требовал кастомного парсера
3. **Определение Столкновений**: Быстрые объекты проходят сквозь стены - нужен swept AABB
4. **Кросс-Платформенность**: Windows работает хорошо, Linux/macOS не протестированы

---

## Что Дальше

### Краткосрочно (1-2 месяца)
- Исправить определение столкновений для быстрых объектов
- Добавить систему анимации спрайтов
- Реализовать поддержку тайлмапов
- Протестировать на Linux и macOS

### Среднесрочно (3-6 месяцев)
- Сериализация сцен
- Базовый визуальный редактор (возможно)
- Интеграция скриптов (Lua?)
- Больше примеров игр

### Долгосрочно (6+ месяцев)
- Entity Component System (ECS)
- Сетевой слой
- Поддержка мобильных платформ (Android?)
- Развитие сообщества

---

## Попробуй

**Репозиторий:** https://github.com/AGamesStudios/SAGE-Engine  
**Последний Релиз:** Ранняя Альфа (2025-01-07)

Собери Pong за 3 минуты:
```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
.\build\Examples\PongTest\Release\PongTest.exe
```

---

## Обратная Связь Приветствуется

Я учусь по ходу дела. Если найдешь баги или есть предложения, открой issue или discussion на GitHub.

**Контакты:**
- Проблемы: https://github.com/AGamesStudios/SAGE-Engine/issues
- Обсуждения: https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**Автор:** AGamesStudios  
**Дата:** Январь 2025  
**Статус:** Ранняя Альфа - Экспериментальное ПО