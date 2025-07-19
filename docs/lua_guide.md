# Lua Scripting Guide

Feather can execute Lua 5.4 scripts via the [lupa](https://github.com/scoder/lupa) bindings. Enable Lua in `sage/config/scripts.yaml` and place `.lua` files inside `data/scripts`.

```bash
pip install lupa
```

The engine exposes a minimal API to Lua:

```lua
log("hello")
create_object("player", "Sprite", {x = 10})
set_param("player", "y", 20)
value = get_param("player", "x")
```

Register callbacks using `on_ready` and `on_update`:

```lua
on_ready(function()
  log("Lua ready")
end)
```

When `watch_scripts` is enabled the `ScriptsWatcher` monitors the folder and automatically reloads files on change.
