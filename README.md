# SAGE Engine

Minimal starting point for the SAGE Engine.

## Quick Start

```bash
python bootstrap.py
python examples/hello_sprite/main.py
pytest -q
```

GUI backends are loaded via plugins. Install the desired extra:

```bash
pip install .[qt6]   # PyQt 6 window
pip install .[tk]    # Tk window
```

Run examples with `--gui` to choose a backend:

```bash
python examples/hello_sprite/main.py --gui qt6
python examples/hello_sprite/main.py --gui auto
sage run --gui list  # show available backends
```

See [Writing your own GUI backend](docs/writing_gui_backend.md) for details.
The folder `examples/plugins/gui_dummy` contains a minimal plugin template.
Short guides: [Audio](docs/audio_quickstart.md),
[Physics](docs/physics_quickstart.md) and
[UI](docs/ui_quickstart.md).

## Папки проекта

- `src` - исходники движка
- `examples` - примеры
- `tests` - тесты
- `docs` - документация
- `tools` - служебные скрипты
