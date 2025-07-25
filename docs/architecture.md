# Feather Architecture v1.0

The engine is composed of small, independent subsystems. Each subsystem
registers itself with the core via ``register_subsystem(name, factory)`` where
``factory`` lazily creates the module object. ``get_subsystem(name)`` returns the
loaded module. Built‑in subsystems include:

- **core** – handles boot, reset and subsystem registration.
- **window** – opens the main application window and fires resize events.
- **resource** – loads `.sage_object` files from disk.
- **object** – stores objects in the Scene and manages parent links.
- **render** – batches sprite draw calls and UI widgets.
- **framesync** – regulates the frame rate without GPU VSync.
- **dag** – executes update tasks in topological order.
- **events** – lightweight dispatcher used by the engine and game objects.
- **ui** – minimal user interface elements and theming.

The list of subsystem names is stored in ``BOOT_SEQUENCE``. ``core_boot()``
initialises them unless they appear in ``disabled_subsystems`` in
``scripts.yaml``. Extra plugins listed under ``plugins`` can register their own
subsystems before boot.

Call ``get_subsystem("input")`` or similar to interact with a subsystem instead
of importing it directly. ``core_reset()`` reboots every active subsystem in
reverse order.
