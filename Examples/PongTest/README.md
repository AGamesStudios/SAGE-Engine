# Pong Test Game

Простая игра Pong для тестирования основных возможностей SAGE Engine.

## 📸 Скриншот

```
╔════════════════════════════════════════════════════════════╗
║                                                            ║
║  ┃          :                          ┃                  ║
║  ┃          :                          ┃                  ║
║  ┃          :            ⬜            ┃                  ║
║  ┃          :                          ┃                  ║
║  ┃          :                          ┃                  ║
║                                                            ║
╚════════════════════════════════════════════════════════════╝
   Green       Dotted Line    White Ball    Red
  (Player 1)   (Center)                   (Player 2)
```

## 🎮 Управление

- **Player 1 (Зелёная ракетка, слева):** `W` (вверх) / `S` (вниз)
- **Player 2 (Красная ракетка, справа):** `UP` (вверх) / `DOWN` (вниз)
- **Выход:** `ESC`

## ⚙️ Сборка

```powershell
# Из корня SAGE-Engine
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release --target PongTest
```

## 🚀 Запуск

```powershell
.\build\Examples\PongTest\Release\PongTest.exe
```

## 📋 Что тестируется

### ✅ Работает отлично:
- Window creation & OpenGL initialization
- 2D Batch Renderer (`DrawQuad`)
- Input system (keyboard)
- Game loop & delta time
- Collision detection (AABB)
- Math library (`Vector2`)
- Application lifecycle (`OnInit`, `OnUpdate`, `OnRender`, `OnShutdown`)
- Colors (`Color::Green()`, `Red()`, `White()`)

### ⚠️ Проблемы:
- Logger форматирование (`%d`, `%f` не работает)
- `Renderer::Init()` вызывается дважды (warning)
- UTF-8 кракозябры в Windows консоли

## 🎯 Правила игры

Классический Pong:
- Мяч отскакивает от верхней и нижней стен
- Мяч отскакивает от ракеток с изменением угла
- Игрок получает очко, когда противник пропускает мяч
- Счёт отображается в консоли (но форматирование сломано)

## 📊 Производительность

- **FPS:** ~60 (стабильный)
- **CPU:** <5% (низкая нагрузка)
- **GPU:** Минимальная (RTX 3050)
- **RAM:** ~50 MB

## 🐛 Известные баги

1. **Logger не подставляет переменные:**
   ```
   SAGE_INFO("Score: %d - %d", score1, score2);
   // Выводит: [INFO] Score: %d - %d
   ```

2. **Мяч иногда проходит сквозь ракетку** при высокой скорости (нужен continuous collision detection)

3. **Двойной вызов Renderer::Init():**
   ```
   [INFO] Renderer initialized (batched mode)
   [WARNING] Renderer::Init() already called!
   ```

## 📝 Код

Полностью автономный код в одном файле: `main.cpp` (~220 строк)

## 🔗 Ссылки

- **Движок:** [SAGE-Engine](https://github.com/AGamesStudios/SAGE-Engine)
- **Тест репорт:** `.project/ENGINE_TEST_REPORT.md`
- **Версия:** alpha-2025.01

## 📄 Лицензия

Этот пример следует лицензии SAGE Engine (MIT License).
