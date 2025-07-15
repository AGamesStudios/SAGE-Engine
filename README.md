# SAGE Engine

Minimal starting point for the SAGE Engine.

## Quick Start

```bash
python bootstrap.py
python examples/hello_sprite/main.py
pytest -q
```

To use the optional Qt interface install the corresponding extras:

```bash
pip install .[qt6]  # or .[qt5]
```

See [Using SAGE UI with Qt](docs/using_sage_ui_with_qt.md) for details.

## Папки проекта

- `src` - исходники движка
- `examples` - примеры
- `tests` - тесты
- `docs` - документация
- `tools` - служебные скрипты
