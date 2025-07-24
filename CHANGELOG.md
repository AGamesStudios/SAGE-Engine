# Changelog

## Alpha 0.3 - Preview
- render sprites using pygame with placeholder images
- new example project demonstrates Lua-driven movement
- input and time subsystems handle keyboard and delta timing
- documentation expanded with guides for each module
- added `core_debug()` and runtime inspection helpers

## Alpha 0.2 - Stable Core
- feat: optional Qt6 / Qt5 support, auto-fallback
- feat: pluggable GUI backends (entry-point sage_gui)
- feat: CLI backend listing and capability ping
- add lightweight core_boot() system with profiling
- introduce minimal object roles and rendering hooks
- load `.sage_object` files via ResourceManager during boot
- batch sprite draw calls and add basic UI rendering
- implement Scene hierarchy with parent_id and cleanup
- document render and UI subsystems
- introduce events system with object integration
- add `core_debug()` and `get_event_handlers()` for inspecting runtime state
- integrate FlowScript parser and runner with DAG startup
- extend FlowScript with variables, arithmetic and scene object helpers
- make FlowScript grammar configurable via YAML with module loading
- add Lua script runner and hot-reload watcher
- introduce a lightweight window subsystem emitting resize events
- implement FrameSync for smooth timing without GPU VSync
- add Input and Time subsystems with keyboard/mouse helpers

## Alpha 0.4 - Work in Progress
- Added Feather-FX prototype (.sage_fx)
- Added Python script runner with sandboxed execution
- Added Python Spawn and Globals examples
- Introduced low performance mode with `--low-perf` flag and automatic
  detection
- Added Draw and Gizmo subsystems for debug visuals
- Added math helpers with safe expression evaluation and vector operations
- Added `profile_frame` context manager for measuring frame time
- Extended events with async handlers and data filters
- Introduced minimal interactive terminal for running scripts
- Added ScriptsWatcher to hot reload Lua, Python and FlowScript scripts
- Added Final Example demonstrating a full game loop with Python logic
- Introduced Scene module with serialization and DAG traversal
