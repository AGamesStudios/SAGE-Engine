# SAGE Feather

Status: **Alpha 0.2 (Stable Core)**

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

## Object Files

Game objects can be stored in JSON files with the ``.sage_object`` extension.
During ``core_boot()`` all files in ``data/objects/`` are loaded automatically.
See [docs/sage_object.md](docs/sage_object.md) for the file format.

The ``tools/build_core.py`` script compiles the engine into ``dist/sage_core.lzma``.
Plugins can be loaded at runtime via ``sage_engine.plugin.load_plugins``.

## Scene

Objects are organised in a Scene using parent/child links. Use
``add_object`` and ``remove_object`` from ``sage_engine.object`` to manage
hierarchy. See ``docs/sage_scene.md`` for details.

## Render and UI

Sprite objects are batched by image to minimise draw calls. UI elements are
handled by the ``sage_engine.ui`` subsystem. See ``docs/render.md`` and
``docs/ui.md`` for details. A demo scene lives in ``data/objects/ui_demo.sage_object``.
For an overview of the modules see [docs/architecture.md](docs/architecture.md).

## Events

The ``sage`` package provides a lightweight event system. Register handlers with
``on(event, handler)`` and trigger them via ``emit(event, data)``. Object fields
prefixed with ``on_`` are automatically connected when added to the scene. See
``docs/events_system.md``.

## FlowScript

FlowScript files (`.sage_fs`) provide simple scripted behaviours. Commands are
defined in ``sage_fs/grammar.yaml`` and can be extended by dropping additional
YAML files into ``sage_fs/flow_modules``. The engine registers a ``flow.run``
DAG task during boot and automatically executes scripts from ``data/scripts``.
See ``docs/flow_script.md`` for details.

## Tests

Run the code style check and unit tests with:

```bash
ruff check .
PYTHONPATH=. pytest -q
```

The checklist for the Alpha release lives in
[SAGE_ALPHA_1.0_checklist.md](SAGE_ALPHA_1.0_checklist.md).
