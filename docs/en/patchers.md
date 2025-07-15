# Patcher Scripts

Patchers update the runtime state every frame. Scripts can be written in
either **Python** or **Lua**. Use `engine.load_patcher()` to load a script file
that defines an `apply(tree)` function. Lua patchers require the optional
`lupa` package and will be skipped if LuaJIT is unavailable.
Scenes relying on Lua should declare the `vm_lua` capability so the engine can warn
if Lua support is missing.

```python
from sage_engine import patchers
p = patchers.load_patcher("logic/blink.lua")
```

When `lupa` is not installed the loader logs a warning and returns a no-op
patcher so projects remain runnable.
