# Lua Scripting Guide

Feather can execute Lua 5.4 scripts via the [lupa](https://github.com/scoder/lupa) bindings. Enable Lua in `sage/config/scripts.yaml` and place `.lua` files inside `data/scripts`.

```bash
pip install lupa
```

The engine exposes a minimal API to Lua:

```lua
log("hello")
create_object("Sprite", "player")
obj = get_object("player")
```

When `watch_scripts` is enabled the `ScriptsWatcher` monitors the folder and automatically reloads files on change.
