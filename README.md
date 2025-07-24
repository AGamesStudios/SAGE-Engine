# SAGE Feather

Status: **Alpha 0.3 (Preview)**

This repository contains **SAGE Feather**, a minimal runtime for the SAGE Engine written in Rust.
It exposes a simple C API that can be used from Python or other languages.

The library includes a ChronoPatch Tree for state storage, a DAG scheduler for tasks
and bindings for running MicroPython scripts.

Audio and physics bindings are optional. When running tests or examples you can
skip them with the ``--skip-audio`` and ``--skip-physics`` flags.

## Building

```bash
cargo build --release --manifest-path rust/feather_core/Cargo.toml
```

The library targets Python 3.8 through 3.13 via PyO3. Set
`PYO3_USE_ABI3_FORWARD_COMPATIBILITY=1` when using newer interpreters.

## Example

Run the Alpha demo to see Feather in action. The demo initialises the Python
engine modules via ``core_boot()`` before calling into the Rust library.
More demos are listed in [docs/examples.md](docs/examples.md).
The `final_example` folder demonstrates a full game loop with hot reloadable Python scripts.
Audio and physics can be disabled if the dependencies are missing:

```bash
python examples/alpha_demo/main.py --skip-audio --skip-physics
```

You can also launch it from within the folder:

```bash
cd examples/alpha_demo
python main.py
```

This example creates a ChronoPatch Tree, patches object velocity,
executes a MicroPython script via a DAG task and prints the final state.
Use ``core_debug()`` to print the current scene and active event handlers.

A simpler example lives in `examples/example_project`. It opens a window,
loads objects and executes a Lua script that moves the `player` sprite.
Run it with `python examples/example_project/main.py`.

## Object Files

Game objects can be stored in JSON files with the ``.sage_object`` extension.
During ``core_boot()`` all files in ``data/objects/`` are loaded automatically.
See [docs/sage_object.md](docs/sage_object.md) for the file format.

The ``tools/build_core.py`` script compiles the engine into ``dist/sage_core.lzma``.
Plugins can be loaded at runtime via ``sage_engine.plugin.load_plugins``.

## Scene

Objects are organised in a Scene using parent/child links. Import helpers from
``sage_engine.scene`` to add or remove objects and to serialise a scene to a
``.sage_scene`` file. See ``docs/sage_scene.md`` for details.

## Render and UI

Sprite objects are batched by image to minimise draw calls. UI elements are
handled by the ``sage_engine.ui`` subsystem. See ``docs/render.md`` and
``docs/ui.md`` for details. A demo scene lives in ``data/objects/ui_demo.sage_object``.
For an overview of the modules see [docs/architecture.md](docs/architecture.md).

## Feather-FX

`sage_fx` loads `.sage_fx` shader files and applies them using either a GPU or
CPU backend. Files are parsed and cached with `load_fx()`. See
[docs/feather_fx.md](docs/feather_fx.md) for the text format and API.
Run `python examples/fx_lab/main.py` to see a demo.

## Draw and Gizmo

`sage_engine.draw` exposes helpers like `draw_line` for rendering debug shapes.
`sage_engine.gizmo` builds on top to visualise object positions. Refer to
[docs/draw.md](docs/draw.md) and [docs/gizmo.md](docs/gizmo.md).

## Math Module

`sage_engine.math` contains small vector and matrix helpers along with
`eval_expr()` for safe expression evaluation. The `plot()` function generates
points for simple graphs. See [docs/math.md](docs/math.md).

## Window

The window subsystem creates the main application window using
`pygame` and dispatches resize events. Configuration resides in
`sage/config/window.yaml`. See [docs/window.md](docs/window.md).

## FrameSync

`sage_engine.framesync` regulates the frame rate without relying on GPU VSync.
Settings live in `sage/config/framesync.yaml` where you can set `target_fps` and
allow or disallow drift. Call `framesync.regulate()` once per frame to maintain
smooth timing. See [docs/framesync.md](docs/framesync.md).

Low-end machines can start the engine in **low performance mode** by passing
`--low-perf` or setting `SAGE_LOW_PERF=1`. See
[docs/performance.md](docs/performance.md) for details. Memory checks rely on
the `psutil` package on Windows and fall back to the standard `resource` module
elsewhere.

## Time

`sage_engine.time` offers `get_time()`, `get_delta()` and `wait(ms)`. Tune `time.scale` or set `time.paused` to control progression. See [docs/time.md](docs/time.md).

## Input

`sage_engine.input` tracks keyboard and mouse state. Use `is_key_down()` or `get_mouse_pos()` from any scripting language. Events like `key_down` and `click` are emitted. See [docs/input.md](docs/input.md).


## Events

The ``sage`` package provides a lightweight event system. Register handlers with
``on(event, handler)`` or an ``async def`` function. Use ``emit(event, data)`` to
trigger them or ``emit_async`` when awaiting inside async code. Event data may
be transformed via filters registered with ``add_filter``. Object fields
prefixed with ``on_`` are automatically connected when added to the scene. See
``docs/events_system.md``.

## FlowScript

FlowScript files (`.sage_fs`) provide simple scripted behaviours. Commands are
defined in ``sage_fs/grammar.yaml`` and can be extended by dropping additional
YAML files into ``sage_fs/flow_modules``. The engine registers a ``flow.run``
DAG task during boot and automatically executes scripts from ``data/scripts``.
See ``docs/flow_script.md`` for details. Both FlowScript and Lua interact with
the scene through a small [logic API](docs/logic_api.md).

## Lua Scripts

Lua 5.4 scripts are supported via the [`lupa`](https://github.com/scoder/lupa)
bindings. Enable Lua in ``sage/config/scripts.yaml`` and place ``.lua`` files in
``data/scripts``. They receive simple helpers like ``create_object`` and ``log``.
Hot reloading can be enabled with ``watch_scripts: true``.

## Python Scripts

Regular Python files can also extend the engine. Enable them with
``enable_python: true`` in ``sage/config/scripts.yaml``. Scripts placed inside
``data/scripts`` execute during ``core_boot()`` in a hardened sandbox. Only a
subset of builtins and modules are available. See
[docs/python_scripts.md](docs/python_scripts.md) for details.

## Terminal

Run `python -m sage_engine.terminal` to start an interactive prompt for
running scripts and managing scenes. See [docs/terminal.md](docs/terminal.md)
for available commands.

## Tests

Run the code style check and unit tests with:

```bash
ruff check .
PYTHONPATH=. pytest -q
```

The checklist for the Alpha release lives in
[SAGE_ALPHA_1.0_checklist.md](SAGE_ALPHA_1.0_checklist.md).
