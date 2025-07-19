# Feather Architecture

This document summarises the core modules that make up SAGE Feather 0.2.

- **core** – handles boot, reset and subsystem registration.
- **window** – opens the main application window and fires resize events.
- **resource** – loads `.sage_object` files from disk.
- **object** – stores objects in the Scene and manages parent links.
- **render** – batches sprite draw calls and UI widgets.
- **dag** – executes update tasks in topological order.
- **events** – lightweight dispatcher used by the engine and game objects.
- **ui** – minimal user interface elements and theming.

Each subsystem is initialised via `core_boot()` in order. They can be
reinitialised without restarting Python using `core_reset()`.
