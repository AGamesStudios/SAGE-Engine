# SAGE Draw

The draw subsystem provides simple primitives for debug visuals. It can draw lines, rectangles and circles onto the current window surface using pygame.

```python
from sage_engine.draw import boot, draw_line, draw_rect, draw_circle
```

Functions append a description to an internal list so tests can inspect what would be rendered.

Call `draw_line`, `draw_rect` or `draw_circle` from scripts or tools to display hitboxes and guides.
