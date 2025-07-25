# Flow Script Guide

FlowScript is a lightweight language for simple game logic. To attach a script, place a `.fs` file in the example's `data/scripts` folder and reference it from an object or scene.

Example `logic.py` alongside a FlowScript file:

```python
# logic.py
from sage_engine import emit

emit("hello")
```

```fs
# move.fs
set_param("speed", 5)
```

Lua scripts (`.lua`) work the same way and are hot reloaded when using `ScriptsWatcher`.
