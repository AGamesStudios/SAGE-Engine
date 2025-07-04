# Editor API

The engine is independent from any particular editor but provides a small
`EditorInterface` that optional editors can implement. This keeps the runtime
lightweight while still allowing third‑party editors to integrate with future
versions of the engine.

``EditorInterface`` defines methods for adding menu items and toolbar buttons.
Editors are free to expose additional functionality as needed. The optional
``sage_editor`` package implements this interface and loads plugins using
``PluginManager('editor')``. Other editors can do the same without depending on
each other.

Install the ``sage_editor`` package to try the reference editor and run it with
``python -m sage_editor`` or the ``sage-editor`` command. When running from the
repository without installing first, prefix with ``PYTHONPATH=src`` so Python can
find the package.
