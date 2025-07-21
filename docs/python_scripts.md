# Python Scripting

The engine can execute `.py` files located in `data/scripts`. During `core_boot()` the scripts are loaded in alphabetical order if `enable_python` is set to `true` in `sage/config/scripts.yaml`.

Scripts run in a restricted namespace with common builtins plus `math` and `random`. Unsafe imports are blocked. They receive the same helpers available to Lua and FlowScript such as `create_object` and `emit`.

Example script:

```python
print("Booted")
create_object("cube", "Sprite", {"x": 0, "y": 0})
```

Enable hot reloading with `watch_scripts: true`.
