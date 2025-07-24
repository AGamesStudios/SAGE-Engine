# Python Scripting

The engine can execute `.py` files located in `data/scripts`. During
`core_boot()` the scripts are loaded in alphabetical order if
`enable_python` is set to `true` in `sage/config/scripts.yaml`.

Scripts run inside a strict sandbox. Only common safe builtins such as
`abs`, `range` and `print` are available. Imports are limited to `math`,
`random`, `time` and helpers from `logic_api`. Functions like `open`,
`eval` or `exec` are removed. Attempting to import other modules raises an
`ImportError`.

The sandbox resets for every execution so variables do not leak between
scripts. Helpers such as `create_object` can be injected with
`set_python_globals`.

Example script:

```python
print("Booted")
create_object("cube", "Sprite", {"x": 0, "y": 0})
```

Enable hot reloading with `watch_scripts: true`.
