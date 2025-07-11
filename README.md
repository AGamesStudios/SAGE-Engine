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
Run `python scripts/clean_pycache.py` afterwards to delete leftover `__pycache__` directories.
Install optional extras via:
```bash
pip install .[opengl,sdl,qt,audio,geometry,physics]
```
The Qt based editor requires `PyQt6` and `PyOpenGL`. The SDL renderer depends on
`PySDL2`, boolean mesh utilities rely on `shapely` (install via the
`geometry` extra) and the physics extension uses `pymunk` (through the
`physics` extra). `union_meshes` with the ``negatives`` argument and
``difference_meshes`` both require Shapely; without it these functions raise
``ImportError``. When optional packages are missing the related features are
disabled and a warning is logged.
Plugin modules live in `~/.sage_plugins` by default. Configure a different
location or extra search paths in `sage.toml` under the `[plugins]` table:
```toml
[plugins]
dir = "~/my_plugins"
extra = ["/opt/sage_plugins"]
engine = ["~/engine_plugins"]
editor = ["~/editor_plugins"]
```
Environment variables such as `SAGE_PLUGIN_DIR`, `SAGE_PLUGINS`,
`SAGE_ENGINE_PLUGIN_DIR` and `SAGE_EDITOR_PLUGIN_DIR` are still
honoured for compatibility.
Set `SAGE_GLWIDGET` to ``module:Class`` if a custom Qt OpenGL widget should be
used by the renderer.
If you specify ``--target`` you must add that directory to ``PYTHONPATH`` so the
engine can be imported.
Extras are listed in ``pyproject.toml`` and can be supplied with ``--extras``.

### Editor
The Qt editor launches with the Fusion style and a dark theme. Use the **View**
menu to toggle the grid overlay. The toolbar **Screenshot** button saves the
current viewport as PNG, while the **File** menu lets you open and save
``.sageproject`` files.

## Logs
Log files are written to `~/.cache/sage/logs/engine.log` by default. Configure a
different directory in `sage.toml` under `[logs]` or set the `SAGE_LOG_DIR`
environment variable.
Unhandled exceptions trigger **SAGE Crash**, writing a detailed report to
`~/.cache/sage/crashes/`.

## Repository layout
``src/`` contains the engine libraries and tools. Most users only need the
``examples/`` directory to explore sample projects or start their own.
Documentation is stored in ``docs/``. Optional tools such as the editor can be
packaged with PyInstaller if standalone executables are required.

Runtime state can be saved and loaded with `engine.save_game` and
`engine.load_game`, producing `.sagesave` files.

Open `examples/blank.sageproject` with the editor or runtime to see the basic structure. The sample scene now contains two sprites and a camera. Additional resources in `examples/Resources/` demonstrate `.sageaudio`, `.sagemesh`, `.sageanimation`, `.sagemap` and `.sagelogic` files. Tile maps are pre‑rendered to textures so large maps draw efficiently. Example scenes under `examples/Scenes/` showcase animation, audio playback, event groups, multiple tile maps and basic physics (`Physics.sagescene`). Call ``renderer.unload_texture(obj)`` when removing objects to release the cached sprite texture.
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

Game objects expose a `visible` flag and `alpha` value for transparency. Each
`Engine` manages its own sprite cache so large projects can tune memory usage
independently via `EngineSettings.image_cache_limit` or by calling
`engine.entities.game_object.set_sprite_cache`.
The `engine.mesh_utils` module includes helpers for creating and editing meshes,
including polygons. Use `create_polygon_mesh` to build a custom shape from
arbitrary vertices and combine meshes with the boolean utilities. Boolean
operations support unions and differences of multiple polygons; interior holes
are preserved via triangulation when converting to meshes. Meshes can be
transformed by applying a 3×3 matrix with `Mesh.apply_matrix` for convenient
translations, rotations or scaling.
Rotations wrap once they exceed `360` degrees (configurable via
`EngineSettings.max_angle`) and objects may rotate around their
bounding-box centre when `rotate_bbox` is enabled. The `rotate` method uses this
setting when `around_bbox` isn't specified.
Debug overlays can be drawn with helpers from `engine.gizmos`. Call
`renderer.add_gizmo(gizmos.cross_gizmo(x, y, color=(1,0,0,1), thickness=3))`
to show crosses, circles or squares in world space or use
`gizmos.polyline_gizmo` for custom shapes. Colors use RGBA tuples and the
`thickness` parameter controls line width.

See the documentation for more details on object types, input backends and renderer options.

## License
Released under the [MIT License](LICENSE). All code © 2025 Amckinator Games Studios.
Contributions are welcome! Please keep commit messages descriptive and run the tests before submitting patches. See [CONTRIBUTING.md](CONTRIBUTING.md).
Contact <amckinatorgames@gmail.com> with any questions.
