# Gizmos

Debug shapes help visualize object positions or custom data. Use the helpers in
`engine.gizmos` to create simple overlays:

```python
from engine.gizmos import cross_gizmo
renderer.add_gizmo(cross_gizmo(0, 0, size=20))
```

Available shapes are **cross**, **circle** and **square**. Gizmos are drawn in
world coordinates by the active renderer and cleared with
`renderer.clear_gizmos()`.
