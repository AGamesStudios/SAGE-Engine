# SAGE Engine

**SAGE Engine** provides a lightweight runtime that can be extended with optional tools such as the Qt based editor. Scenes are stored in the `.sagescene` format and full projects in `.sageproject` files. The reference editor lives in the separate ``sage_editor`` package and communicates with the engine via a small interface so alternative editors can be developed independently.
This project requires Python 3.10 or newer. The engine is written in pure
Python and also runs on **PyPy 3.10+** for extra performance.
The engine is developed and maintained by **Amckinator Games Studios (AGStudios)**.
Custom resources use compact `.sageaudio`, `.sagemesh` and `.sageanimation` descriptors for sound, mesh and animation data.
Sprite animations expose speed, pause and reverse options while ``AudioManager`` supports simple music playback.
``ResourceManager`` can load and import files asynchronously to avoid blocking the event loop when using ``asyncio``.

Documentation lives under `docs/`. Start with [docs/en/index.md](docs/en/index.md) for an overview, see [plugins](docs/en/plugins.md) for extension points and [optimisation tips](docs/en/optimisation.md) for performance advice.
For known caveats such as dependency requirements and experimental features see
[limitations](docs/en/limitations.md).

## Quick start
```bash
pip install -r requirements.txt
ruff check .
PYTHONPATH=src pytest -q
```
Install optional extras via:
```bash
pip install .[opengl,sdl,qt,audio]
```
Plugin modules live in `~/.sage_plugins` by default. Set `SAGE_PLUGIN_DIR` to
change this location or list extra directories with `SAGE_PLUGINS`. The
variables `SAGE_ENGINE_PLUGINS` and `SAGE_EDITOR_PLUGINS` override or extend the
search path for engine and editor plugins. These environment variables are read
when plugins are loaded (usually when the engine or editor starts), so set them
before launching:
```bash
SAGE_ENGINE_PLUGINS=~/my_plugins python -m engine game.sageproject
```
If you specify ``--target`` you must add that directory to ``PYTHONPATH`` so the
engine can be imported.
Extras are listed in ``pyproject.toml`` and can be supplied with ``--extras``.

## Repository layout
``src/`` contains the engine libraries and tools. Most users only need the
``examples/`` directory to explore sample projects or start their own.
Documentation is stored in ``docs/``. Optional tools such as the editor can be
packaged with PyInstaller if standalone executables are required.

Runtime state can be saved and loaded with `engine.save_game` and
`engine.load_game`, producing `.sagesave` files.

Open `examples/blank.sageproject` with the editor or runtime to see the basic structure. The sample scene now contains two sprites and a camera. Additional resources in `examples/Resources/` demonstrate `.sageaudio`, `.sagemesh`, `.sageanimation`, `.sagemap` and `.sagelogic` files. Tile maps are pre‑rendered to textures so large maps draw efficiently. Example scenes under `examples/Scenes/` showcase animation, audio playback, event groups, multiple tile maps and basic physics (`Physics.sagescene`).
For a larger demonstration use `examples/advanced.sageproject`, which links several scenes to show animations, physics and large tile maps.

## Running
Use `python -m engine path/to/project.sageproject` to launch a game. Running `python main.py` will start the editor if installed, otherwise it behaves the same as the engine runtime. The optional editor can also be launched directly with `python -m sage_editor` or the `sage-editor` script when installed. The engine tries to use the OpenGL backend first but falls back to SDL or the headless Null renderer when dependencies are missing:
```bash
python -m engine --renderer sdl examples/blank.sageproject
python -m engine --renderer null examples/blank.sageproject
```
When running from the repository without installing first, add ``src`` to
``PYTHONPATH`` so Python can locate the packages:
```bash
PYTHONPATH=src python -m engine examples/blank.sageproject
PYTHONPATH=src python -m sage_editor
```
Use `--vsync` or `--no-vsync` to toggle vertical sync on supporting renderers.
For heavy scenes you can enable asynchronous event updates:
```python
Engine(scene=my_scene, async_events=True).run()
```
or use the asyncio variant:
```python
await Engine(scene=my_scene, asyncio_events=True).update_async(0.0)
```
The engine creates a dedicated event loop when ``asyncio_events`` is enabled.
This mode is experimental and may not integrate with other loops.
Use a ``VariableStore`` for thread-safe variables when running asynchronously.
Extension hooks ``start`` and ``stop`` may also be coroutine functions and are
awaited automatically. When running with ``Engine.run_async`` object ``update``
methods can yield coroutines that will execute without blocking the loop.
Variables may be marked as private so only selected values are shared between
events. Input is handled by the SDL backend unless `qt` is selected to integrate
with a Qt event loop.
Gamepad input can be enabled with the `gamepad` backend if SDL2 supports controllers.
Analog sticks expose axis values that can be combined with key bindings using
`InputManager.bind_axis('move', axis_id=0, positive=K_RIGHT, negative=K_LEFT)`.

Event logic can be stored separately in `.sagelogic` files or generated via
Python scripts referenced by a scene using the `logic_scripts` field. The
logic loader allows `#` or `//` comments so complex behaviour can be organised
comfortably. The event system resolves variables using `$name` shorthand and
supports the `CallFunction` action, `InputAxis` condition and `EvalExpr`
condition for custom Python behaviour.

Game objects expose a `visible` flag and `alpha` value for transparency. The
`engine.mesh_utils` module includes helpers for creating and editing meshes,
including polygons.

See the documentation for more details on object types, input backends and renderer options.

## License
Released under the [MIT License](LICENSE). All code © 2025 Amckinator Games Studios.
Contributions are welcome! Please keep commit messages descriptive and run the tests before submitting patches. See [CONTRIBUTING.md](CONTRIBUTING.md).
Contact <amckinatorgames@gmail.com> with any questions.
