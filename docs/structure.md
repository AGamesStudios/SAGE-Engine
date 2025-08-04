# 📘 Структура проекта SAGE Engine

Этот документ отражает актуальную структуру репозитория после очистки.

```text
SAGE-Engine/
├── main.py
├── engine.sagecfg
├── examples/
│   └── tty_demo.py
├── meta/
│   ├── cleanup_summary.md
│   └── terminal_support.md
├── docs/
│   ├── structure.md
│   └── modules/
│       ├── tty.md
│       ├── color.md
│       └── ui_ascii.md
├── resources/
│   └── themes/
│       └── tty_dark.json
├── sage_engine/
│   ├── core/
│   │   ├── __init__.py
│   │   └── extensible.py
│   ├── color/
│   │   ├── __init__.py
│   │   ├── model.py
│   │   ├── parser.py
│   │   ├── gradient.py
│   │   ├── blend.py
│   │   └── theme.py
│   ├── tty/
│   │   ├── __init__.py
│   │   ├── buffer.py
│   │   ├── color.py
│   │   ├── core.py
│   │   ├── draw.py
│   │   ├── input.py
│   │   ├── screen.py
│   │   ├── ui_core.py
│   │   ├── box.py
│   │   ├── list.py
│   │   ├── editor.py
│   │   ├── widgets.py
│   │   └── theme.py
├── tests/
│   ├── test_core.py
│   ├── test_tty.py
│   ├── test_color.py
│   └── test_ui_ascii.py
```

Каждый модуль подключается к циклу `boot → update → draw → flush → shutdown` через
`core.register()` и может предоставлять публичный API через `core.expose()`.
