# SAGE Engine

**SAGE Engine** provides a lightweight runtime that can be extended with optional tools such as the Qt based editor. Scenes are stored in the `.sagescene` format and full projects in `.sageproject` files.
This project requires Python 3.10 or newer.
The engine is developed and maintained by **Amckinator Games Studios (AGStudios)**.
Custom resources use compact `.sageaudio`, `.sagemesh` and `.sageanimation` descriptors for sound, mesh and animation data.
Sprite animations expose speed, pause and reverse options while ``AudioManager`` supports simple music playback.

Documentation lives under `docs/`. Start with [docs/en/index.md](docs/en/index.md) for an overview, see [plugins](docs/en/plugins.md) for extension points and [optimisation tips](docs/en/optimisation.md) for performance advice.
For known caveats such as dependency requirements and experimental features see
[limitations](docs/en/limitations.md).

## Quick start
```bash
pip install -r requirements.txt
ruff check .
PYTHONPATH=. pytest -q
```
Optional renderers, audio drivers and the SDK can be installed via extras:
```bash
pip install .[opengl,sdl,audio,sdk]
```
Alternatively run the installer to choose extras interactively:
```bash
python scripts/install.py --extras opengl sdl audio
```
You can also use **SAGE Setup** (requires PyQt6) for a graphical installer.
It defaults to installing under ``~/sage_engine`` and lets you pick extras
like ``opengl``, ``sdl``, ``audio``, ``qt`` and ``sdk``:
```bash
python -m sage_setup
```
Optional tools such as the editor and development SDK are distributed
separately so the engine can be used standalone. ``SAGE Launcher`` is
installed alongside the engine and now lists recent projects with options
to create or open them.

Runtime state can be saved and loaded with `engine.save_game` and
`engine.load_game`, producing `.sagesave` files.

Open `examples/blank.sageproject` with the editor or runtime to see the basic structure. The sample scene now contains two sprites and a camera. Additional resources in `examples/Resources/` demonstrate `.sageaudio`, `.sagemesh`, `.sageanimation`, `.sagemap` and `.sagelogic` files. Tile maps are pre‑rendered to textures so large maps draw efficiently. Example scenes under `examples/Scenes/` showcase animation, audio playback, event groups, multiple tile maps and basic physics (`Physics.sagescene`).
For a larger demonstration use `examples/advanced.sageproject`, which links several scenes to show animations, physics and large tile maps.

## Running
Use `python -m engine path/to/project.sageproject` to launch a game. Running `python main.py` will start the editor if installed, otherwise it behaves the same as the engine runtime. The engine tries to use the OpenGL backend first but falls back to SDL or the headless Null renderer when dependencies are missing:
```bash
python -m engine --renderer sdl examples/blank.sageproject
python -m engine --renderer null examples/blank.sageproject
```
The optional **SAGE Launcher** (requires PyQt6) provides a simple GUI for opening projects:
```bash
python -m sage_launcher
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
