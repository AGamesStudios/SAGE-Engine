# Logic API

Scripts written in FlowScript or Lua interact with the engine through a small set of helper functions.

```
create_object(id, role, params)
set_param(id, key, value)
value = get_param(id, key)
destroy_object(id)
emit_event(name, data)
log(message)
```

Use `on_ready` and `on_update` to register callbacks for engine events:

```lua
on_ready(function()
  create_object("hero", "Sprite", {x = 0, y = 0})
end)
```

The `ready` event is emitted once after scripts are loaded. The `update` event
can be triggered by the application to run per-frame logic.
