# Camera and coordinates

`camera` controls the view position and zoom while `coord` converts
between world units and screen pixels. The camera stores its aspect ratio
so it can build a 3×3 view‑projection matrix each frame.

```python
from sage_engine import camera, coord

camera.set(0.0, 0.0, zoom=2.0, aspect_ratio=16/9)
px, py = coord.world_to_screen(1.0, 1.0)
```

Move or scale the view with helper functions:

```python
camera.pan(1.0, 0.0)
camera.zoom_to(3.0)
```

Use `coord.WorldSpace(x, y).to_screen()` to convert structured
positions. UI widgets stay in screen space regardless of the camera
state.
