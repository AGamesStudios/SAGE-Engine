# SAGE Engine

Документация проекта расположена в каталоге [docs](docs/). Полезные разделы:

- [Введение](docs/intro.md)
- [Быстрый старт](docs/getting_started.md)
- [Архитектура](docs/architecture.md)
- [Модули](docs/modules.md)
- [Структура проекта](docs/structure.md)
- [Роли](docs/roles.md)
- [Сцена](docs/scene.md)
- [События](docs/events.md)
- [Blueprint](docs/blueprint.md)
- [FlowScript](docs/flow_script.md)
- [Таймеры](docs/timers.md)
- [Профайлер](docs/profiler.md)
- [Логирование](docs/modules/logger.md)
- [Система логирования](docs/logger.md)
- [Диагностика сбоев](docs/crash.md)
- [Debug Overlay](docs/debug.md)
- [Тестирование](docs/testing.md)
- [Recovery](docs/recovery.md)
- [RenderStream](docs/renderstream.md)
- [Рендер](docs/modules/render.md)
- [SAGE Graphic](docs/modules/gfx.md)
- [Graphic Module](docs/modules/graphic.md)
- [Sprite Module](docs/modules/sprite.md)
- [Animation Module](docs/modules/animation.md)
- [Resource System](docs/modules/resource.md)
- [Configuration](docs/configuration.md)
- [Performance](docs/performance.md)
- [SAGE Objects](docs/modules/objects.md)
- [SAGE Format](docs/modules/format.md)
- [Эффекты](docs/modules/effects.md)
- [Плагины](docs/plugin.md)
- [Редактор](docs/editor.md)
- [Руководство разработчика](docs/dev_guidelines.md)
- [Правила проектирования](docs/design_rules.md)
- [Демо-игра Pixel Signals](examples/demo_game/main.py)
- [Интеграция рендера](docs/examples/render_integration.md)
- [Совместимость](docs/compatibility.md)
- [Руководство по стилю](docs/style_guide.md)
- [Стабильные интерфейсы](docs/stable_api.md)
- [TODO](docs/todo.md)

Тестирование запускается командой `sage-test`.

Примеры находятся в [docs/examples](docs/examples/).
The demo game in `examples/demo_game` illustrates basic usage. On Windows use
Python 3.10–3.12 and run `python demo.py`. Set `SAGE_LOGLEVEL=DEBUG` for verbose
logs.
Use `sage-pack` to build compressed resource archives.
To validate the Win32 backend, run the demo and resize or move the window.
Events will be printed in the console when `SAGE_LOGLEVEL=DEBUG`.

- [Отчёт аудита](audit_report.md)
- [Сводка находок](audit_findings.md)
- [План исправлений](fix_roadmap.md)
- [План тестирования](test_plan.md)
- [Профили производительности](benchmark.md)

