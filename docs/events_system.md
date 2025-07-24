# SAGE Events

The `sage` package implements a tiny event dispatcher used across the engine.
Handlers are registered with `on(event, handler)` and triggered by
`emit(event, data)`. Use `once(event, handler)` for callbacks that remove
themselves after the first call. A handler can be detached with `off(event, handler)`.

Object parameters starting with `on_` are treated as event callbacks when the
object is added to the scene. For example:

```json
{
  "id": "btn",
  "role": "UI",
  "on_click": "print('clicked')"
}
```

When this object enters the scene the string is executed every time the
`click` event is emitted.

Call `cleanup_events()` periodically or when objects are removed to discard
handlers whose owners have been marked for removal.
Use `get_event_handlers()` to inspect the current mapping of events to
callbacks during debugging.

Asynchronous handlers can be registered with `async def` functions. Use
`emit_async()` to await them or `emit()` to schedule them in the background.
Event data can be modified by filters registered with `add_filter(event, func)`.
