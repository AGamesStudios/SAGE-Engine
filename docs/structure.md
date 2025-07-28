# 📘 Структура проекта SAGE Engine

Этот документ описывает все каталоги и файлы репозитория **SAGE Engine** и поясняет правила работы с архитектурой SAGE Feather. Любое изменение структуры проекта требует обновления этого файла.

## 🔹 Дерево каталогов

```text
SAGE-Engine/
├── README.md
├── audit_report.md
├── docs/
│   ├── architecture.md
│   ├── blueprint.md
│   ├── compatibility.md
│   ├── design_rules.md
│   ├── dev_guidelines.md
│   ├── editor.md
│   ├── events.md
│   ├── flow_script.md
│   ├── getting_started.md
│   ├── intro.md
│   ├── modules/
│   │   ├── logger.md
│   │   ├── render.md
│   │   └── window.md
│   ├── modules.md
│   ├── plugin.md
│   ├── profiler.md
│   ├── renderstream.md
│   ├── roles.md
│   ├── scene.md
│   ├── structure.md
│   ├── stable_api.md
│   ├── style_guide.md
│   ├── timers.md
│   ├── todo.md
│   └── examples/
│       ├── basic_scene.md
│       └── custom_role.md
├── engine.json
├── examples/
│   ├── software_render.py
│   └── graphic_demo.py
├── pyproject.toml
├── roles/
│   ├── Camera.role.json
│   ├── Player.role.json
│   └── Sprite.role.json
├── sage_engine/
│   ├── __init__.py
│   ├── api.py
│   ├── audio/
│   │   └── __init__.py
│   ├── blueprint/
│   │   └── __init__.py
│   ├── compat/
│   │   └── __init__.py
│   ├── core/
│   │   └── __init__.py
│   ├── dag/
│   │   └── __init__.py
│   ├── devtools/
│   │   ├── __init__.py
│   │   ├── cli.py
│   │   ├── config_cli.py
│   │   └── generate_roles.py
│   ├── events/
│   │   └── __init__.py
│   ├── flow/
│   │   ├── __init__.py
│   │   ├── lua/
│   │   │   └── __init__.py
│   │   └── python/
│   │       └── __init__.py
│   ├── game_state/
│   │   └── __init__.py
│   ├── logger/
│   │   ├── __init__.py
│   │   ├── core.py
│   │   ├── crash.py
│   │   ├── errors.py
│   │   ├── handlers/
│   │   │   ├── base.py
│   │   │   └── console.py
│   │   ├── hooks.py
│   │   └── levels.py
│   ├── plugins/
│   │   └── __init__.py
│   ├── profiling/
│   │   └── __init__.py
│   ├── render/
│   │   ├── __init__.py
│   │   ├── api.py
│   │   └── backends/
│   │       ├── __init__.py
│   │       ├── software.py
│   │       └── vulkan.py
│   ├── gfx/
│   │   ├── __init__.py
│   │   ├── runtime.py
│   │   └── backends/
│   │       ├── __init__.py
│   │       └── software.py
│   ├── graphic/
│   │   ├── __init__.py
│   │   ├── color.py
│   │   ├── fx.py
│   │   ├── scene.py
│   │   ├── state.py
│   │   └── compat.py
│   ├── effects/
│   │   ├── __init__.py
│   │   ├── api.py
│   │   ├── compat.py
│   │   └── builtin/
│   ├── resource/
│   │   └── __init__.py
│   ├── roles/
│   │   ├── __init__.py
│   │   ├── camera_schema.py
│   │   ├── interfaces.py
│   │   └── sprite_schema.py
│   ├── scheduler/
│   │   ├── __init__.py
│   │   ├── time.py
│   │   └── timers.py
│   ├── shaders/
│   │   └── __init__.py
│   ├── tasks/
│   │   └── __init__.py
│   ├── window/
│   │   ├── __init__.py
│   │   └── impl/
│   │       ├── cocoa.py
│   │       ├── win32.py
│   │       └── x11.py
│   ├── world/
│   │   ├── __init__.py
│   │   └── view.py
│   └── settings.py
├── sage_testing/
│   ├── __init__.py
│   ├── assert.py
│   ├── collect.py
│   ├── performance.py
│   ├── runner.py
│   ├── visual.py
│   ├── fixtures/
│   │   └── __init__.py
│   └── reports/
│       └── __init__.py
├── tests/
│   ├── conftest.py
│   ├── test_audio.py
│   ├── test_blueprint.py
│   ├── test_cli.py
│   ├── test_compat.py
│   ├── test_core.py
│   ├── test_crash.py
│   ├── test_dag.py
│   ├── test_events.py
│   ├── test_flow_run.py
│   ├── test_game_state.py
│   ├── test_logger.py
│   ├── test_plugins.py
│   ├── test_render.py
│   ├── test_resource.py
│   ├── test_roles_json.py
│   ├── test_scene.py
│   ├── test_tasks.py
│   ├── test_time.py
│   ├── test_timers.py
│   ├── test_visual.py
│   ├── test_window.py
│   └── test_world_load.py
```

## 🔹 Архитектурные правила

- Все модули изолированы и подключаются через `core.register(phase, callback)`.
- Цикл движка состоит из фаз `boot → update → draw → flush → shutdown`.
- Любые изменения объектов возможны только через `SceneEdit`. Прямое изменение полей запрещено.
- События создаются и обрабатываются модулем `events` через `emit` и `on`.
- Логирование выполняется через `logger`, уровни задаются переменной `SAGE_LOGLEVEL`.
- Для обратной совместимости все данные имеют поле `schema_version`; модуль `compat` выполняет миграции.

## 🔹 Масштабирование проекта

- Новые роли описываются JSON файлами в каталоге `roles/` и подключаются генератором `devtools/generate_roles.py`.
- Подплатформенные реализации (Win32, X11, Cocoa) размещаются в `window/impl/`.
- Дополнительные рендер-бэкенды добавляются в `render/backends/`. По умолчанию
  имеется `software.py`.
- При добавлении модуля его следует указать в `engine.json` и написать тесты в `tests/`.

## 🔹 Правила логирования и тестирования

- Все публичные функции сопровождаются докстрингами.
- При изменениях необходимо дополнять тесты и запускать `pytest -q`.
- Логи пишутся в консоль через `logger.handlers.console` и могут сохраняться в файлы.

## 🔹 Рекомендации новичкам

1. Ознакомьтесь с `getting_started.md` и примерами в `examples/`.
2. Изучите структуры ролей в каталоге `roles/` и попробуйте создать свою.
3. Для поиска ошибок запускайте тесты и используйте профайлер из модуля `profiling`.

## 🔹 Обязанности по поддержке

Если при изменении проекта добавляются или переименовываются каталоги, модули или фазы, этот документ **обязательно** должен быть обновлён в том же pull request. Без актуальной документации изменение не принимается.
