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
