# Editor API

The engine is independent from any particular editor but provides a small
`EditorInterface` that optional editors can implement. This keeps the runtime
lightweight while still allowing third‑party editors to integrate with future
versions of the engine.

``EditorInterface`` defines methods for adding menu items and toolbar buttons.
Editors are free to expose additional functionality as needed. Plugins may call
these methods when loaded via ``PluginManager('editor')``.
