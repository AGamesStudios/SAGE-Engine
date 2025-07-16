# SAGE Engine

**Alpha 1.0 Candidate**

Minimal starting point for the SAGE Engine.

⚠️ **Note**: Several subsystems (Audio, Physics, Particles) are currently experimental. These modules are under active development and may be incomplete or non-functional in the current version.

## Quick Start

```bash
python bootstrap.py
python examples/hello_sprite/main.py
pytest -q
```

Install the packages from ``requirements.txt`` to run the examples and tests
on a clean environment.

GUI backends are loaded via plugins. Install the desired extra:

```bash
pip install .[qt6]   # PyQt 6 window
pip install .[tk]    # Tk window
pip install .[opengl]  # OpenGL renderer
```

Run examples with `--gui` to choose a GUI backend. Use `--render` to pick a render backend:

```bash
python examples/hello_sprite/main.py --gui qt6 --render opengl
python examples/hello_sprite/main.py --gui auto --render auto
sage run --gui list   # show available GUI backends
sage run --render opengl script.py
```

Running with the OpenGL backend renders sprites via GPU instancing. The first
frame of the hello example is shown below (screenshot omitted here).

See [Writing your own GUI backend](docs/writing_gui_backend.md) for details.
See [Writing your own render backend](docs/writing_render_backend.md) for details.
The folder `examples/plugins/gui_dummy` contains a minimal plugin template.
Short guides: [Audio](docs/audio_quickstart.md),
[Physics](docs/physics_quickstart.md),
[UI](docs/ui_quickstart.md),
[Tilemap](docs/tilemap_quickstart.md) and
[Particles](docs/particles_quickstart.md),
[Custom shaders](docs/render_shaders.md).
[Sprite rendering](docs/sprite_rendering.md).
[Camera & coordinates](docs/camera_coords.md).
[Fonts and text](docs/fonts_and_text.md).
[Objects](docs/objects.md).

## Папки проекта

- `src` - исходники движка
- `examples` - примеры
- `tests` - тесты
- `docs` - документация
- `tools` - служебные скрипты
