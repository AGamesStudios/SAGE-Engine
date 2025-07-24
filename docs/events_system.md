# SAGE Events

The `sage` package implements a tiny event dispatcher used across the engine.
Handlers are registered with `on(event, handler)` and triggered by
`emit(event, data)`. Use `once(event, handler)` for callbacks that remove
themselves after the first call. A handler can be detached with `off(event, handler)`.

Every event passes a single `data` argument which can be `None`. Handlers may
either accept one parameter or no parameters at all. When a handler has no
parameters it simply ignores the event data:

```python
def ready_handler(_):  # or `def ready_handler():` works too
    print("Ready!")

on("ready", ready_handler)
emit("ready")  # data will be None
```

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
Errors inside handlers are logged and do not interrupt other callbacks.
Each call to `emit()` prints a short message describing the dispatched event
and number of handlers.
