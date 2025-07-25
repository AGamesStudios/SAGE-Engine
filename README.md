# SAGE Engine

**SAGE Engine** — легкий, модульный и супероптимизированный 2D/3D игровой движок с поддержкой Python, Lua и оригинального FlowScript. Он подходит для слабых ПК, веба и десктопов, обеспечивая гибкость и большую скорость.

## 📦 Версии

- **Alpha 0.1** ✅ — Ядро, профилировка, ChronoPatch, SAGE Render
- **Alpha 0.2** ✅ — Объекты, сцены, события, модульность, Lua
- **Alpha 0.3** ✅ — Python, FlowScript, Feather-FX, логика, скрипты, оптимизация
- **Alpha 0.4** 🚧 — Ввод, UI, Terminal, VFrame, асинхронность
- **Beta** ⏳ — Визуальный редактор, проекты, визуальная логика

## 📁 Структура проекта

```
├── data/             # Общие ресурсы
├── examples/         # Примеры
├── docs/             # Документация
├── rust/             # Rust ядро
├── sage/             # Точка входа
├── sage_engine/      # Ядро движка
├── sage_object/      # Объекты
├── sage_fs/          # SAGE FlowScript
├── sage_fx/          # FeatherFX
├── tools/            # Утилиты
├── tests/            # Юнит-тесты
├── README.md
├── LICENSE
```

## ⚙️ Установка

```bash
git clone https://github.com/yourname/sage-engine.git
cd sage-engine
python -m venv .venv
source .venv/bin/activate    # Windows: .venv\Scripts\activate
pip install -r requirements.txt
python examples/basic_display/main.py
```

## 📜 Скриптовые языки

- **Python** — стандартная логика
- **Lua** — легкий и быстрый
- **SAGE FlowScript** — оригинальный язык

## 🚀 Технологии

- `SAGE Render` — отрисовка для слабых ПК
- `SAGE Object` — роли и объекты
- `SAGE Scene` — иерархия и сохранение
- `SAGE Events` — система событий
- `SAGE FlowScript` — язык программирования
- `SAGE Draw` / `SAGE Math` — математика и отладка
- `SAGE Terminal` — встроенный терминал
- `SAGE Plugins` — система плагинов
- `SAGE VFrame` — гладкий кадр без временных пауз

## 🧪 Примеры

```bash
cd examples/camera_follow
python main.py
```

Все примеры хранятся отдельно, собственные `data/` для каждого.

## 👤 Авторы

Разработка: [AmckinatorProject]

Поддержка: SAGE Team, OpenSource Helpers

Вдохновлено: PyGame, Godot, Defold, Unreal Nanite

