# Gizmos

Debug shapes help visualize object positions or custom data. Use the helpers in
`engine.gizmos` to create simple overlays:

```python
from sage_engine.gizmos import cross_gizmo, polyline_gizmo
renderer.add_gizmo(cross_gizmo(0, 0, size=20,
                               color=(1, 0, 0, 1), thickness=3))
renderer.add_gizmo(polyline_gizmo([(0, 0), (1, 1), (2, 0)],
                                  color=(0, 1, 0, 1)))
```

Available shapes are **cross**, **circle**, **square** and custom
**polylines**. Gizmos are drawn in world coordinates by the active renderer.
They normally disappear after one frame but set ``frames`` to keep them longer.
Call ``renderer.clear_gizmos()`` to remove them immediately. The ``color``
parameter is an RGBA tuple and ``thickness`` controls line width.
