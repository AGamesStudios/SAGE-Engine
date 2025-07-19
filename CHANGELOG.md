# Changelog

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
