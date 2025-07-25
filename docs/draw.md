# SAGE Draw

The draw subsystem provides simple primitives for debug visuals. It collects line, rectangle and circle requests so tests can inspect what would be rendered.

```python
from sage_engine.draw import boot, draw_line, draw_rect, draw_circle
```

Functions append a description to an internal list so tests can inspect what would be rendered.

Call `draw_line`, `draw_rect` or `draw_circle` from scripts or tools to display hitboxes and guides.
