# Patcher Scripts

Patchers update the runtime state every frame. Scripts can be written in
either **Python** or **Lua**. Use `engine.load_patcher()` to load a script file
that defines an `apply(tree)` function. Lua patchers require the optional
`lupa` package and will be skipped if LuaJIT is unavailable.

```python
from engine import patchers
p = patchers.load_patcher("logic/blink.lua")
```

When `lupa` is not installed the loader logs a warning and returns a no-op
patcher so projects remain runnable.
