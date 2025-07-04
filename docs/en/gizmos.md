# Gizmos

Debug shapes help visualize object positions or custom data. Use the helpers in
`engine.gizmos` to create simple overlays:

```python
from engine.gizmos import cross_gizmo, polyline_gizmo
renderer.add_gizmo(cross_gizmo(0, 0, size=20,
                               color=(1, 0, 0, 1), thickness=3))
renderer.add_gizmo(polyline_gizmo([(0, 0), (1, 1), (2, 0)],
                                  color=(0, 1, 0, 1)))
```

Available shapes are **cross**, **circle**, **square** and custom
**polylines**. Gizmos are drawn in world coordinates by the active renderer and
cleared with `renderer.clear_gizmos()`. The `color` parameter is an RGBA tuple
and `thickness` controls line width.
