# 📘 Raycast Module

`raycast` provides simple 2D ray casting helpers for interaction and AI logic.
It works entirely in software and does not rely on the physics engine.

## API

```python
from sage_engine.raycast import cast_line, cast_circle

hit = cast_line(0, 0, 100, 100)
if hit:
    print("hit", hit.object_id, hit.point)

hits = cast_circle(50, 50, 32)
```

`cast_line` returns the nearest `RaycastHit` or ``None``. `cast_circle` returns
all hits sorted by distance.
