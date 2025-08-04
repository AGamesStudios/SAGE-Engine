# 📘 Структура проекта SAGE Engine

Этот документ отражает актуальную структуру репозитория после очистки.

```text
SAGE-Engine/
├── main.py
├── README.md
├── meta/
│   ├── audit_findings.md
│   └── fix_roadmap.md
├── docs/
│   ├── getting_started.md
│   └── structure.md
├── sage_engine/
│   ├── core/
│   │   ├── __init__.py
│   │   └── extensible.py
│   ├── logger/
│   │   └── __init__.py
│   ├── window/
│   │   └── __init__.py
│   ├── render/
│   │   └── __init__.py
│   ├── input/
│   │   └── __init__.py
│   ├── world/
│   │   └── __init__.py
│   └── gfx/
│       └── __init__.py
├── tests/
│   └── test_core.py
```

Каждый модуль подключается к циклу `boot → update → draw → flush → shutdown` через
`core.register()` и может предоставлять публичный API через `core.expose()`.
