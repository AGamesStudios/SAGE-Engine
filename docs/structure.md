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
│   │   ├── __init__.py
│   │   └── impl/
│   │       ├── __init__.py
│   │       ├── stub.py
│   │       └── win32.py
│   ├── render/
│   │   ├── __init__.py
│   │   └── backends/
│   │       ├── __init__.py
│   │       └── software.py
│   └── graphic/
│       ├── __init__.py
│       ├── api.py
│       ├── color.py
│       ├── fx.py
│       └── style.py
├── tests/
│   └── test_core.py
```

Каждый модуль подключается к циклу `boot → update → draw → flush → shutdown` через
`core.register()` и может предоставлять публичный API через `core.expose()`.
