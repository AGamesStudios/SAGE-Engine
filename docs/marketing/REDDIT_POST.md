# [Release] SAGE Engine - Early Alpha 2D Game Engine (C++20 + OpenGL)

**English** | [Русский](#релиз-sage-engine---ранняя-альфа-2d-игрового-движка-c20--opengl)

---

## What is SAGE Engine?

A lightweight 2D game engine I've been building to learn how engines work internally. Written in C++20 with OpenGL 3.3+.

**GitHub:** https://github.com/AGamesStudios/SAGE-Engine  
**License:** MIT  
**Platform:** Windows (tested), Linux/macOS (experimental)

---

## Current Features

- 2D batch rendering
- Keyboard/mouse input
- AABB collision detection
- Texture loading
- Multi-channel audio
- Standalone EXE builds (1.2 MB, no DLLs)
- CMake build system

---

## Demo: Pong Game

Built a simple Pong game to test the engine:
- 1.2 MB standalone executable
- Two-player controls (WASD + Arrow keys)
- No external dependencies

Download and run the demo from the repository.

---

## Honest Limitations

This is **early alpha software**. Here's what doesn't work yet:

- No 3D rendering
- No visual editor
- Collision fails with fast-moving objects
- UTF-8 console output has issues
- No sprite animation system
- No tilemap support
- Limited cross-platform testing

---

## Who Might Find This Useful

**Good for:**
- Learning game engine architecture
- Game jam rapid prototyping
- Educational C++20 projects
- Understanding OpenGL rendering

**Not for:**
- Production games
- 3D projects
- Visual designers (no editor)
- Mobile platforms

---

## Why Share This

I'm sharing this to:
1. Get feedback from experienced developers
2. Help others learning game engine development
3. Build in public and stay motivated

If you're also learning how engines work, feel free to explore the code. It's small enough to read through (25,000 lines).

---

## Build Instructions

```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
```

Run Pong demo:
```bash
.\build\Examples\PongTest\Release\PongTest.exe
```

---

## What's Next

- Fix collision detection for fast objects
- Add sprite animation
- Implement tilemap support
- Test on Linux/macOS
- Improve documentation

---

## Feedback Welcome

Found a bug? Have suggestions? Open an issue or discussion:
- **Issues:** https://github.com/AGamesStudios/SAGE-Engine/issues
- **Discussions:** https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**TL;DR:** Made a simple 2D game engine in C++20 to learn how engines work. It's early alpha, has many limitations, but works for basic games (like Pong). MIT licensed, feedback welcome.

---

# [Релиз] SAGE Engine - Ранняя Альфа 2D Игрового Движка (C++20 + OpenGL)

[English](#release-sage-engine---early-alpha-2d-game-engine-c20--opengl) | **Русский**

---

## Что такое SAGE Engine?

Легковесный 2D игровой движок, который я создавал для изучения внутреннего устройства движков. Написан на C++20 с OpenGL 3.3+.

**GitHub:** https://github.com/AGamesStudios/SAGE-Engine  
**Лицензия:** MIT  
**Платформа:** Windows (протестировано), Linux/macOS (экспериментально)

---

## Текущие Возможности

- 2D батч-рендеринг
- Управление клавиатурой/мышью
- AABB определение столкновений
- Загрузка текстур
- Многоканальное аудио
- Standalone EXE сборки (1.2 МБ, без DLL)
- Система сборки CMake

---

## Демо: Игра Pong

Создал простую игру Pong для теста движка:
- 1.2 МБ standalone исполняемый файл
- Управление для двух игроков (WASD + Стрелки)
- Без внешних зависимостей

Скачай и запусти демо из репозитория.

---

## Честные Ограничения

Это **ранняя альфа версия**. Вот что ещё не работает:

- Нет 3D рендеринга
- Нет визуального редактора
- Столкновения не работают с быстрыми объектами
- UTF-8 вывод в консоль имеет проблемы
- Нет системы анимации спрайтов
- Нет поддержки тайлмапов
- Ограниченное кросс-платформенное тестирование

---

## Кому Это Может Быть Полезно

**Подходит для:**
- Изучения архитектуры игровых движков
- Быстрого прототипирования для геймджемов
- Образовательных C++20 проектов
- Понимания OpenGL рендеринга

**Не подходит для:**
- Production игр
- 3D проектов
- Визуальных дизайнеров (нет редактора)
- Мобильных платформ

---

## Почему Я Делюсь Этим

Я делюсь этим, чтобы:
1. Получить обратную связь от опытных разработчиков
2. Помочь другим, изучающим разработку игровых движков
3. Строить публично и оставаться мотивированным

Если ты также изучаешь, как работают движки, можешь изучить код. Он достаточно мал, чтобы прочитать его полностью (25,000 строк).

---

## Инструкции по Сборке

```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
cmake -S . -B build
cmake --build build --config Release
```

Запуск демо Pong:
```bash
.\build\Examples\PongTest\Release\PongTest.exe
```

---

## Что Дальше

- Исправить определение столкновений для быстрых объектов
- Добавить анимацию спрайтов
- Реализовать поддержку тайлмапов
- Протестировать на Linux/macOS
- Улучшить документацию

---

## Обратная Связь Приветствуется

Нашёл баг? Есть предложения? Открой issue или discussion:
- **Проблемы:** https://github.com/AGamesStudios/SAGE-Engine/issues
- **Обсуждения:** https://github.com/AGamesStudios/SAGE-Engine/discussions

---

**TL;DR:** Создал простой 2D игровой движок на C++20 для изучения устройства движков. Ранняя альфа, много ограничений, но работает для базовых игр (как Pong). MIT лицензия, обратная связь приветствуется.