# SAGE Engine

**SAGE Engine** — легкий и модульный 2D/3D движок c поддержкой Python, Lua и FlowScript. Он рассчитан на слабые ПК и веб, обеспечивая высокую скорость и простоту расширения.

## 🚀 О движке

SAGE использует малый набор обязательных зависимостей и позволяет подключать плагины только при необходимости. Любой подсистеме можно подобрать альтернативный бэкенд или полностью отключить её через конфиг.

## 📦 Версии

- **Alpha 0.1** — базовое ядро, профилировка, ChronoPatch, SAGE Render
- **Alpha 0.2** — объекты, сцены, события, Lua
- **Alpha 0.3** — Python и FlowScript, Feather-FX, система логики
- **Alpha 0.4** — модульный ввод, UI, терминал и плавные кадры

## 📁 Структура проекта

Схема см. [docs/project_structure.md](docs/project_structure.md)

## ⚙️ Установка

```bash
git clone https://github.com/yourname/sage-engine.git
cd sage-engine
python -m venv .venv
source .venv/bin/activate  # Windows: .venv\Scripts\activate
pip install -r requirements.txt
python tools/new_project.py MyGame
python examples/camera_follow/main.py
```

## 📜 Скриптовые языки

- **Python** — стандартная логика
- **Lua** — лёгкий и быстрый
- **FlowScript** — минимальный язык движка

## 🧩 Модули и плагины

Движок разделён на независимые модули. Подсистемы можно заменять или расширять новыми бэкендами. Плагины подключаются через конфигурацию или код.

## 🚀 Технологии

- `SAGE Render` — рисование спрайтов
- `SAGE Object` — роли и объекты
- `SAGE Scene` — сериализация сцен
- `SAGE Events` — система событий
- `SAGE Input` — модульная система ввода
- `SAGE FlowScript` — скриптовый язык
- `SAGE Draw` / `SAGE Math` — визуализация и расчёты
 - `SAGE Terminal` — лёгкий GUI-терминал на CustomTkinter
- `SAGE Plugins` — плагины и расширения
- `SAGE VFrame` — плавный кадр без задержек

## 🧪 Примеры

Запускайте примеры из каталога `examples`. Подробности в [docs/examples.md](docs/examples.md).
Для управления проектами воспользуйтесь графическим [SAGE Terminal](docs/terminal.md).
Запустите его командой `python -m tools.sage_terminal.terminal`.

## 📜 Лицензия

Лицензия SAGE — полностью бесплатна. Исходный код закрыт. Движок доступен для некоммерческого и инди-использования. Обратная связь и телеметрия не собирается.

## 👤 Авторы

Разработка: [AmckinatorProject]

Поддержка: SAGE Team, OpenSource Helpers

Вдохновлено различными игровыми движками, но код полностью оригинален
