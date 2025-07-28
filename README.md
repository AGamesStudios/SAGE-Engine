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
- [RenderStream](docs/renderstream.md)
- [Рендер](docs/modules/render.md)
- [SAGE Graphic](docs/modules/gfx.md)
- [SAGE Objects](docs/objects.md)
- [Эффекты](docs/modules/effects.md)
- [Плагины](docs/plugin.md)
- [Редактор](docs/editor.md)
- [Руководство разработчика](docs/dev_guidelines.md)
- [Правила проектирования](docs/design_rules.md)
- [Пример Software Render](examples/software_render.py)
- [Пример Graphic](examples/graphic_test.py)
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
To validate the Win32 backend, run the demo and resize or move the window.
Events will be printed in the console when `SAGE_LOGLEVEL=DEBUG`.

- [Отчёт аудита](audit_report.md)
- [Сводка находок](audit_findings.md)
- [План исправлений](fix_roadmap.md)
- [План тестирования](test_plan.md)
- [Профили производительности](benchmark.md)

